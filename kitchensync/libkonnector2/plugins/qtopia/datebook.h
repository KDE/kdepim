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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef OpieHelperDateBook_H
#define OpieHelperDateBook_H

#include <qptrlist.h>
#include <libkcal/event.h>

#include "helper.h"

namespace KSync{
    class CalendarSyncee;
}
class QDomElement;
namespace OpieHelper {
    enum Days { Monday=1,
                Tuesday =2,
                Wednesday =4,
                Thursday = 8,
                Friday = 16,
                Saturday = 32,
                Sunday = 64 };

    class DateBook : public Base {
    public:
        DateBook( CategoryEdit* edit = 0,
                  KSync::KonnectorUIDHelper* helper = 0,
                  const QString &tz = QString::null,
                  Device* dev = 0);
        ~DateBook();
        bool toKDE( const QString & fileName, ExtraMap& map, KSync::CalendarSyncee* );
        KTempFile* fromKDE( KSync::CalendarSyncee* syncee, ExtraMap& map );

    private:
        static QStringList supportedAttributes();
        QString endDate( const QDateTime& time, bool allDay );
        QString startDate( const QDateTime& time, bool allDay );
        QString event2string( KCal::Event *event, ExtraMap& );
        KCal::Event* toEvent( QDomElement, ExtraMap&, const  QStringList& lst );
    };
}
#endif
