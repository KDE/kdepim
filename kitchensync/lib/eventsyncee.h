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
#ifndef KSYNC_EVENT_SYNCEE_H
#define KSYNC_EVENT_SYNCEE_H

#include "incidencetemplate.h"
#include "synctemplate.h"

#include <event.h>

namespace KSync {

class EventSyncEntry : public IncidenceTemplate<KCal::Event>
{
  public:
    EventSyncEntry(KCal::Event* ev = 0l) :
        IncidenceTemplate<KCal::Event>( ev )
    {
    }

    SyncEntry* clone();
    bool mergeWith( SyncEntry* );
};

class EventSyncee : public SyncTemplate<EventSyncEntry>
{
  public:
    enum Supports  {
    // InicdenceBase
        Organizer = 0,
        ReadOnly,
        DtStart,
        Duration,
        Float,
        // Inicdence
        Attendee,
        CreatedDate,
        Revision,
        Description,
        Summary,
        Category,
        Relations,
        ExDates,
        Attachments,
        Secrecy,
        Resources,
        Priority,
        Alarms,
        Recurrence,
        Location,
        // Event
        DtEnd
    };
    EventSyncee();
    Syncee* clone();
    QString type() const;
    QString newId() const;
};

}

#endif
