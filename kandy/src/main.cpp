#include <kapp.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include "kandy.h"

static const char *description =
    I18N_NOOP("Communicating with your mobile phone.");

static const char *version = "0.1";

int main(int argc, char **argv)
{
  KAboutData about("kandy", I18N_NOOP("Kandy"), version, description,
  KAboutData::License_GPL, "(C) 2001 Cornelius Schumacher");
  about.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );
  KCmdLineArgs::init(argc,argv,&about);

  KApplication app;

  // register ourselves as a dcop client
  app.dcopClient()->registerAs(app.name(),false);

  // see if we are starting with session management
  if (app.isRestored())
      RESTORE(Kandy)
  else
  {
    // no session.. just start up normally
    Kandy *widget = new Kandy;
    if (argc > 1)
        widget->load(argv[1]);
    widget->show();
  }

  return app.exec();
}
