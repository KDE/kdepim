
#include <kdebug.h>

#include <syncer.h>

#include "syncalgo.h"

using namespace KSync;

PIMSyncAlg::PIMSyncAlg( SyncUi* ui )
    : SyncAlgorithm( ui ) {


}
PIMSyncAlg::~PIMSyncAlg() {
}

/*
 * let's find the best algorithm first. If both are MetaMode
 * use meta mode or if not use syncFirst
 */
void PIMSyncAlg::syncToTarget( Syncee* syncee,
                               Syncee* target,
                               bool override) {
    if (syncee->syncMode() == Syncee::MetaLess ||
        syncee->firstSync() )
        return syncFirst(syncee, target, override);
    else if ( target->syncMode() == Syncee::MetaLess ||
              target->firstSync() )
        return syncFirst( syncee, target, override );
    else
        return syncMeta( syncee, target, override );
}
/*
 * First Sync or no MetaData
 * slightly changed syncToTarget from StandardSync
 */
void PIMSyncAlg::syncFirst( Syncee* syncee,
                            Syncee* target,
                            bool override ) {
    kdDebug() << "SyncFirst " << endl;
    SyncEntry *targetEntry = 0l;

    /* start with the first */
    SyncEntry *sourceEntry = syncee->firstEntry();
    /* whilte it's not the last+1 one */
    while (sourceEntry) {

        if ( sourceEntry->state() == SyncEntry::Removed ) {
            sourceEntry = syncee->nextEntry();
            continue;
        }

        /* let's see if it's in the other set */
        targetEntry = 0l;
        target->findEntry(sourceEntry->id());

        /* if it is check if modified. if not it's new */
        if (targetEntry) {
            // Entry already exists in target
            if (sourceEntry->equals(targetEntry)) {
                // Entries are equal, no action required
            } else {
                // Entries are different, resolve conflict
                if (override) {
                    // Force override
                    target->replaceEntry(targetEntry,sourceEntry->clone() );
                } else {
                    if (syncee->hasChanged(sourceEntry) &&
                        target->hasChanged(targetEntry)) {
                        // Both entries have changed
                        SyncEntry *result = deconflict(sourceEntry,targetEntry);
                        if (result == sourceEntry) {
                            target->replaceEntry(targetEntry,sourceEntry->clone() );
                        }
                    } else if (syncee->hasChanged(sourceEntry) &&
                               !target->hasChanged(targetEntry)) {
                        // take source entry
                        target->replaceEntry(targetEntry,sourceEntry->clone() );
                    } else if (!syncee->hasChanged(sourceEntry) &&
                               target->hasChanged(targetEntry)) {
                        // take target entry, no action required
                    }
                }
            }
        } else {
            // New entry if id starts with konnector id... set a new one
            addEntry( syncee, target, sourceEntry );
        }
        sourceEntry = syncee->nextEntry();
    }
}
/*
 * We're now in the MetaMode
 * First sync added.
 * Then check the modified
 * Case 1)
 *  Modified   ---- untouched ----> Modify
 *  Modified   ---- Modified ---> Deconflict
 *  Modified   ---- Removed ----> Deconflict
 *  and vice versa
 *
 */
void PIMSyncAlg::syncMeta( Syncee* syncee,
                           Syncee* target,
                           bool over ) {
    kdDebug() << "SyncMeta " << endl;
    QPtrList<SyncEntry> entries = syncee->added();
    SyncEntry* entry;
    /* added */
    for ( entry = entries.first(); entry; entry = entries.next() ) {
        /* assign new uid */
        addEntry( syncee, target, entry );
    }
    /* modified */
    forAll( syncee->modified(), syncee, target, over );
    forAll( syncee->removed(), syncee,  target,over );

}
void PIMSyncAlg::addEntry( Syncee* in, Syncee* out, SyncEntry* add ) {
    if ( add->id().startsWith("Konnector-") ) {
        QString oldId = add->id();
        add->setId( in->newId() );
        in->insertId( add->type(), oldId, add->id() );
        out->insertId( add->type(), oldId, add->id() );
    }
    out->addEntry( add->clone() );
}
void PIMSyncAlg::forAll(QPtrList<SyncEntry> entries,  Syncee* syncee,
                        Syncee* target,
                        bool over ) {
    SyncEntry* entry;
    SyncEntry* other;
    /* for all modified */
    for ( entry = entries.first(); entry; entry = entries.next() ) {
        other = target->findEntry( entry->id()  );
        if (other ) { // exists should always do
            /* not equal */
            if (!entry->equals(other ) ) {
                /* just override it */
                if (over )
                    target->replaceEntry(other,  entry->clone() );
                /* test if modified or removed and deconflict */
                else{
                    if (other->wasRemoved() || other->wasModified() ) {
                        SyncEntry *result =deconflict(entry, other);
                        if (result == entry) {
                            target->replaceEntry(other, entry->clone() );
                        }
                    }
                    /* not changed but why the heck it's not equal then? */
                    else {
                        target->replaceEntry(other,  entry->clone() );
                    }
                }
            }
        }else {
            addEntry(syncee, target, entry);
        }

    }
}
