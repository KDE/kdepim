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
  kdDebug(5231) << "deconflicting:" << endl;
  kdDebug(5231) << "  source: " << syncEntry->name() << endl;
  kdDebug(5231) << "  target: " << targetEntry->name() << endl;

  return 0;
}
