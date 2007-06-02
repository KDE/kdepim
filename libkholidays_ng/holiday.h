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

#ifndef KHOLIDAYS_HOLIDAY_H
#define KHOLIDAYS_HOLIDAY_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QDateTime>

#include "kholidays_export.h"

/**
  Represents and manages a Holiday

  A Holiday is any day that may have special significance to a fairly large
  number of people.  A "holy day" can be a holiday, a "day-off" for
  all public employees can be a holiday.  Also, the remembrance of an
  historical event can be a holiday.

  Types of Holidays:

  + Public: a day-off from work or school (typically) to commemorate
       a historical event, or to honor people.  Public holidays are always
       bounded to a political region.

  + Cultural : a special day in the culture and therefore usually
       bounded to a political region.  A day where people "celebrate"
       in some fashion.  eg. Halloween, St. Valentines, ...

  + Religious: a special day in a religion and not bounded to any
       political region.  Religions can have holidays that range in
       devotion to those were work/school are forbidden, days of
       prayer obligation, to historical remembrances, etc.
       Governments often have public holidays for some religious holidays.

  + Financial: banks and markets may close on some public holidays.
       it would be useful for financial applications to know when.
       Financial holidays are tied to public holidays and therefore are
       always bounded to a political region.

  + Noteworthy Event: a day of interest.  eg. Daylight Savings Time Start/Stop,
       Election Day, ...

*/

namespace LibKHolidays {

class KHOLIDAYS_EXPORT Holiday
{
  public:

    typedef QList<Holiday> List;

    Holiday();
    Holiday( const Holiday & );
    ~Holiday();
    bool operator==( const Holiday & ) const;

    /**
      Return copy of this Holiday. The caller owns the returned objet.
    */
    Holiday *clone();

    /**
      Set holiday name.
    */
    void setName( const QString &name );
    /**
      Return holiday name.
    */
    QString name() const;
    /**
      Set holiday date.
    */
    void setDate( const QDate &date );
    /**
      Return holiday date.
    */
    QDate date() const;
    /**
      Return date as string formatted according to the users locale settings.
    */
    QString dateStr() const;
    /**
      Return date as string formatted according to the users locale settings.

      @param shortfmt if true return string in short format, if false return
                      long format
    */
    QString dateStr( bool shortfmt = true ) const;

  //TODO: use enums, not strings.
  //      if strings are used then we need to validate them
    /**
      Set Holiday types from a QStringList.
    */
    void setTypes( const QStringList &types );
    /**
      Set Holiday types based on a comma delimited string.
    */
    void setTypes(const QString &typeStr);
    /**
      Return Holiday types as a list of strings.
    */
    QStringList types() const;
    /**
      Return Holiday types as a comma separated string.
    */
    QString typesStr() const;

  QString holiday( const QDate &date ) const;
  bool isHoliday( const QDate &date );
  bool isWorkDayOff( const QDate &date );
  bool isSchoolDayOff( const QDate &date );
  bool isBankDayOff( const QDate &date );
  bool isNewMoon( const QDate &date );  //move to a Lunar? class?
  bool isFullMoon( const QDate &date ); //move to a Lunar? class?

  private:
    QString mName;
    QString mDateSpec;
    QDate mDate;
    QString mDescription;
    QStringList mTypes;
    //weekend mover?
    //    datetype? fixed,  floating,  weekend mover, ??
    //  political region? ( if applicable,  i.e. not religious )

    class Private;
    Private *d;
};

}

#endif
