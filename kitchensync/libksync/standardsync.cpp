/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002,2004 Holger Hans Peter Freyther <freyther@kde.org>


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

#include "standardsync.h"

#include "syncer.h"
#include "syncee.h"

#include <kdebug.h>

using namespace KSync;


/*
 * We only do Syncing based on the Record History. First
 * Sync should carry the 'Added' Attribute
 */
void StandardSync::syncToTarget( Syncee* syncee,
                                 Syncee* target,
                                 bool override)
{
        return syncMeta( syncee, target, override );
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
void StandardSync::syncMeta( Syncee* syncee,
                           Syncee* target,
                           bool over )
{
    kdDebug(5250) << "SYNC: SyncMeta " << endl;
    QPtrList<SyncEntry> entries = syncee->added();
    SyncEntry* entry;
    SyncEntry* targetEntry;


    for ( entry = entries.first(); entry; entry = entries.next() ) {
	targetEntry = target->findEntry( entry->id() );
	kdDebug(5250) << "SYNC: About to add " << entry->name() << endl;
	if(!targetEntry ){
	  kdDebug(5250) << "SYNC: Not added before " << endl;
          addEntry( syncee, target, entry );
	}else {
	  kdDebug(5250) << "SYNC: Added before " << endl;
	}
    }
    /* modified */
    syncSyncEntryListToSyncee( syncee->modified(), syncee, target, over );
    syncSyncEntryListToSyncee( syncee->removed(), syncee,  target,over );

}

/*
 * On non trusted Ids let us replace and reset them
 */
void StandardSync::addEntry( Syncee* in, Syncee* out, SyncEntry* add )
{
    if ( add->id().startsWith("Konnector-") ) {
        QString oldId = add->id();
        add->setId( in->generateNewId() );
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
void StandardSync::syncSyncEntryListToSyncee(QPtrList<SyncEntry> entries,  Syncee* syncee,
                        Syncee* target,
                        bool over )
{
    kdDebug(5250) << "For All" << endl;
    SyncEntry* entry;
    SyncEntry* other;
    SyncEntry* result;
    /* for all modified and deleted*/
    for ( entry = entries.first(); entry; entry = entries.next() ) {
        result = 0;
        other = target->findEntry( entry->id()  );
        if (other ) { // exists, should always do
	    kdDebug(5250) << "Entry 1 " << entry->name() << endl;
	    kdDebug(5250) << "Entry 2 " << other->name() << endl;

            /* entry modified and other unchanged */
            if(entry->wasModified() && other->state()== SyncEntry::Undefined ) {
                kdDebug(5250) << "Modified and unchanged " << endl;
                entry->mergeWith( other );
                target->replaceEntry( other, entry->clone() );
            }
            /* entry removed and other unchanged or removed too */
            else if ( entry->wasRemoved() &&
                      other->wasRemoved() ) {
                kdDebug(5250) << "Removed and removed too " << endl;
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
                 * restore by taking the other entry
                 * and resetting the state. If the other record is intact
                 * maybe we can restore
                 */
                else {
                  entry = other->clone();
                  entry->setState( SyncEntry::Undefined );
                }
            }
            /* entry was removed and other changed */
            else if ( entry->wasRemoved() &&
                other->wasModified() ) {
                kdDebug(5250) << "Entry wasRemoved and other wasModified override is "
                          << over << endl;
                result = 0;
                if (!over)
                    result = deconflict(entry,other);

                /*
                 * If we do not overwrite and no decision was made
                 * then don't sync.
                 */
                if (!over && !result ) {
                  kdDebug(5250) << "SYNC:   no decision" << endl;
                  entry->setDontSync( true );
                  other->setDontSync( true );
                }else if (result == entry || over) {
                    // no need to merge here, we will remove
                    target->replaceEntry(other,entry->clone() );
                }

            } else if ( entry->wasModified() && other->wasModified() ) {
                kdDebug(5250) << "Both where modified and override is" << over<< endl;
                kdDebug(5250) << "Entry1 timestamp " << entry->timestamp() << endl;
                kdDebug(5250) << "Entry2 timestamp " << other->timestamp() << endl;
                kdDebug(5250) << "Equals " << entry->equals( other ) << endl;

                result = 0l;
                if (!over )
                    result = deconflict(entry,other);

                if (!over && !result ) {
                  kdDebug(5250) << "SYNC:   no decision" << endl;
                  entry->setDontSync( true );
                  other->setDontSync( true );
                }else if (result == entry || over) {
                    entry->mergeWith( other );
                    target->replaceEntry(other,entry->clone() );
                }
            }
        } else {
	    kdDebug(5250) << "added " << endl;
            addEntry(syncee, target, entry);
        }

    }
}

