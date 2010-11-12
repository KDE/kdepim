/*
    kleopatraapplication.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "kleopatraapplication.h"

#include "mainwindow.h"
#include "systrayicon.h"

#include <smartcard/readerstatus.h>
#include <conf/configuredialog.h>

#include <utils/gnupg-helper.h>
#include <utils/filesystemwatcher.h>
#include <utils/kdpipeiodevice.h>
#include <utils/log.h>
#include <utils/getpid.h>

#include <models/keycache.h>

#ifdef HAVE_USABLE_ASSUAN
# include <uiserver/uiserver.h>
#endif

#include <commands/signencryptfilescommand.h>
#include <commands/decryptverifyfilescommand.h>

#include <KGlobal>
#include <KIconLoader>
#include <KLocale>
#include <KCmdLineOptions>
#include <KDebug>
#include <KUrl>
#include <KWindowSystem>

#include <QFile>
#include <QDir>
#include <QPointer>

#include <boost/shared_ptr.hpp>
#include <boost/range.hpp>
#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>

#include <memory>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace boost;

static void add_resources() {
  KGlobal::locale()->insertCatalog( "libkleopatra" );
  KIconLoader::global()->addAppDir( "libkleopatra" );
  KIconLoader::global()->addAppDir( "kwatchgnupg" );
  KIconLoader::global()->addAppDir( "kdepim" );
}

static const struct {
    const char * option;
    const char * description;
    char short_option[4];
} kleo_options[] = {
    { "daemon",             I18N_NOOP("Run UI server only, hide main window"),    ""  },
    { "openpgp",            I18N_NOOP("Use OpenPGP for the following operation"), "p" },
    { "cms",                I18N_NOOP("Use CMS (X.509, S/MIME) for the following operation"), "c" },
    { "import-certificate", I18N_NOOP("Import certificate file(s)"),              "i" },
    { "encrypt",            I18N_NOOP("Encrypt file(s)"),                         "e" },
    { "sign",               I18N_NOOP("Sign file(s)"),                            "s" },
    { "sign-encrypt",       I18N_NOOP("Sign and/or encrypt file(s)"),             "E" },
    { "encrypt-sign",       I18N_NOOP("Same as --sign-encrypt, do not use"),      ""  },
    { "decrypt",            I18N_NOOP("Decrypt file(s)"),                         "d" },
    { "verify",             I18N_NOOP("Verify file/signature"),                   "V" },
    { "decrypt-verify",     I18N_NOOP("Decrypt and/or verify file(s)"),          "D" },
    //{ "show-certificate",   I18N_NOOP("Show Certificate(s) by fingerprint(s)"),   ""  },
};


static KCmdLineOptions make_kleopatra_args() {
    KCmdLineOptions options;
#ifdef HAVE_USABLE_ASSUAN
    options.add("uiserver-socket <argument>", ki18n("Location of the socket the ui server is listening on" ));
#endif
    for ( unsigned int i = 0 ; i < sizeof kleo_options / sizeof *kleo_options ; ++i ) {
        if ( *kleo_options[i].short_option )
            options.add( kleo_options[i].short_option );
        options.add( kleo_options[i].option, ki18n( kleo_options[i].description ) );
    }
    options.add("+[File]", ki18n("File(s) to process"));
    return options;
}

// static
KCmdLineOptions KleopatraApplication::commandLineOptions() {
    static KCmdLineOptions options = make_kleopatra_args();
    return options;
}

static QList<QByteArray> default_logging_options() {
    QList<QByteArray> result;
    result.push_back( "io" );
    return result;
}

class KleopatraApplication::Private {
    friend class ::KleopatraApplication;
    KleopatraApplication * const q;
public:
    explicit Private( KleopatraApplication * qq )
        : q( qq ),
          ignoreNewInstance( true )
    {
        KDAB_SET_OBJECT_NAME( readerStatus );
#ifndef QT_NO_SYSTEMTRAYICON
        KDAB_SET_OBJECT_NAME( sysTray );

        sysTray.setAnyCardHasNullPin( readerStatus.anyCardHasNullPin() );
        sysTray.setAnyCardCanLearnKeys( readerStatus.anyCardCanLearnKeys() );

        connect( &readerStatus, SIGNAL(anyCardHasNullPinChanged(bool)),
                 &sysTray, SLOT(setAnyCardHasNullPin(bool)) );
        connect( &readerStatus, SIGNAL(anyCardCanLearnKeysChanged(bool)),
                 &sysTray, SLOT(setAnyCardCanLearnKeys(bool)) );
#endif
    }

private:
    void connectConfigureDialog() {
        if ( configureDialog && q->mainWindow() )
            connect( configureDialog, SIGNAL(configCommitted()), q->mainWindow(), SLOT(slotConfigCommitted()) );
    }
    void disconnectConfigureDialog() {
        if ( configureDialog && q->mainWindow() )
            disconnect( configureDialog, SIGNAL(configCommitted()), q->mainWindow(), SLOT(slotConfigCommitted()) );
    }

public:
    bool ignoreNewInstance;
    QPointer<ConfigureDialog> configureDialog;
    QPointer<MainWindow> mainWindow;
    SmartCard::ReaderStatus readerStatus;
#ifndef QT_NO_SYSTEMTRAYICON
    SysTrayIcon sysTray;
#endif
    shared_ptr<KeyCache> keyCache;
    shared_ptr<Log> log;
    shared_ptr<FileSystemWatcher> watcher;

public:
    void setupKeyCache() {
        keyCache = KeyCache::mutableInstance();
        watcher.reset( new FileSystemWatcher );

        watcher->blacklistFiles( gnupgFileBlacklist() );
        watcher->addPath( gnupgHomeDirectory() );
        watcher->setDelay( 1000 );
        keyCache->addFileSystemWatcher( watcher );
    }

    void setupLogging() {
        log = Log::mutableInstance();

        const QByteArray envOptions = qgetenv( "KLEOPATRA_LOGOPTIONS" );
        const bool logAll = envOptions.trimmed() == "all";
        const QList<QByteArray> options = envOptions.isEmpty() ? default_logging_options() : envOptions.split( ',' ) ;

        const QByteArray dirNative = qgetenv( "KLEOPATRA_LOGDIR" );
        if ( dirNative.isEmpty() )
            return;
        const QString dir = QFile::decodeName( dirNative );
        const QString logFileName = QDir( dir ).absoluteFilePath( QString::fromLatin1( "kleopatra.log.%1" ).arg( mygetpid() ) );
        std::auto_ptr<QFile> logFile( new QFile( logFileName ) );
        if ( !logFile->open( QIODevice::WriteOnly | QIODevice::Append ) ) {
            kDebug() << "Could not open file for logging: " << logFileName << "\nLogging disabled";
            return;
        }

        log->setOutputDirectory( dir );
        if ( logAll || options.contains( "io" ) )
            log->setIOLoggingEnabled( true );
        qInstallMsgHandler( Log::messageHandler );

#ifdef HAVE_USABLE_ASSUAN
        if ( logAll || options.contains( "pipeio" ) )
            KDPipeIODevice::setDebugLevel( KDPipeIODevice::Debug );
        UiServer::setLogStream( log->logFile() );
#endif

    }
};


KleopatraApplication::KleopatraApplication()
    : KUniqueApplication(), d( new Private( this ) )
{
    add_resources();
    d->setupKeyCache();
    d->setupLogging();
#ifndef QT_NO_SYSTEMTRAYICON
    d->sysTray.show();
#endif
    setQuitOnLastWindowClosed( false );
}

KleopatraApplication::~KleopatraApplication() {
    // work around kdelibs bug https://bugs.kde.org/show_bug.cgi?id=162514
    KGlobal::config()->sync();
}

static QStringList files_from_args( const shared_ptr<const KCmdLineArgs> & args ) {
    QStringList result;
    for ( int i = 0, end = args->count() ; i < end ; ++i ) {
        const KUrl url = args->url(i);
        if ( url.protocol() == QLatin1String("file") )
            result.push_back( url.toLocalFile() );
    }
    return result;
}

namespace {
    typedef void (KleopatraApplication::*Func)( const QStringList &, GpgME::Protocol );
    struct _Funcs {
        const char * opt;
        Func func;
    };
}

int KleopatraApplication::newInstance() {
    kDebug() << "ignoreNewInstance =" << d->ignoreNewInstance;
    if ( d->ignoreNewInstance )
        return 0;

    const shared_ptr<KCmdLineArgs> args( KCmdLineArgs::parsedArgs(), mem_fn( &KCmdLineArgs::clear ) );

    const QStringList files = files_from_args( args );

    const bool openpgp = args->isSet( "openpgp" );
    const bool cms     = args->isSet( "cms" );

    kDebug( openpgp ) << "found OpenPGP";
    kDebug( cms )     << "found CMS";

    if ( openpgp && cms ) {
        kDebug() << "ambigious protocol: --openpgp and --cms";
        return 1;
    }

    static const _Funcs funcs[] = {
        { "import-certificate", &KleopatraApplication::importCertificatesFromFile },
        { "encrypt", &KleopatraApplication::encryptFiles               },
        { "sign", &KleopatraApplication::signFiles                  },
        { "encrypt-sign", &KleopatraApplication::signEncryptFiles           },
        { "sign-encrypt", &KleopatraApplication::signEncryptFiles           },
        { "decrypt", &KleopatraApplication::decryptFiles               },
        { "verify", &KleopatraApplication::verifyFiles                },
        { "decrypt-verify", &KleopatraApplication::decryptVerifyFiles         },
    };

    const _Funcs * const it1 = std::find_if( begin( funcs ), end( funcs ),
                                             bind( &KCmdLineArgs::isSet, args, bind( &_Funcs::opt, _1 ) ) );

    if ( const Func func = it1 == end( funcs ) ? 0 : it1->func ) {
        const _Funcs * it2 = std::find_if( it1+1, end( funcs ),
                                           bind( &KCmdLineArgs::isSet, args, bind( &_Funcs::opt, _1 ) ) );
        if ( it2 != end( funcs ) ) {
            kDebug() << "ambiguous command" << it1->opt << "vs." << it2->opt;
            return 1;
        }
        if ( files.empty() ) {
            kDebug() << it1->opt << "without arguments";
            return 1;
        }
        kDebug() << "found" << it1->opt;
        (this->*func)( files, openpgp ? GpgME::OpenPGP : cms ? GpgME::CMS : GpgME::UnknownProtocol );
    } else {
        if ( files.empty() ) {
            kDebug() << "openOrRaiseMainWindow";
            openOrRaiseMainWindow();
        } else {
            kDebug() << "files without command"; // possible?
            return 1;
        }
    }

    return 0;
}

#ifndef QT_NO_SYSTEMTRAYICON
const SysTrayIcon * KleopatraApplication::sysTrayIcon() const {
    return &d->sysTray;
}

SysTrayIcon * KleopatraApplication::sysTrayIcon() {
    return &d->sysTray;
}
#endif

const MainWindow * KleopatraApplication::mainWindow() const {
    return d->mainWindow;
}

MainWindow * KleopatraApplication::mainWindow() {
    return d->mainWindow;
}

void KleopatraApplication::setMainWindow( MainWindow * mainWindow ) {
    if ( mainWindow == d->mainWindow )
        return;

    d->disconnectConfigureDialog();

    d->mainWindow = mainWindow;
#ifndef QT_NO_SYSTEMTRAYICON
    d->sysTray.setMainWindow( mainWindow );
#endif

    d->connectConfigureDialog();
}

static void open_or_raise( QWidget * w ) {
    if ( w->isMinimized() ) {
        KWindowSystem::unminimizeWindow( w->winId());
        w->raise();
    } else if ( w->isVisible() ) {
        w->raise();
    } else {
        w->show();
    }
}

void KleopatraApplication::openOrRaiseMainWindow() {
    MainWindow * mw = mainWindow();
    if ( !mw ) {
        mw = new MainWindow;
        mw->setAttribute( Qt::WA_DeleteOnClose );
        setMainWindow( mw );
        d->connectConfigureDialog();
    }
    open_or_raise( mw );
}

void KleopatraApplication::openOrRaiseConfigDialog() {
    if ( !d->configureDialog ) {
        d->configureDialog = new ConfigureDialog;
        d->configureDialog->setAttribute( Qt::WA_DeleteOnClose );
        d->connectConfigureDialog();
    }
    open_or_raise( d->configureDialog );
}

#ifndef QT_NO_SYSTEMTRAYICON
void KleopatraApplication::startMonitoringSmartCard() {
    d->readerStatus.startMonitoring();
}

void KleopatraApplication::importCertificatesFromFile( const QStringList & files, GpgME::Protocol /*proto*/) {
    openOrRaiseMainWindow();
    if ( !files.empty() )
        d->sysTray.mainWindow()->importCertificatesFromFile( files );
}
#endif // QT_NO_SYSTEMTRAYICON

