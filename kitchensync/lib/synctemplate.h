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
#ifndef KSYNC_GENERICSYNCEE_H
#define KSYNC_GENERICSYNCEE_H

#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>

#include "syncee.h"

/**
 * this is my first template ever
 */
namespace KSync {
    template <class Entry= SyncEntry>
    class SyncTemplate : public Syncee {
    public:
        typedef QPtrList<Entry> PtrList;
        SyncTemplate(uint i = 0) : Syncee(i)  {
            mList.setAutoDelete( true );
        };
        ~SyncTemplate() { };
/*        QString type() const { return QString::fromLatin1(typeName); }*/
        /**
         * basic clone implementation
         */
        QString type() const { return QString::fromLatin1("SyncTemplate"); }
        Syncee* clone() {
            SyncTemplate* temp = new SyncTemplate<Entry>();
            temp->setSyncMode( syncMode() );
	    temp->setFirstSync( firstSync() );
            Entry* entry;
            for ( entry = mList.first(); entry != 0; entry = mList.next() ) {
                temp->addEntry( (Entry*)entry->clone() );
            }
            return temp;
        };

        SyncEntry* firstEntry() {
            return mList.first();
        }
        SyncEntry* nextEntry() {
            return mList.next();
        }
        SyncEntry::PtrList added() {
            return find( SyncEntry::Added );
        }
        SyncEntry::PtrList modified() {
            return find( SyncEntry::Modified );
        }
        SyncEntry::PtrList removed() {
            return find(SyncEntry::Removed );
        }
        void addEntry( SyncEntry* entry ) {
            kdDebug(5230) << "addEntry " << entry->type() << endl;
            Entry* tempEntry = dynamic_cast<Entry*> ( entry );
            if ( tempEntry == 0l ) {
                kdDebug(5230) << "could not cast" << endl;
                return;
            };
            tempEntry->setSyncee( this );
            if ( tempEntry->state() == SyncEntry::Undefined ) {
                if ( hasChanged( tempEntry ) )
                    tempEntry->setState( SyncEntry::Modified );
            }
            mList.append( tempEntry );
        }
        void removeEntry( SyncEntry* entry ) {
            Entry* tempEntry = dynamic_cast<Entry*> ( entry );
            if ( tempEntry == 0l )
                return;
            mList.remove( tempEntry );
        }

    protected:
        SyncEntry::PtrList find( int state ) {
            kdDebug(5230) << "find state " << state << endl;
            SyncEntry::PtrList found;
            Entry* entry;
            for (entry = mList.first(); entry != 0; entry = mList.next() ) {
                if ( entry->state() == state ) {
                    kdDebug(5230) << "matched state in find " << entry->state() << endl;
                    found.append( entry );
                }
            }
            return found;
        }
        PtrList mList;

    };
}


#endif
