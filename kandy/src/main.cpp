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
#include "commandscheduler.h"
#include "kandyprefs.h"

static const char *description =
    I18N_NOOP("Communicating with your mobile phone.");

static const char *version = "0.3";

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

  modem->setDevice(KandyPrefs::serialDevice());
  modem->setSpeed(19200);
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
                   "http://devel-home.kde.org/~kandy");
  about.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );
  KCmdLineArgs::init(argc,argv,&about);
  KCmdLineArgs::addCmdLineOptions(options);

  KApplication app;
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // register ourselves as a dcop client
  app.dcopClient()->registerAs(app.name(),false);

  Modem *modem = new Modem;
  CommandScheduler *scheduler = new CommandScheduler(modem);

  // see if we are starting with session management
  if (app.isRestored()) {
    // TODO: do session management
//      RESTORE(Kandy)
  } else
  {
    // no session.. just start up normally
    Kandy *k = new Kandy(scheduler);

    MobileMain *m = new MobileMain(scheduler);
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
    QObject::connect(m,SIGNAL(modemConnect()),k,SLOT(modemConnect()));
    QObject::connect(m,SIGNAL(modemDisconnect()),k,SLOT(modemDisconnect()));
    QObject::connect(k,SIGNAL(connectStateChanged(bool)),
                     m,SLOT(setConnected(bool)));

    QObject::connect( modem, SIGNAL( errorMessage( const QString & ) ),
                      k, SLOT( showErrorMessage( const QString & ) ) );

    initModem(modem);

    if (KandyPrefs::startupModem()) k->modemConnect();
  }

  return app.exec();
}