void KleopatraApplication::encryptFiles( const QStringList & files, GpgME::Protocol proto ) {
    SignEncryptFilesCommand * const cmd = new SignEncryptFilesCommand( files, 0 );
    cmd->setEncryptionPolicy( Force );
    cmd->setSigningPolicy( Allow );
    if ( proto != GpgME::UnknownProtocol )
        cmd->setProtocol( proto );
    cmd->start();
}

void KleopatraApplication::signFiles( const QStringList & files, GpgME::Protocol proto ) {
    SignEncryptFilesCommand * const cmd = new SignEncryptFilesCommand( files, 0 );
    cmd->setSigningPolicy( Force );
    cmd->setEncryptionPolicy( Deny );
    if ( proto != GpgME::UnknownProtocol )
        cmd->setProtocol( proto );
    cmd->start();
}

void KleopatraApplication::signEncryptFiles( const QStringList & files, GpgME::Protocol proto ) {
    SignEncryptFilesCommand * const cmd = new SignEncryptFilesCommand( files, 0 );
    if ( proto != GpgME::UnknownProtocol )
        cmd->setProtocol( proto );
    cmd->start();
}

void KleopatraApplication::decryptFiles( const QStringList & files, GpgME::Protocol /*proto*/ ) {
    DecryptVerifyFilesCommand * const cmd = new DecryptVerifyFilesCommand( files, 0 );
    cmd->setOperation( Decrypt );
    cmd->start();
}

void KleopatraApplication::verifyFiles( const QStringList & files, GpgME::Protocol /*proto*/ ) {
    DecryptVerifyFilesCommand * const cmd = new DecryptVerifyFilesCommand( files, 0 );
    cmd->setOperation( Verify );
    cmd->start();
}

void KleopatraApplication::decryptVerifyFiles( const QStringList & files, GpgME::Protocol /*proto*/ ) {
    DecryptVerifyFilesCommand * const cmd = new DecryptVerifyFilesCommand( files, 0 );
    cmd->start();
}

void KleopatraApplication::setIgnoreNewInstance( bool ignore ) {
    d->ignoreNewInstance = ignore;
}

bool KleopatraApplication::ignoreNewInstance() const {
    return d->ignoreNewInstance;
}

#include "moc_kleopatraapplication.cpp"
