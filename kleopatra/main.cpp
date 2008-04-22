/*
    main.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klar�vdalens Datakonsult AB

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

#include "aboutdata.h"
#include "systemtrayicon.h"
#ifdef KLEO_BUILD_OLD_MAINWINDOW
# include "certmanager.h"
#else
# include "mainwindow.h"
# include <commands/refreshkeyscommand.h>
#endif

#include "libkleo/kleo/cryptobackendfactory.h"

#include <utils/gnupg-helper.h>
#include <utils/filesystemwatcher.h>
#include <utils/kdpipeiodevice.h>
#include <utils/log.h>
#ifdef Q_OS_WIN
#include <utils/gnupg-registry.h>
#endif

#ifdef HAVE_USABLE_ASSUAN
# include "kleo-assuan.h"
# include <uiserver/uiserver.h>
# include <uiserver/assuancommand.h>
# include <uiserver/echocommand.h>
# include <uiserver/decryptcommand.h>
# include <uiserver/verifycommand.h>
# include <uiserver/decryptverifyfilescommand.h>
# include <uiserver/decryptfilescommand.h>
# include <uiserver/verifyfilescommand.h>
# include <uiserver/prepencryptcommand.h>
# include <uiserver/encryptcommand.h>
# include <uiserver/signcommand.h>
# include <uiserver/signencryptfilescommand.h>
#else
namespace Kleo {
    class UiServer;
}
#endif

#include <models/keycache.h>

#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <ksplashscreen.h>
#include <KUniqueApplication>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextDocument> // for Qt::escape
#include <QProcess>
#include <QStringList>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

#include <boost/shared_ptr.hpp>

#ifdef Q_OS_WIN
#include <QSettings>

#include <windows.h>
#endif

#include <memory>
#include <cassert>

namespace {
    template <typename T>
    boost::shared_ptr<T> make_shared_ptr( T * t ) {
        return t ? boost::shared_ptr<T>( t ) : boost::shared_ptr<T>() ;
    }
    static QStringList watchList() {
        const QString home = Kleo::gnupgHomeDirectory();
        QFileInfo info( home );
        if ( !info.isDir() )
            return QStringList();
        QDir homeDir( home );
        QStringList fileList = homeDir.entryList( QDir::AllEntries | QDir::NoDotAndDotDot );
        fileList.removeAll( "dirmngr-cache.d" );
        QStringList result;
        Q_FOREACH( const QString& i, fileList )
            result.push_back( homeDir.absoluteFilePath( i ) );
        return result;
    }
}

#ifdef Q_OS_WIN
static const char gnupg_key[] = "HKEY_LOCAL_MACHINE\\Software\\GNU\\GnuPG";
static const char gnupg_subkey[] = "gpgProgram";

static void checkForInvalidRegistryEntries( QWidget* msgParent )
{
    QSettings settings( gnupg_key, QSettings::NativeFormat );
    if ( !settings.contains( gnupg_subkey ) )
      return;
    if ( KMessageBox::questionYesNo( msgParent, 
                                     i18nc( "@info", "Kleopatra detected an obsolete registry key (<resource>%1\\%2</resource>), "
                                           "added by either by previous gpg4win versions or applications such as WinPT or EnigMail."
                                           "Keeping the entry might lead to problems during operations (old GnuPG backend being used). "
                                           "It is advised to delete the key.", gnupg_key, gnupg_subkey ),
                                     i18n( "Old Registry Entries Detected" ), 
                                     KGuiItem( i18n( "Delete Registry Key" ) ),
                                     KStandardGuiItem::cancel(),
                                     "doNotWarnAboutRegistryEntriesAgain" ) != KMessageBox::Yes )
        return;
    
    settings.remove( gnupg_subkey );
    settings.sync();
    if ( settings.status() == QSettings::NoError )
      return;
    KMessageBox::error( msgParent, 
                        i18nc( "@info", "Could not delete the key <resource>%1\\%2</resource> from the registry.", gnupg_key, gnupg_subkey ), 
                        i18n( "Error Deleting Key" ) );
}
#endif // Q_OS_WIN

static QString environmentVariable( const QString& var, const QString& defaultValue=QString() )
{
    const QStringList env = QProcess::systemEnvironment();
    Q_FOREACH ( const QString& i, env )
    {
        if ( !i.startsWith( var + '=' ) )
            continue;

        const int equalPos = i.indexOf( '=' );
        assert( equalPos >= 0 );
        return i.mid( equalPos + 1 );
    }
    return defaultValue;
}

static void setupLogging()
{
    const QString envOptions = environmentVariable( "KLEOPATRA_LOGOPTIONS", "io" );
    const bool logAll = envOptions.trimmed() == "all";
    const QStringList options = envOptions.split( ',' );
        
    const QString dir = environmentVariable( "KLEOPATRA_LOGDIR" );
    if ( dir.isEmpty() )
        return;
    std::auto_ptr<QFile> logFile( new QFile( dir + "/kleo-log" ) );
    if ( !logFile->open( QIODevice::WriteOnly | QIODevice::Append ) ) {
        qDebug() << "Could not open file for logging: " << dir + "/kleo-log\nLogging disabled";
        return;
    }
        
    Kleo::Log::mutableInstance()->setOutputDirectory( dir );
    if ( logAll || options.contains( "io" ) )
        Kleo::Log::mutableInstance()->setIOLoggingEnabled( true );
    qInstallMsgHandler( Kleo::Log::messageHandler );
    
#ifdef HAVE_USABLE_ASSUAN
    if ( logAll || options.contains( "pipeio" ) )
        KDPipeIODevice::setDebugLevel( KDPipeIODevice::Debug );

    assuan_set_assuan_log_stream( Kleo::Log::instance()->logFile() );
#endif
}

#ifndef KLEO_BUILD_OLD_MAINWINDOW
static void fillKeyCache( KSplashScreen * splash, Kleo::UiServer * server ) {

  QEventLoop loop;
  Kleo::RefreshKeysCommand * cmd = new Kleo::RefreshKeysCommand( 0 );
  QObject::connect( cmd, SIGNAL(finished()), &loop, SLOT(quit()) );
#ifdef HAVE_USABLE_ASSUAN
  QObject::connect( cmd, SIGNAL(finished()), server, SLOT(enableCryptoCommands()) );
#else
  Q_UNUSED( server );
#endif
  splash->showMessage( i18n("Loading certificate cache...") );
  cmd->start();
  loop.exec();
  splash->showMessage( i18n("Certificate cache loaded.") );
}
#endif

int main( int argc, char** argv )
{
  AboutData aboutData;
  
  KCmdLineArgs::init(argc, argv, &aboutData);

  KCmdLineOptions options;
#ifdef KLEO_BUILD_OLD_MAINWINDOW
  options.add("external", ki18n("Search for external certificates initially"));
  options.add("query ", ki18n("Initial query string"));
#else
  options.add("daemon", ki18n("Run UI server only, hide main window"));
#endif
  options.add("import-certificate ", ki18n("Name of certificate file to import"));
#ifdef HAVE_USABLE_ASSUAN
  options.add("uiserver-socket <argument>", ki18n("Location of the socket the ui server is listening on" ) );
#endif
  
  KCmdLineArgs::addCmdLineOptions( options );

  KUniqueApplication app;

  // pin KeyCache to a shared_ptr to define it's minimum lifetime:
  const boost::shared_ptr<Kleo::KeyCache> keyCache = Kleo::KeyCache::mutableInstance();
  const boost::shared_ptr<Kleo::Log> log = Kleo::Log::mutableInstance();
  const boost::shared_ptr<Kleo::FileSystemWatcher> watcher( new Kleo::FileSystemWatcher );
  
  watcher->addPaths( watchList() );
  watcher->setDelay( 1000 );
  keyCache->addFileSystemWatcher( watcher );
  setupLogging();
    
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KGlobal::locale()->insertCatalog( "libkleopatra" );
  KIconLoader::global()->addAppDir( "libkleopatra" );
  KIconLoader::global()->addAppDir( "kdepim" );

#ifndef KLEO_BUILD_OLD_MAINWINDOW
  SystemTrayIconFor<MainWindow> sysTray;
  sysTray.show();
#endif

  KSplashScreen splash( UserIcon( "kleopatra_splashscreen" ), Qt::WindowStaysOnTopHint );

  int rc;
#ifdef HAVE_USABLE_ASSUAN
  try {
      Kleo::UiServer server( args->getOption("uiserver-socket") );

      server.enableCryptoCommands( false );

# ifndef KLEO_BUILD_OLD_MAINWINDOW
      QObject::connect( &server, SIGNAL(startKeyManagerRequested()),
                        &sysTray, SLOT(openOrRaiseMainWindow()) );
# endif

#define REGISTER( Command ) server.registerCommandFactory( boost::shared_ptr<Kleo::AssuanCommandFactory>( new Kleo::GenericAssuanCommandFactory<Kleo::Command> ) )
      REGISTER( DecryptCommand );
      REGISTER( DecryptFilesCommand );
      REGISTER( DecryptVerifyFilesCommand );
      REGISTER( EchoCommand );
      REGISTER( EncryptCommand );
      REGISTER( EncryptFilesCommand );
      REGISTER( EncryptSignFilesCommand );
      REGISTER( PrepEncryptCommand );
      REGISTER( SignCommand );
      REGISTER( SignEncryptFilesCommand );
      REGISTER( SignFilesCommand );
      REGISTER( VerifyCommand );
      REGISTER( VerifyFilesCommand );
#undef REGISTER

      server.start();

# ifndef KLEO_BUILD_OLD_MAINWINDOW
      sysTray.setToolTip( i18n( "Kleopatra UI Server listening on %1", server.socketName() ) );
# endif
#endif

#ifdef KLEO_BUILD_OLD_MAINWINDOW
      if( !Kleo::CryptoBackendFactory::instance()->smime() ) {
          KMessageBox::error(0,
                             i18n( "<qt>The crypto plugin could not be initialized.<br />"
                                   "Certificate Manager will terminate now.</qt>") );
          return -2;
      }

      splash.show();

      CertManager* manager = new CertManager( args->isSet("external"),
                                              args->getOption("query"),
                                              args->getOption("import-certificate") );
      manager->show();
      splash.finish( manager );
#else

      const bool daemon = args->isSet("daemon");

      if ( !daemon )
          splash.show();
#ifdef HAVE_USABLE_ASSUAN
      fillKeyCache( &splash, &server );
#else
      fillKeyCache( &splash, 0 );
#endif

      if ( !daemon ) {
          MainWindow* mainWindow = new MainWindow;
          mainWindow->show();
          sysTray.setMainWindow( mainWindow );
          splash.finish( mainWindow );
#ifdef Q_OS_WIN
          checkForInvalidRegistryEntries( mainWindow );
#endif // Q_OS_WIN
      }

#endif

      args->clear();
      QApplication::setQuitOnLastWindowClosed( false );
      rc = app.exec();

#ifdef HAVE_USABLE_ASSUAN
# ifndef KLEO_BUILD_OLD_MAINWINDOW
      QObject::disconnect( &server, SIGNAL(startKeyManagerRequested()),
                           &sysTray, SLOT(openOrRaiseMainWindow()) );
#endif

      server.stop();
      server.waitForStopped();
  } catch ( const std::exception & e ) {
      QMessageBox::information( 0, i18n("GPG UI Server Error"),
                                i18n("<qt>The Kleopatra GPG UI Server Module couldn't be initialized.<br/>"
                                     "The error given was: <b>%1</b><br/>"
                                     "You can use Kleopatra as a certificate manager, but cryptographic plugins that "
                                     "rely on a GPG UI Server being present might not work correctly, or at all.</qt>",
                                     Qt::escape( QString::fromUtf8( e.what() ) ) ));
      rc = app.exec();
  }
#endif

    return rc;
}
