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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "synceelist.h"

#include "calendarsyncee.h"
#include "addressbooksyncee.h"
#include "bookmarksyncee.h"
#include "unknownsyncee.h"

using namespace KSync;

SynceeList::SynceeList()
{
}

SynceeList::~SynceeList()
{
}

CalendarSyncee *SynceeList::calendarSyncee() const
{
  return templateSyncee<CalendarSyncee>();
}

AddressBookSyncee *SynceeList::addressBookSyncee() const
{
  return templateSyncee<AddressBookSyncee>();
}

BookmarkSyncee *SynceeList::bookmarkSyncee() const
{
  return templateSyncee<BookmarkSyncee>();
}

UnknownSyncee *SynceeList::unknownSyncee()const
{
  return templateSyncee<UnknownSyncee>();
}

template<class T>
T  *SynceeList::templateSyncee()const
{
  T *syncee;

  ConstIterator it;
  for( it = begin(); it != end(); ++it ) {
    syncee = dynamic_cast<T*>( *it );
    if ( syncee ) return syncee;
  }

  return 0;
}

/**
 * This method will call \sa clear but also
 * delete the contained Syncees.
 */
void SynceeList::deleteAndClear()
{
  for ( Iterator it = begin(); it != end(); ++it )
    delete *it;
  clear();
}

