
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
	    kdDebug() << "Entry removed " << sourceEntry->name() << endl;
            sourceEntry = syncee->nextEntry();
            continue;
        }

        /* let's see if it's in the other set */
        targetEntry = 0l;
        targetEntry = target->findEntry(sourceEntry->id());

        /* if it is check if modified. if not it's new */
        if (targetEntry) {
	    kdDebug() << "Found target " << endl;
            // Entry already exists in target
            if (sourceEntry->equals(targetEntry)) {
	        kdDebug() << "No action required" << endl;
                // Entries are equal, no action required
            } else {
                // Entries are different, resolve conflict
                if (override && targetEntry->state() != SyncEntry::Removed ) {
                    // Force override
		    kdDebug() << "override" << endl;
                    target->replaceEntry(targetEntry,sourceEntry->clone() );
                } else {
                    if (syncee->hasChanged(sourceEntry) &&
                        target->hasChanged(targetEntry)) {
			kdDebug() << "Deconflict " <<  endl;
			kdDebug() << "Entry 1 state: " << sourceEntry->state() << endl;
			kdDebug() << "Entry 2 state: " << targetEntry->state() << endl;
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
	    kdDebug() << "adding target " << endl;
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
    SyncEntry* targetEntry;
    /* added */
    for ( entry = entries.first(); entry; entry = entries.next() ) {
        /* assign new uid */
	targetEntry = target->findEntry( entry->id() );
	kdDebug() << "About to add " << entry->name() << endl;
	if(!targetEntry ){
	  kdDebug() << "Not added before " << endl;
          addEntry( syncee, target, entry );
	}else {
	  kdDebug() << "Added before " << endl;
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
void PIMSyncAlg::forAll(QPtrList<SyncEntry> entries,  Syncee* syncee,
                        Syncee* target,
                        bool over ) {
    kdDebug() << "For All" << endl;
    SyncEntry* entry;
    SyncEntry* other;
    /* for all modified and deleted*/
    for ( entry = entries.first(); entry; entry = entries.next() ) {
        other = target->findEntry( entry->id()  );
        if (other ) { // exists should always do
	    kdDebug() << "Entry 1 " << entry->name() << endl;
	    kdDebug() << "Entry 2 " << other->name() << endl;
            /* not equal */
            if (!entry->equals(other ) ) {
                /* just override it */
		kdDebug() << "Not equals " << endl;
                if (over && (other->state() != SyncEntry::Removed) ) {
		    kdDebug() << "override" << endl;
                    target->replaceEntry(other,  entry->clone() );
                /* test if modified or removed and deconflict */
                }else{
		    /* both changed */
                    if (( other->wasRemoved() || other->wasModified() )&&
			(entry->wasRemoved() || entry->wasModified() ) ) {
		        kdDebug() << "Other was modified or Removed" << endl;
                        SyncEntry *result =deconflict(entry, other);
                        if (result == entry) {
                            target->replaceEntry(other, entry->clone() );
                        }
                    }
		    /* other changed but not we so do not bother anymore */
		    else if( (other->wasRemoved() || other->wasModified() ) &&
		              (entry->state() == SyncEntry::Undefined ) ) {
			      kdDebug() << "other changed " << endl;
		    }
                    /* not changed but why the heck it's not equal then? */
                    else {
		        kdDebug() << "Other did not change so we did?" << endl;
                        target->replaceEntry(other,  entry->clone() );
                    }
                }
            }
        }else {
	    kdDebug() << "added " << endl;
            addEntry(syncee, target, entry);
        }

    }
}
