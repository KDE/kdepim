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
#ifndef KSYNC_TODO_SYNCEE_H
#define KSYNC_TODO_SYNCEE_H

#include <todo.h>

#include "synctemplate.h"

namespace KSync {

class TodoSyncEntry : public SyncEntry
{
  public:
    TodoSyncEntry( KCal::Todo* = 0);
    TodoSyncEntry( const TodoSyncEntry& );
    ~TodoSyncEntry();
    KCal::Todo* todo();

    QString type() const;
    QString name();
    QString id();
    void setId(const QString& id );
    SyncEntry* clone();
    bool equals( SyncEntry* entry );
    QString timestamp();
    bool mergeWith( SyncEntry* );

  private:
    KCal::Todo* mTodo;
};

class TodoSyncee : public SyncTemplate<TodoSyncEntry>
{
  public:
    enum Supports {
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
        // Todo
        DtDue,
        StartDate,
        Completed,
        Percent
    };

    TodoSyncee();
    QString type() const;
    Syncee *clone();
    QString newId() const;
};

}

#endif
