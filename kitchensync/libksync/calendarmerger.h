/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>

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
#ifndef KSYNC_CALENDARSYNCEE_MERGER_H
#define KSYNC_CALENDARSYNCEE_MERGER_H


#include <merger.h>

namespace KSync{
class CalendarSyncEntry;
class CalendarMerger : public Merger 
{
public:
    /**
     * ENUM of commonly used Attributues
     */
    enum BaseSupports {
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
        BaseLast
    };

    /**
     * Attributes Unique to Todo
     */
    enum TodoSupports {
        DtDue = BaseLast,
        StartDate,
        Completed,
        Percent,
        StartDateTime, /* time of startdate supported probably also not support Float*/
        DueDateTime   /* time of duedate   supported probably also not support Float*/      
    };

    /**
     * Attribute Unique to Events
     */
    enum EventSupports  {
        DtEnd = BaseLast
    };

    CalendarMerger( const QBitArray& todo, const QBitArray& event);
    ~CalendarMerger();

    bool merge( SyncEntry* entry, SyncEntry* other );
private:
    /**
     * merge the common incidence attributes
     * @param entry where attributes should be merged in
     * @param other From where to pull the Attrubutes
     * @param the SupportArray
     */
    void mergeTodo( CalendarSyncEntry *entry, CalendarSyncEntry* other);
    void mergeCal ( CalendarSyncEntry *entry, CalendarSyncEntry* other);
    QBitArray mEvent, mTodo;
};
}


#endif
