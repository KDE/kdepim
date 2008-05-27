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
#include "systemtrayicon.h"

#include <utils/gnupg-helper.h>
#include <utils/filesystemwatcher.h>
#include <utils/kdpipeiodevice.h>
#include <utils/log.h>

#include <models/keycache.h>

#ifdef HAVE_USABLE_ASSUAN
# include <uiserver/uiserver.h>
#endif

#include <KGlobal>
#include <KIconLoader>
#include <KLocale>
#include <KCmdLineOptions>
#include <KDebug>

#include <QFile>

#include <boost/shared_ptr.hpp>

#include <memory>

using namespace Kleo;
using namespace boost;

static void add_resources() {
  KGlobal::locale()->insertCatalog( "libkleopatra" );
  KIconLoader::global()->addAppDir( "libkleopatra" );
  KIconLoader::global()->addAppDir( "kdepim" );
}

static KCmdLineOptions make_kleopatra_args() {
    KCmdLineOptions options;
    options.add("daemon", ki18n("Run UI server only, hide main window"));
    options.add("import-certificate ", ki18n("Name of certificate file to import"));
#ifdef HAVE_USABLE_ASSUAN
    options.add("uiserver-socket <argument>", ki18n("Location of the socket the ui server is listening on" ));
#endif
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
public:
    Private() : ignoreNewInstance( true ) {}

public:
    bool ignoreNewInstance;
    SystemTrayIconFor<MainWindow> sysTray;
    shared_ptr<KeyCache> keyCache;
    shared_ptr<Log> log;
    shared_ptr<FileSystemWatcher> watcher;

public:
    void setupKeyCache() {
        keyCache = KeyCache::mutableInstance();
        watcher.reset( new FileSystemWatcher );
  
        watcher->addPaths( gnupgFileWatchList() );
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
        std::auto_ptr<QFile> logFile( new QFile( dir + "/kleo-log" ) );
        if ( !logFile->open( QIODevice::WriteOnly | QIODevice::Append ) ) {
            kDebug() << "Could not open file for logging: " << dir + "/kleo-log\nLogging disabled";
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
    : KUniqueApplication(), d( new Private )
{
    add_resources();
    d->setupKeyCache();
    d->setupLogging();
    d->sysTray.show();
    setQuitOnLastWindowClosed( false );
}

KleopatraApplication::~KleopatraApplication() {
    // work around kdelibs bug https://bugs.kde.org/show_bug.cgi?id=162514
    KGlobal::config()->sync();
}

int KleopatraApplication::newInstance() {
    kDebug() << d->ignoreNewInstance;
    if ( d->ignoreNewInstance )
        return 0;
    KCmdLineArgs * const args = KCmdLineArgs::parsedArgs();
    importCertificatesFromFile( args->getOptionList("import-certificate") );
    args->clear();
    return 0;
}

const SystemTrayIconFor<MainWindow> * KleopatraApplication::sysTrayIcon() const {
    return &d->sysTray;
}

SystemTrayIconFor<MainWindow> * KleopatraApplication::sysTrayIcon() {
    return &d->sysTray;
}

const MainWindow * KleopatraApplication::mainWindow() const {
    return d->sysTray.mainWindow();
}

MainWindow * KleopatraApplication::mainWindow() {
    return d->sysTray.mainWindow();
}

void KleopatraApplication::openOrRaiseMainWindow() {
    d->sysTray.openOrRaiseMainWindow();
}

void KleopatraApplication::importCertificatesFromFile( const QStringList & files ) {
    d->sysTray.openOrRaiseMainWindow();
    if ( !files.empty() )
        d->sysTray.mainWindow()->importCertificatesFromFile( files );
}

void KleopatraApplication::setIgnoreNewInstance( bool ignore ) {
    d->ignoreNewInstance = ignore;
}

bool KleopatraApplication::ignoreNewInstance() const {
    return d->ignoreNewInstance;
}

#include "moc_kleopatraapplication.cpp"
