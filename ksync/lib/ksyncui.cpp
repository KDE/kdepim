// $Id$

#include <kdebug.h>

#include "ksyncer.h"

#include "ksyncui.h"

KSyncUi::KSyncUi()
{
}

KSyncUi::~KSyncUi()
{
}

KSyncEntry *KSyncUi::deconflict(KSyncEntry *syncEntry,KSyncEntry *targetEntry)
{
  kdDebug() << "deconflicting:" << endl;
  kdDebug() << "  source: " << syncEntry->name() << endl;
  kdDebug() << "  target: " << targetEntry->name() << endl;

  return 0;
}
