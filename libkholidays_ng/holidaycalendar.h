/*
    This file is part of libkholidays.
    Copyright (c) 2004 Allen Winter <winter@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef KHOLIDAYS_HOLIDAYCALENDAR_H
#define KHOLIDAYS_HOLIDAYCALENDAR_H

#include <qstring.h>
#include <qdatetime.h>

#include "holiday.h"

namespace KHolidays {

/**
   Sort direction.
*/
enum SortDirection {
  SortDirectionAscending,
  SortDirectionDescending
};

/**
   How Holidays are to be sorted.
*/
enum HolidaySortField {
  HolidaySortUnsorted,
  HolidaySortDate,
  HolidaySortName
};

/**
  A class for Holiday Calendar handling.
*/

class HolidayCalendar {
  public:
    HolidayCalendar();
    ~HolidayCalendar();

    /**
       Add a Holiday to the calendar.
    */
    bool add( Holiday *holiday );
    /**
       Remove a Holiday from the calendar.
    */
    void remove( Holiday *holiday );
    /**
       Removes all Holiday from the calendar.
    */
    void erase();

    /**
       Dedupes (removes duplicate Holidays) a Holiday List.
    */
    static Holiday::List dedupe( Holiday::List *holidayList );
    /**
       Sorts a Holiday List according to sortField and sortDirection.
    */
    static Holiday::List sort( Holiday::List *holidayList,
                               HolidaySortField sortField,
                               SortDirection sortDirection );

    /**
       Retrieves a Holiday on the basis of the name.
    */
    Holiday *holiday( const QString &name );
    /**
       Returns a list of all Holidays in the calendar.
    */
    Holiday::List rawList(
      HolidaySortField sortField = HolidaySortDate,
      SortDirection sortDirection = SortDirectionAscending );
    /**
       Returns a list of all Holidays that occur on the specified date.
    */
    Holiday::List rawListForDate(
      const QDate &date,
      HolidaySortField sortField = HolidaySortDate,
      SortDirection sortDirection = SortDirectionAscending );
    /**
       Returns a list of all Holidays that occur within the
       date range specified.
    */
    Holiday::List rawListForRange(
      const QDate &start,
      const QDate &end,
      HolidaySortField sortField = HolidaySortDate,
      SortDirection sortDirection = SortDirectionAscending );
    /**
       Returns a list of all Holidays that occur within the specified year.
    */
    Holiday::List rawListForYear(
      const int &year,
      HolidaySortField sortField = HolidaySortDate,
      SortDirection sortDirection = SortDirectionAscending );

    /**
       Returns a filtered list of all Holidays.
    */
    Holiday::List list(
      HolidaySortField sortField = HolidaySortDate,
      SortDirection sortDirection = SortDirectionAscending );
    /**
       Returns a filtered list of all Holidays that occur on the specified date.
    */
    Holiday::List listForDate(
      const QDate &date,
      HolidaySortField sortField = HolidaySortDate,
      SortDirection sortDirection = SortDirectionAscending );
    /**
       Returns a filtered list of all Holidays that occur within
       the specified date range.
    */
    Holiday::List listForRange(
      const QDate &start,
      const QDate &end,
      HolidaySortField sortField = HolidaySortDate,
      SortDirection sortDirection = SortDirectionAscending );
    /**
       Returns a filtered list of all Holidays that occur within
       the specified year.
    */
    Holiday::List listForYear(
      const int &year,
      HolidaySortField sortField = HolidaySortDate,
      SortDirection sortDirection = SortDirectionAscending );

  /****** The methods below may or may not be needed -- don't know yet *****/

  /**
     Calculates Christmas date in the Gregorian calendar
     @param year to compute the date for
  */
  QDate christmas( int year );

  /**
     Calculates Easter date in the Gregorian calendar
     @param year to compute the date for

     Code by Rich Bowen <rbowen@rcbowen.com>

     See also http://www.pauahtun.org/CalendarFAQ/cal/node3.html
     for more details on calculating Easter.
   */
    QDate easter( int year );
  /**
     Calculates Easter date in the Greek Orthodox calendar for a given year.
     @param year to compute the date for

     Code by Pascalis Ligdas, based on original code by Apostolos Syropoulos.
  */
    QDate orthodoxEaster( int year );

  private:
    class Private;
    Private *d;
};

}

#endif
