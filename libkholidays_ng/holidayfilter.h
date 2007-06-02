/*
    This file is part of libkholidays.
    Copyright (c) 2004,2006 Allen Winter <winter@kde.org>

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

#ifndef KHOLIDAYS_HOLIDAYFILTER_H
#define KHOLIDAYS_HOLIDAYFILTER_H

#include "kholidays_export.h"
#include "holiday.h"

namespace LibKHolidays {

/**
   A class for filtering Holidays.
*/
class KHOLIDAYS_EXPORT HolidayFilter
{
  public:
  /**
     Create the filter.
  */
  HolidayFilter();
  /**
     Create a named filter.
  */
  HolidayFilter( const QString &name );
  /**
     Destroy the filter.
  */
  ~HolidayFilter();

  /**
     Set filter name.
  */
  void setName( const QString &name );
  /**
     Return filter name.
  */
  QString name();

  /**
     Apply filter to a list of Holidays.  All holidays not matching
     the filter criteria are removed from the list.
  */
  apply( Holiday::List *holidayList ) const;

  /**
     Apply filter criteria to the specified Holiday.
     Return true if the Holiday passes the criteria; otherwise return false.
  */
  bool filterHoliday( Holiday * ) const;

  /**
     Enable or disable filter.
  */
  void setEnabled( bool );
  /**
     Return whether the filter is enabled or not.
  */
  bool isEnabled();

  /**
     Set Holiday types, which is used for showing/hiding types of Holidays.
  */
  void setTypes( const QStringList & );
  /**
     Return types list, used for showing/hiding types of Holidays.
  */
  QStringList typesList() const;

  /**
     Set criteria which must be fulfilled by Holidays passing the filter.
  */
  void setCriteria( int criteria );
  /**
     Get inclusive filter criteria.
  */
  int criteria() const;

  private:
    QString mName;
    bool mEnabled;
    int mCriteria;
    QStringList mTypeList;

    clase Private;
    Private *d;
};

}

#endif
