
// $Id$

#include <kdebug.h>
#include "syncer.h"

#include "standardsync.h"

using namespace KSync;

void StandardSync::syncToTarget( Syncee* source, Syncee* target,  bool override ) {
    kdDebug(5200) << "StandardSync::syncToTarget(): from: " << source->filename()
              << " to: " << target->filename() << "  override: "
              << override  << endl;

    SyncEntry *sourceEntry = source->firstEntry();
    while (sourceEntry) {
        SyncEntry *targetEntry = target->findEntry(sourceEntry->id());
        if (targetEntry) {
            // Entry already exists in target
            if (sourceEntry->equals(targetEntry)) {
                // Entries are equal, no action required
            } else {
                // Entries are different, resolve conflict
                if (override) {
                    // Force override
                    target->replaceEntry(targetEntry,sourceEntry);
                } else {
                    if (source->hasChanged(sourceEntry) &&
                        target->hasChanged(targetEntry)) {
                        // Both entries have changed
                        SyncEntry *result = deconflict(sourceEntry,targetEntry);
                        if (result == sourceEntry) {
                            target->replaceEntry(targetEntry,sourceEntry);
                        }
                    } else if (source->hasChanged(sourceEntry) &&
                               !target->hasChanged(targetEntry)) {
                        // take source entry
                        target->replaceEntry(targetEntry,sourceEntry);
                    } else if (!source->hasChanged(sourceEntry) &&
                               target->hasChanged(targetEntry)) {
                        // take target entry, no action required
                    }
                }
            }
        } else {
            // New entry
            target->addEntry(sourceEntry);
        }

        sourceEntry = source->nextEntry();
    }
}
