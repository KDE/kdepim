/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "synceelist.h"

#include "calendarsyncee.h"
#include "addressbooksyncee.h"
#include "eventsyncee.h"
#include "todosyncee.h"

using namespace KSync;

SynceeList::SynceeList()
{
}

SynceeList::~SynceeList()
{
}

CalendarSyncee *SynceeList::calendarSyncee() const
{
  CalendarSyncee *syncee;

  ConstIterator it;
  for( it = begin(); it != end(); ++it ) {
    syncee = dynamic_cast<CalendarSyncee *>( *it );
    if ( syncee ) return syncee;
  }

  return 0;
}

AddressBookSyncee *SynceeList::addressBookSyncee() const
{
  AddressBookSyncee *syncee;

  ConstIterator it;
  for( it = begin(); it != end(); ++it ) {
    syncee = dynamic_cast<AddressBookSyncee *>( *it );
    if ( syncee ) return syncee;
  }

  return 0;
}

EventSyncee *SynceeList::eventSyncee() const
{
  EventSyncee *syncee;

  ConstIterator it;
  for( it = begin(); it != end(); ++it ) {
    syncee = dynamic_cast<EventSyncee *>( *it );
    if ( syncee ) return syncee;
  }

  return 0;
}

TodoSyncee *SynceeList::todoSyncee() const
{
  TodoSyncee *syncee;

  ConstIterator it;
  for( it = begin(); it != end(); ++it ) {
    syncee = dynamic_cast<TodoSyncee *>( *it );
    if ( syncee ) return syncee;
  }

  return 0;
}
