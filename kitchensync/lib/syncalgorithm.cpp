// $Id$

#include <kdebug.h>

#include "syncui.h"

#include "syncalgorithm.h"

using namespace KSync;

SyncEntry* SyncAlgorithm::deconflict( SyncEntry* syncEntry,  SyncEntry* target ) {
    SyncEntry* entry = syncEntry; // default to this
    if (mUI != 0 )
        entry =mUI->deconflict( syncEntry, target );

    return entry;
}
bool SyncAlgorithm::confirmDelete( SyncEntry* syncEntry, SyncEntry* target ) {
    bool ret = true;
    if (mUI)
        ret = mUI->confirmDelete( syncEntry, target );

    return ret;
}
void SyncAlgorithm::informBothDeleted( SyncEntry* entry, SyncEntry* target ) {
    if (mUI)
        mUI->informBothDeleted( entry, target );
}
