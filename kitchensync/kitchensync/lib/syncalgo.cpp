
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
    kdDebug(5230) << "SyncFirst " << endl;
    SyncEntry *targetEntry = 0l;

    /* start with the first */
    SyncEntry *sourceEntry = syncee->firstEntry();
    /* whilte it's not the last+1 one */
    while (sourceEntry) {

        if ( sourceEntry->state() == SyncEntry::Removed ) {
	    kdDebug(5230) << "Entry removed " << sourceEntry->name() << endl;
            sourceEntry = syncee->nextEntry();
            continue;
        }

        /* let's see if it's in the other set */
        targetEntry = 0l;
        targetEntry = target->findEntry(sourceEntry->id());

        /* if it is check if modified. if not it's new */
        if (targetEntry) {
	    kdDebug(5230) << "Found target " << endl;
            // Entry already exists in target
            if (sourceEntry->equals(targetEntry)) {
	        kdDebug(5230) << "No action required" << endl;
                // Entries are equal, no action required
            } else {
                // Entries are different, resolve conflict
                if (override && targetEntry->state() != SyncEntry::Removed ) {
                    // Force override
		    kdDebug(5230) << "override" << endl;
                    target->replaceEntry(targetEntry,sourceEntry->clone() );
                } else {
                    if (syncee->hasChanged(sourceEntry) &&
                        target->hasChanged(targetEntry)) {
			kdDebug(5230) << "Deconflict " <<  endl;
			kdDebug(5230) << "Entry 1 state: " << sourceEntry->state() << endl;
			kdDebug(5230) << "Entry 2 state: " << targetEntry->state() << endl;
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
	    kdDebug(5230) << "adding target " << endl;
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
    kdDebug(5230) << "SyncMeta " << endl;
    QPtrList<SyncEntry> entries = syncee->added();
    SyncEntry* entry;
    SyncEntry* targetEntry;
    /* added */
    for ( entry = entries.first(); entry; entry = entries.next() ) {
        /* assign new uid */
	targetEntry = target->findEntry( entry->id() );
	kdDebug(5230) << "About to add " << entry->name() << endl;
	if(!targetEntry ){
	  kdDebug(5230) << "Not added before " << endl;
          addEntry( syncee, target, entry );
	}else {
	  kdDebug(5230) << "Added before " << endl;
	}
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
/*
 * Ok we are either modified
 * or removed
 * Now go through each item and look for one with the
 * same uid
 * If found check if it was Modified or Removed too
 * If yes check if they're equal otherwise
 * we need to deconflict
 */
void PIMSyncAlg::forAll(QPtrList<SyncEntry> entries,  Syncee* syncee,
                        Syncee* target,
                        bool over ) {
    kdDebug() << "For All" << endl;
    SyncEntry* entry;
    SyncEntry* other;
    SyncEntry* result;
    /* for all modified and deleted*/
    for ( entry = entries.first(); entry; entry = entries.next() ) {
        result = 0;
        other = target->findEntry( entry->id()  );
        if (other ) { // exists, should always do
	    kdDebug(5230) << "Entry 1 " << entry->name() << endl;
	    kdDebug(5230) << "Entry 2 " << other->name() << endl;

            /* entry modified and other unchanged */
            if(entry->wasModified() && other->state()== SyncEntry::Undefined ) {
                kdDebug(5230) << "Modified and unchanged " << endl;
                target->replaceEntry( other, entry->clone() );
            }
            /* entry removed and other unchanged or removed too */
            else if ( entry->wasRemoved() &&
                       ( other->wasRemoved() || other->state() == SyncEntry::Undefined ) ) {
                kdDebug(5230) << "Removed and either removed or unchanged too " << endl;
                target->replaceEntry( other, entry->clone() );
            }
            /* entry was removed and other changed */
            else if ( entry->wasRemoved() &&
                other->wasModified() ) {
                kdDebug(5230) << "Entry wasRemoved and other wasModified override is "
                          << over << endl;
                if (!over)
                    result = deconflict(entry,other);
                if (result == entry || over) {
                    target->replaceEntry(other,entry->clone() );
                }

            }else if ( entry->wasModified() && other->wasModified() ) {
                kdDebug(5230) << "Both where modified override" << over<< endl;
                kdDebug(5230) << "Entry1 timestamp " << entry->timestamp() << endl;
                kdDebug(5230) << "Entry2 timestamp " << other->timestamp() << endl;
                kdDebug(5230) << "Equals " << entry->equals( other ) << endl;

                if (!over )
                    result = deconflict(entry,other);

                if (result == entry || over) {
                    target->replaceEntry(other,entry->clone() );
                }

            }

        }else {
	    kdDebug(5230) << "added " << endl;
            addEntry(syncee, target, entry);
        }

    }
}
