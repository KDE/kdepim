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
bool SyncUi::confirmDelete( SyncEntry* entry, SyncEntry* target ) {
    kdDebug(5231) << "Entry with name " << target->name() << " was deleted " << endl;
    kdDebug(5231) << "From " << target->syncee()->source() << endl;
    return true;
}
void SyncUi::informBothDeleted( SyncEntry* entry, SyncEntry* other ) {
    kdDebug(5231) << "Entry with uid " << entry->id() << "was deleted on both sides " << endl;
}
