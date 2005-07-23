/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

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

#ifndef OPIE_META_CALENDAR_H
#define OPIE_META_CALENDAR_H

#include <calendarsyncee.h>

#include <libkcal/todo.h>
#include <libkcal/event.h>

#include "md5metatemplate.h"

namespace OpieHelper {

class MetaCalendar : public MD5Template<KSync::CalendarSyncee, KSync::CalendarSyncEntry> 
{
public:
    MetaCalendar( KSync::CalendarSyncee* sync, const QString& file );
    ~MetaCalendar();

protected:
    QString entryToString( KSync::CalendarSyncEntry* );
private:
    QString todoToString ( KCal::Todo* );
    QString eventToString( KCal::Event* );
    static QString days( const QBitArray& ar );
};

}

#endif
