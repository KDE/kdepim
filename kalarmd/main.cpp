// $Id$
//
// KOrganizer/KAlarm alarm daemon main program
//

#include <stdlib.h>
#include <kdebug.h>

#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "alarmapp.h"

static const char* kalarmdVersion = "0.9";
static const KCmdLineOptions options[] =
{
   {0,0,0}
};

int main(int argc, char **argv)
{
  KAboutData aboutData("kalarmd",I18N_NOOP("AlarmDaemon"),
      kalarmdVersion,I18N_NOOP("KOrganizer/KAlarm Alarm Daemon"),KAboutData::License_GPL,
      "(c) 1997-1999 Preston Brown\n(c) 2000-2001 Cornelius Schumacher\n(c) 2001 David Jarvie",0,
      "http://korganizer.kde.org");
  aboutData.addAuthor("Cornelius Schumacher",I18N_NOOP("Maintainer"),
                      "schumacher@kde.org");
  aboutData.addAuthor("David Jarvie",I18N_NOOP("KAlarm Author"),
                      "software@astrojar.org.uk");
  aboutData.addAuthor("Preston Brown",I18N_NOOP("Original Author"),
                      "pbrown@kde.org");

  KCmdLineArgs::init(argc,argv,&aboutData);
  KCmdLineArgs::addCmdLineOptions(options);
  KUniqueApplication::addCmdLineOptions();

  if (!AlarmApp::start())
    exit(0);

  AlarmApp app;

  return app.exec();
}
