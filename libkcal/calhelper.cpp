/*
  This file is part of the kcal library.

  Copyright (c) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
/**
  @file
  This file is part of the API for handling calendar data and provides
  static convenience functions for making decisions about calendar data.

  @brief
  Provides methods for making decisions about calendar data.

  @author Allen Winter \<allen@kdab.net\>
*/

#include "calhelper.h"
#include "calendarresources.h"

using namespace KCal;

bool CalHelper::isMyKolabIncidence( Calendar *calendar, Incidence *incidence )
{
  CalendarResources *cal = dynamic_cast<CalendarResources*>( calendar );
  if ( !cal || !incidence ) {
    return true;
  }
  ResourceCalendar *res = cal->resource( incidence );
  if ( !res ) {
    return true;
  }
  const QString subRes = res->subresourceIdentifier( incidence );
  if ( !subRes.contains( "/.INBOX.directory/" ) ) {
    return false;
  }
  return true;
}

bool CalHelper::isMyCalendarIncidence( Calendar *calendar, Incidence *incidence )
{
  return isMyKolabIncidence( calendar, incidence );
}


Incidence *CalHelper::findMyCalendarIncidenceByUid( Calendar *calendar, const QString &uid )
{
  // Determine if this incidence is in my calendar (and owned by me)
  Incidence *existingIncidence = 0;
  if ( calendar ) {
    existingIncidence = calendar->incidence( uid );
    if ( !isMyCalendarIncidence( calendar, existingIncidence ) ) {
      existingIncidence = 0;
    }
    if ( !existingIncidence ) {
      const Incidence::List list = calendar->incidences();
      for ( Incidence::List::ConstIterator it=list.begin(), end=list.end(); it!=end; ++it ) {
        if ( (*it)->schedulingID() == uid && isMyCalendarIncidence( calendar, *it ) ) {
          existingIncidence = *it;
          break;
        }
      }
    }
  }
  return existingIncidence;
}
