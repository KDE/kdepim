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
#ifndef KSYNC_SYNCEELIST_H
#define KSYNC_SYNCEELIST_H

#include <qvaluelist.h>

#include "syncee.h"

namespace KSync {

class CalendarSyncee;
class AddressBookSyncee;
class BookmarkSyncee;
class UnknownSyncee;

/**
  This class provides a list of Syncees.
*/
class KDE_EXPORT SynceeList : public QValueList<Syncee *>
{
  public:
    SynceeList();
    ~SynceeList();

    CalendarSyncee *calendarSyncee() const;
    AddressBookSyncee *addressBookSyncee() const;
    BookmarkSyncee *bookmarkSyncee() const;
    UnknownSyncee  *unknownSyncee()const;

    template<class T>
    T              *templateSyncee()const;

    void deleteAndClear();
};

}

#endif
