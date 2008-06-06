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
#include "kleopatraapplication.h"
#include "mainwindow.h"

#include <commands/reloadkeyscommand.h>
#include <commands/selftestcommand.h>

#include <utils/gnupg-helper.h>

#ifdef HAVE_USABLE_ASSUAN
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
# include <uiserver/selectcertificatecommand.h>
#else
namespace Kleo {
    class UiServer;
}
#endif

#include <kcmdlineargs.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ksplashscreen.h>
#include <KDebug>

#include <QTextDocument> // for Qt::escape
#include <QStringList>
#include <QMessageBox>
#include <QTimer>
#include <QEventLoop>
#include <QThreadPool>

#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace boost;

namespace {
    template <typename T>
    boost::shared_ptr<T> make_shared_ptr( T * t ) {
        return t ? boost::shared_ptr<T>( t ) : boost::shared_ptr<T>() ;
    }
}

static bool selfCheck( KSplashScreen & splash ) {
    splash.showMessage( i18n("Performing Self-Check...") );
    Kleo::Commands::SelfTestCommand cmd( 0 );
    cmd.setAutoDelete( false );
    cmd.setAutomaticMode( true );
    cmd.setSplashScreen( &splash );
    QEventLoop loop;
    QObject::connect( &cmd, SIGNAL(finished()), &loop, SLOT(quit()) );
    QObject::connect( &cmd, SIGNAL(info(QString)), &splash, SLOT(showMessage(QString)) );
    QTimer::singleShot( 0, &cmd, SLOT(start()) ); // start() may emit finished()...
    loop.exec();
    if ( cmd.isCanceled() ) {
        splash.showMessage( i18nc("didn't pass", "Self-Check Failed") );
        return false;
    } else {
        splash.showMessage( i18n("Self-Check Passed") );
        return true;
    }
}

static void fillKeyCache( KSplashScreen * splash, Kleo::UiServer * server ) {

  QEventLoop loop;
  Kleo::ReloadKeysCommand * cmd = new Kleo::ReloadKeysCommand( 0 );
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

int main( int argc, char** argv )
{
  {
      const unsigned int threads = QThreadPool::globalInstance()->maxThreadCount();
      QThreadPool::globalInstance()->setMaxThreadCount( qMax( 2U, threads ) );
  }

  AboutData aboutData;

  KCmdLineArgs::init(argc, argv, &aboutData);

  KCmdLineArgs::addCmdLineOptions( KleopatraApplication::commandLineOptions() );

  KleopatraApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KSplashScreen splash( UserIcon( "kleopatra_splashscreen" ), Qt::WindowStaysOnTopHint );

  int rc;
#ifdef HAVE_USABLE_ASSUAN
  try {
      Kleo::UiServer server( args->getOption("uiserver-socket") );

      QObject::connect( &server, SIGNAL(startKeyManagerRequested()),
                        &app, SLOT(openOrRaiseMainWindow()) );

#define REGISTER( Command ) server.registerCommandFactory( boost::shared_ptr<Kleo::AssuanCommandFactory>( new Kleo::GenericAssuanCommandFactory<Kleo::Command> ) )
      REGISTER( DecryptCommand );
      REGISTER( DecryptFilesCommand );
      REGISTER( DecryptVerifyFilesCommand );
      REGISTER( EchoCommand );
      REGISTER( EncryptCommand );
      REGISTER( EncryptFilesCommand );
      REGISTER( EncryptSignFilesCommand );
      REGISTER( PrepEncryptCommand );
      REGISTER( SelectCertificateCommand );
      REGISTER( SignCommand );
      REGISTER( SignEncryptFilesCommand );
      REGISTER( SignFilesCommand );
      REGISTER( VerifyCommand );
      REGISTER( VerifyFilesCommand );
#undef REGISTER

      server.start();
#endif

      const bool daemon = args->isSet("daemon");

      if ( !daemon )
          splash.show();
      if ( !selfCheck( splash ) )
          return 1;
#ifdef HAVE_USABLE_ASSUAN
      fillKeyCache( &splash, &server );
#else
      fillKeyCache( &splash, 0 );
#endif

      app.setIgnoreNewInstance( false );

      if ( !daemon ) {
          app.newInstance();
          splash.finish( app.mainWindow() );
      }

      rc = app.exec();

#ifdef HAVE_USABLE_ASSUAN
      app.setIgnoreNewInstance( true );
      QObject::disconnect( &server, SIGNAL(startKeyManagerRequested()),
                           &app, SLOT(openOrRaiseMainWindow()) );

      server.stop();
      server.waitForStopped();
  } catch ( const std::exception & e ) {
      QMessageBox::information( 0, i18n("GPG UI Server Error"),
                                i18n("<qt>The Kleopatra GPG UI Server Module couldn't be initialized.<br/>"
                                     "The error given was: <b>%1</b><br/>"
                                     "You can use Kleopatra as a certificate manager, but cryptographic plugins that "
                                     "rely on a GPG UI Server being present might not work correctly, or at all.</qt>",
                                     Qt::escape( QString::fromUtf8( e.what() ) ) ));
      app.setIgnoreNewInstance( false );
      rc = app.exec();
      app.setIgnoreNewInstance( true );
  }
#endif

    return rc;
}
