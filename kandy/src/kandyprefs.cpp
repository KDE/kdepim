// $Id$

#include <qdir.h>

#include "kandyprefs.h"

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdebug.h>

KandyPrefs *KandyPrefs::mInstance = 0;


KandyPrefs::KandyPrefs() :
  KPrefs("kandyrc")
{
  KPrefsItem::setCurrentGroup("Serial Port");
  
  addPrefsItem(new KPrefsItemString("Serial Device",
                                    &mSerialDevice,"/dev/ttyS1"));
  addPrefsItem(new KPrefsItemBool("StartupModem",&mStartupModem,
                                  false));
                                  
  KPrefsItem::setCurrentGroup("Windows");
                                  
  addPrefsItem(new KPrefsItemBool("StartupTerminalWin",&mStartupTerminalWin,
                                  false));
  addPrefsItem(new KPrefsItemBool("StartupMobileWin",&mStartupMobileWin,
                                  true));
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
