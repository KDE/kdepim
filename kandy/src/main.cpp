/*
    This file is part of Kandy.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qfile.h>

#include <kapplication.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "modem.h"
#include "kandy.h"
#include "mobilemain.h"
#include "mobilegui.h"
#include "commandscheduler.h"
#include "kandyprefs.h"

static const char description[] =
    I18N_NOOP("Communicating with your mobile phone.");

static const char version[] = "0.5.1";

static KCmdLineOptions options[] =
{
   { "terminal", I18N_NOOP("Show terminal window."), 0 },
   { "mobilegui", I18N_NOOP("Show mobile GUI."), 0 },
   { "nogui", I18N_NOOP("Don't show GUI."), 0 },
   { "+[profile]", I18N_NOOP("Filename of command profile file."), 0 },
   KCmdLineLastOption // End of options.
};

void initModem(Modem *modem)
{
  kdDebug() << "Opening serial Device: "
            << KandyPrefs::serialDevice()
            << endl;

  modem->setSpeed( KandyPrefs::baudRate().toUInt() );
  modem->setData(8);
  modem->setParity('N');
  modem->setStop(1);

#if 0
  if (!modem->dsrOn()) {
    KMessageBox::sorry(this, i18n("Modem is off."), i18n("Modem Error"));
    modem->close();
    return;
  }
  if (!modem->ctsOn()) {
    KMessageBox::sorry(this, i18n("Modem is busy."), i18n("Modem Error"));
    modem->close();
    return;
  }
#endif

#if 0
  modem->writeLine("");
  usleep(250000);
  modem->flush();
  modem->writeLine("ATZ");
#endif
}

int main(int argc, char **argv)
{
  KAboutData about("kandy", I18N_NOOP("Kandy"), version, description,
                   KAboutData::License_GPL, "(C) 2001 Cornelius Schumacher",0,
                   "http://kandy.kde.org/");
  about.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );
  about.addAuthor( "Heiko Falk", 0, "hf2@ls12.cs.uni-dortmund.de" );
  KCmdLineArgs::init(argc,argv,&about);
  KCmdLineArgs::addCmdLineOptions(options);

  KApplication app;
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // register ourselves as a dcop client
  app.dcopClient()->registerAs(app.name(),false);

  Modem *modem = new Modem(KandyPrefs::self());
  CommandScheduler *scheduler = new CommandScheduler(modem);

  // see if we are starting with session management
  if (app.isRestored()) {
    // TODO: do session management
//      RESTORE(Kandy)
  } else
  {
    // no session.. just start up normally
    Kandy *k = new Kandy(scheduler);

    MobileMain *m = new MobileMain(scheduler, KandyPrefs::self());
    
    if (!args->isSet("gui")) {
    } else {
      if (KandyPrefs::startupTerminalWin() ||
          args->isSet("terminal")) {
        k->show();
      }
      if (KandyPrefs::startupMobileWin() ||
          args->isSet("mobilegui")) {
        m->show();
      }
    }

    if (args->count() == 1) {
      k->load(QFile::decodeName(args->arg(0)));
    } else if (args->count() > 1) {
      args->usage();
    }

    args->clear();

    QObject::connect(k,SIGNAL(showMobileWin()),m,SLOT(show()));
    QObject::connect(m,SIGNAL(showTerminalWin()),k,SLOT(show()));
    QObject::connect(m,SIGNAL(showPreferencesWin()),
                     k,SLOT(optionsPreferences()));

    QObject::connect( m->view(), SIGNAL( connectModem() ), k,
                      SLOT( modemConnect() ) );
    QObject::connect( m->view(), SIGNAL( disconnectModem() ), k,
                      SLOT( modemDisconnect() ) );

    QObject::connect( modem, SIGNAL( errorMessage( const QString & ) ),
                      k, SLOT( showErrorMessage( const QString & ) ) );

    initModem( modem );

    if ( KandyPrefs::startupModem() )
      m->view()->toggleConnection();
  }

  return app.exec();
}
