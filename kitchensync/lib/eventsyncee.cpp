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

#include <libkcal/calformat.h>

#include <kstaticdeleter.h>
#include "eventsyncee.h"

using namespace KSync;

SyncEntry* EventSyncEntry::clone()
{
    return new EventSyncEntry( *this );
}

EventSyncee::EventSyncee()
    : SyncTemplate<EventSyncEntry>(DtEnd+1)
{
}

/* merging hell! */
namespace {
    typedef MergeBase<KCal::Event, EventSyncee> MergeEvent;
    static MergeEvent* mergeMap = 0l;
    static KStaticDeleter<MergeEvent> deleter;
    
    void mergeDtEnd( KCal::Event* const dest, const KCal::Event* src)
    {
        dest->setDtEnd( src->dtEnd() );
    }
    
    MergeEvent* mapEve()
    {
        if (!mergeMap ) {
            deleter.setObject( mergeMap, new MergeEvent );
            mergeMap->add( EventSyncee::DtEnd, mergeDtEnd );
        }
        return mergeMap;
    }
}

bool EventSyncEntry::mergeWith( SyncEntry* entry )
{
    if ( entry->name() != name() || !syncee() || !entry->syncee() )
        return false;
    EventSyncEntry* toEv = static_cast<EventSyncEntry*>(entry);
    QBitArray da = toEv->syncee()->bitArray();
    QBitArray hier = syncee()->bitArray();

    for (uint i = 0; i< da.size() && i < hier.size(); i++ ) {
        if (da[i] && !hier[i] )
            mapEve()->invoke(i, incidence(), toEv->incidence() );
    }

    return true;
}

QString EventSyncee::type() const
{
    return QString::fromLatin1( "EventSyncee" );
}

Syncee* EventSyncee::clone()
{
    EventSyncee* temp = new EventSyncee();
    temp->setSyncMode( syncMode() );
    temp->setFirstSync( firstSync() );
    temp->setSupports( bitArray() );
    temp->setSource( source() );
    EventSyncEntry* entry;
    for ( entry = mList.first(); entry != 0; entry = mList.next() ) {
        temp->addEntry( entry->clone() );
    }
    return temp;
}

QString EventSyncee::newId() const
{
    return KCal::CalFormat::createUniqueId();
}
