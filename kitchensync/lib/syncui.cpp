// $Id$

#include <kdebug.h>

#include "syncer.h"

#include "syncui.h"

using namespace KSync;

SyncUi::SyncUi()
{
}

SyncUi::~SyncUi()
{
}

SyncEntry *SyncUi::deconflict(SyncEntry *syncEntry, SyncEntry *targetEntry)
{
  kdDebug() << "deconflicting:" << endl;
  kdDebug() << "  source: " << syncEntry->name() << endl;
  kdDebug() << "  target: " << targetEntry->name() << endl;

  return 0;
}
