// $Id$

#include <qdir.h>

#include "kandyprefs.h"

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

KandyPrefs *KandyPrefs::mInstance = 0;


KandyPrefs::KandyPrefs() :
  KConfigSkeleton("kandyrc")
{
  KConfigSkeleton::setCurrentGroup("Serial Port");
  
  addItemString( "", "Serial Device", mSerialDevice, "/dev/ttyS1");
  addItemBool( "", "StartupModem", mStartupModem, false );
                                  
  KConfigSkeleton::setCurrentGroup("Windows");
                                  
  addItemBool( "", "StartupTerminalWin", mStartupTerminalWin, false );
  addItemBool( "", "StartupMobileWin", mStartupMobileWin, true );
}

KandyPrefs::~KandyPrefs()
{
  kdDebug() << "KandyPrefs::~KandyPrefs()" << endl;

  delete mInstance;
  mInstance = 0;
}

KandyPrefs *KandyPrefs::instance()
{
  if (!mInstance) {
    mInstance = new KandyPrefs();
    mInstance->readConfig();
  }
  
  return mInstance;
}
