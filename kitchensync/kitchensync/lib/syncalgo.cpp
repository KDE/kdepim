/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>

#include <syncee.h>

#include "syncalgo.h"

using namespace KSync;

PIMSyncAlg::PIMSyncAlg( SyncUi* ui )
    : SyncAlgorithm( ui )
{
}

PIMSyncAlg::~PIMSyncAlg()
{
}

/*
 * let's find the best algorithm first. If both are MetaMode
 * use meta mode or if not use syncFirst
 */
void PIMSyncAlg::syncToTarget( Syncee* syncee,
                               Syncee* target,
                               bool override)
{
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
                            bool override )
{
    kdDebug(5231) << "SyncFirst " << endl;
    SyncEntry *targetEntry = 0l;

    /* start with the first */
    SyncEntry *sourceEntry = syncee->firstEntry();
    /* whilte it's not the last+1 one */
    while (sourceEntry) {

        if ( sourceEntry->state() == SyncEntry::Removed ) {
	    kdDebug(5231) << "Entry removed " << sourceEntry->name() << endl;
            sourceEntry = syncee->nextEntry();
            continue;
        }

        /* let's see if it's in the other set */
        targetEntry = 0l;
        targetEntry = target->findEntry(sourceEntry->id());

        /* if it is check if modified. if not it's new */
        if (targetEntry) {
	    kdDebug(5231) << "Found target " << endl;
            // Entry already exists in target
            if (sourceEntry->equals(targetEntry)) {
	        kdDebug(5231) << "No action required" << endl;
                // Entries are equal, no action required
            } else {
                // Entries are different, resolve conflict
                if (override && targetEntry->state() != SyncEntry::Removed ) {
                    // Force override
		    kdDebug(5231) << "overriding and merging!" << endl;
		    // we try to keep as much attributes as possible
		    sourceEntry->mergeWith( targetEntry );
                    target->replaceEntry(targetEntry,sourceEntry->clone() );
                } else {
                    if (syncee->hasChanged(sourceEntry) &&
                        target->hasChanged(targetEntry)) {
			kdDebug(5231) << "Deconflict " <<  endl;
			kdDebug(5231) << "Entry 1 state: " << sourceEntry->state() << endl;
			kdDebug(5231) << "Entry 2 state: " << targetEntry->state() << endl;
                        // Both entries have changed
                        SyncEntry *result = deconflict(sourceEntry,targetEntry);
                        if (result == sourceEntry) {
			    kdDebug(5231) << "Merging and then replacing!" << endl;
			    sourceEntry->mergeWith( targetEntry );
                            target->replaceEntry(targetEntry,sourceEntry->clone() );
                        }else
                            targetEntry->mergeWith( sourceEntry );

                    } else if (syncee->hasChanged(sourceEntry) &&
                               !target->hasChanged(targetEntry)) {
                        // take source entry
                        kdDebug(5231) << "Take source entry" << endl;
                        sourceEntry->mergeWith( targetEntry );
                        target->replaceEntry(targetEntry,sourceEntry->clone() );
                    } else if (!syncee->hasChanged(sourceEntry) &&
                               target->hasChanged(targetEntry)) {
                        // take target entry, no action required but merge
                        kdDebug(5231) << "Take target entry" << endl;
                        targetEntry->mergeWith(sourceEntry);
                    }
                }
            }
        } else {
            // New entry if id starts with konnector id... set a new one
	    kdDebug(5231) << "adding target " << endl;
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
                           bool over )
{
    kdDebug(5231) << "SyncMeta " << endl;
    QPtrList<SyncEntry> entries = syncee->added();
    SyncEntry* entry;
    SyncEntry* targetEntry;
    /* added */
    for ( entry = entries.first(); entry; entry = entries.next() ) {
        /* assign new uid */
	targetEntry = target->findEntry( entry->id() );
	kdDebug(5231) << "About to add " << entry->name() << endl;
	if(!targetEntry ){
	  kdDebug(5231) << "Not added before " << endl;
          addEntry( syncee, target, entry );
	}else {
	  kdDebug(5231) << "Added before " << endl;
	}
    }
    /* modified */
    forAll( syncee->modified(), syncee, target, over );
    forAll( syncee->removed(), syncee,  target,over );

}

void PIMSyncAlg::addEntry( Syncee* in, Syncee* out, SyncEntry* add )
{
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
                        bool over )
{
    kdDebug(5231) << "For All" << endl;
    SyncEntry* entry;
    SyncEntry* other;
    SyncEntry* result;
    /* for all modified and deleted*/
    for ( entry = entries.first(); entry; entry = entries.next() ) {
        result = 0;
        other = target->findEntry( entry->id()  );
        if (other ) { // exists, should always do
	    kdDebug(5231) << "Entry 1 " << entry->name() << endl;
	    kdDebug(5231) << "Entry 2 " << other->name() << endl;

            /* entry modified and other unchanged */
            if(entry->wasModified() && other->state()== SyncEntry::Undefined ) {
                kdDebug(5231) << "Modified and unchanged " << endl;
                entry->mergeWith( other );
                target->replaceEntry( other, entry->clone() );
            }
            /* entry removed and other unchanged or removed too */
            else if ( entry->wasRemoved() &&
                      other->wasRemoved() ) {
                kdDebug(5231) << "Removed and removed too " << endl;
                // no need for merge
                informBothDeleted( entry, other );
                target->replaceEntry( other, entry->clone() );
             /* entry removed and other undefined confirmDelete */
            } else if ( entry->wasRemoved() &&
                       other->state() == SyncEntry::Undefined ) {
                /* if confirmed that is fairly easy */
                if (confirmDelete(entry, other) )
                    target->replaceEntry( other, entry->clone() );
                /*
                 * aye aye how can we do this
                 * first of all we need to remove the modified flag
                 * then we need to set the support to 0 on the syncee
                 * and call a mergeWith
                 * then we reset the BitArray
                 */
                else {
                    QBitArray ar = entry->syncee()->bitArray();
                    QBitArray oth;
                    oth.fill( false, ar.size() );
                    entry->syncee()->setSupports( oth );
                    /* refill the object with life */
                    entry->mergeWith( other );

                    /*
                     * This is specefic to two Syncees
                     * if!over we set the other to Modifed
                     * so the original(other) one will replace the former deleted one
                     * if we're on override there will be no second call
                     */
                    if (!over ) {
                        entry->setState( SyncEntry::Undefined );
                        other->setState( SyncEntry::Modified);
                    } else
                        entry->setState( SyncEntry::Modified);

                    /* restore */
                    entry->syncee()->setSupports( ar );
                }
            }
            /* entry was removed and other changed */
            else if ( entry->wasRemoved() &&
                other->wasModified() ) {
                kdDebug(5231) << "Entry wasRemoved and other wasModified override is "
                          << over << endl;
                if (!over)
                    result = deconflict(entry,other);
                if (result == entry || over) {
                    // no need to merge here too we still remove
                    target->replaceEntry(other,entry->clone() );
                }

            } else if ( entry->wasModified() && other->wasModified() ) {
                kdDebug(5231) << "Both where modified override" << over<< endl;
                kdDebug(5231) << "Entry1 timestamp " << entry->timestamp() << endl;
                kdDebug(5231) << "Entry2 timestamp " << other->timestamp() << endl;
                kdDebug(5231) << "Equals " << entry->equals( other ) << endl;

                if (!over )
                    result = deconflict(entry,other);

                if (result == entry || over) {
                    entry->mergeWith( other );
                    target->replaceEntry(other,entry->clone() );
                }

            }

        } else {
	    kdDebug(5231) << "added " << endl;
            addEntry(syncee, target, entry);
        }

    }
}
