/*
  This file is part of libkcal.

  Copyright (c) 2009 Klar�lvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

  @author Allen Winter \<allen@kdab.net\>
*/

#ifndef KCAL_CALHELPER_H
#define KCAL_CALHELPER_H

class QString;

namespace KCal {
class Calendar;
class Incidence;

/**
  @brief
  Provides methods for making decisions about calendar data.
*/
namespace CalHelper {

  /**
    Determine if the specified incidence is likely a Kolab incidence
    owned by the the user.

    @param calendar is a pointer to a valid Calendar object.
    @param incidence is a pointer to an Incidence object.

    @return true if it is likely that the specified incidence belongs
    to the user in their Kolab resource; false otherwise.
  */
  bool isMyKolabIncidence( Calendar *calendar, Incidence *incidence );

  /**
    Determine if the specified incidence is likely owned by the the user,
    independent of the Resource type.

    @param calendar is a pointer to a valid Calendar object.
    @param incidence is a pointer to an Incidence object.

    @return true if it is likely that the specified incidence belongs
    to the user; false otherwise.
  */
  bool isMyCalendarIncidence( Calendar *calendar, Incidence *incidence );

  /**
    Searches for the specified Incidence by UID, returning an Incidence pointer
    if and only if the found Incidence is owned by the user.

    @param calendar is a pointer to a valid Calendar object.
    @param Uid is a QString containing an Incidence UID.

    @return a pointer to the Incidence found; 0 if the Incidence is not found
    or the Incidence is found but is not owned by the user.
  */
  Incidence *findMyCalendarIncidenceByUid( Calendar *calendar, const QString &uid );

  /**
    Determines if the Calendar is using a Groupware resource type.
    @param calendar is a pointer to a valid Calendar object.

    @return true if the Calendar is using a known Groupware resource type;
    false otherwise.
    @since 4.4
  */
  bool usingGroupware( Calendar *calendar );

  /**
    Determines if the Calendar has any writable folders with Events content
    that are owned by me.
    @param family is the resource family name or "calendar" if empty.

    @return true if the any such writable folders are found; false otherwise.
    @since 4.5
  */
  bool hasMyWritableEventsFolders( const QString &family );
}

}

#endif

