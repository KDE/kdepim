// $Id:

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
