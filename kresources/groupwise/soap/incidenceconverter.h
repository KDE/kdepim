/*
    This file is part of kdepim.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_GW_INCIDENCECONVERTER_H
#define KABC_GW_INCIDENCECONVERTER_H

#include <libkcal/event.h>
#include <libkcal/todo.h>

#include <time.h>

#include "gwconverter.h"

class ngwt__Recipient;

class IncidenceConverter : public GWConverter
{
  public:
    IncidenceConverter( struct soap* );

    void setFrom( const QString &name, const QString &email,
      const QString &uuid );

    KCal::Event* convertFromAppointment( ngwt__Appointment* );
    ngwt__Appointment* convertToAppointment( KCal::Event* );

    KCal::Todo* convertFromTask( ngwt__Task* );
    ngwt__Task* convertToTask( KCal::Todo* );

    KCal::Journal* convertFromNote( ngwt__Note* note);
    ngwt__Note* convertToNote( KCal::Journal* journal );

  private:
    bool convertToCalendarItem( KCal::Incidence*, ngwt__CalendarItem* );
    bool convertFromCalendarItem( ngwt__CalendarItem*, KCal::Incidence* );

    void getItemDescription( ngwt__CalendarItem*, KCal::Incidence* );
    void setItemDescription( KCal::Incidence*, ngwt__CalendarItem* );

    void getAttendees( ngwt__CalendarItem*, KCal::Incidence* );
    void setAttendees( KCal::Incidence *, ngwt__CalendarItem * );

    void getRecurrence( ngwt__CalendarItem*, KCal::Incidence* );
    void setRecurrence( KCal::Incidence *, ngwt__CalendarItem * );

    // used for converting weekly recurrences from GW
//     QBitArray getDayBitArray( ngwt__DayOfWeekList * );

    ngwt__Recipient *createRecipient( const QString &name,
      const QString &email, const QString &uuid = QString::null );

    QString mTimezone;

    QString mFromName;
    QString mFromEmail;
    QString mFromUuid;
};

#endif
