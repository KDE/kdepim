// $Id$

#include <qstring.h>

#include <kcmdlineargs.h>
#include <kdebug.h>

#include "alarmdaemon.h"

#include "alarmapp.h"
#include "alarmapp.moc"


AlarmApp::AlarmApp() :
  KUniqueApplication(),
  mAd(0)
{
}

AlarmApp::~AlarmApp()
{
}

int AlarmApp::newInstance()
{
  kdDebug() << "kalarmd:AlarmApp::newInstance()" << endl;

  // Check if we already have a running alarm daemon widget
  if (mAd) return 0;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  mAd = new AlarmDaemon(0, "ad");

  return 0;
}
