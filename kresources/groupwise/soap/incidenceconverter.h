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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KABC_GW_INCIDENCECONVERTER_H
#define KABC_GW_INCIDENCECONVERTER_H

#include <libkcal/event.h>
#include <libkcal/todo.h>

#include <time.h>

#include "gwconverter.h"

class IncidenceConverter : public GWConverter
{
  public:
    IncidenceConverter( struct soap* );

    KCal::Event* convertFromAppointment( ns1__Appointment* );
    ns1__Appointment* convertToAppointment( KCal::Event* );

    KCal::Todo* convertFromTask( ns1__Task* );
    ns1__Task* convertToTask( KCal::Todo* );

  private:
    bool convertToCalendarItem( KCal::Incidence*, ns1__CalendarItem* );
    bool convertFromCalendarItem( ns1__CalendarItem*, KCal::Incidence* );

    void getItemDescription( ns1__CalendarItem*, KCal::Incidence* );
    void setItemDescription( KCal::Incidence*, ns1__CalendarItem* );

    void getAttendees( ns1__CalendarItem*, KCal::Incidence* );
    void setAttendees( KCal::Incidence *, ns1__CalendarItem * );

    QString mTimezone;
};

#endif
