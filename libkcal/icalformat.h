/*
    This file is part of libkcal.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_ICALFORMAT_H
#define KCAL_ICALFORMAT_H

#include <qstring.h>

#include "scheduler.h"

#include "calformat.h"
#include <kdepimmacros.h>

namespace KCal {

class ICalFormatImpl;
class FreeBusy;

/**
  This class implements the iCalendar format. It provides methods for
  loading/saving/converting iCalendar format data into the internal KOrganizer
  representation as Calendar and Events.

  @short iCalendar format implementation
*/
class LIBKCAL_EXPORT ICalFormat : public CalFormat
{
  public:
    ICalFormat();
    virtual ~ICalFormat();

    /**
      Loads a calendar on disk in iCalendar format into calendar.
      Returns true if successful, else returns false. Provides more error
      information by exception().

      @param calendar Calendar object to be filled.
      @param fileName The name of the calendar file on disk.
    */
    bool load( Calendar * calendar, const QString &fileName );
    /**
      Writes out the calendar to disk in iCalendar format. Returns true if
      successful and false on error.
     
      @param calendar The Calendar object to be written.
      @param fileName The name of the calendar file on disk.
    */
    bool save( Calendar * calendar, const QString &fileName );

    /**
      Parse string and populate calendar with that information.
    */
    bool fromString( Calendar * calendar, const QString & );

    /**
      Parse string and return first ical component.
    */
    Incidence *fromString( const QString & );

    /**
      Return calendar information as string.
    */
    QString toString( Calendar * );
    /**
      Return incidence as full iCalendar formatted text.
    */
    QString toICalString( Incidence * );
    /**
      Return incidence as iCalendar formatted text.
    */
    QString toString( Incidence * );
    /**
      Return recurrence as iCalendar formatted text.
    */
    QString toString( Recurrence * );
    /**
      Parse string and fill recurrence object with
      that information
    */
    bool fromString ( Recurrence *, const QString& );
  
    /**
      Create a scheduling message for event @p e using method @p m.
    */
    QString createScheduleMessage(IncidenceBase *e,Scheduler::Method m);
    /**
      Parse scheduling message provided as string @p s.
    */
    ScheduleMessage *parseScheduleMessage( Calendar *, const QString &s);

    /**
      Parse FREEBUSY object.
    */
    FreeBusy *parseFreeBusy( const QString & );

    /**
      Set id of used time zone and whether this time zone is UTC or not.
    */
    void setTimeZone( const QString &id, bool utc );
    /**
      Return id string of timezone used.
    */
    QString timeZoneId() const;
    /**
      Return true if timezone used is UTC, otherwise return false.
    */
    bool utc() const;

  protected:
    /**
      Parse string and return first ical component of a raw byte array of 
      a utf8 encoded string. This is an overload used for efficiency reading
      to avoid utf8 conversions, which are expensive, when reading from disk.
    */
    bool fromRawString( Calendar * calendar, const QCString & );

  private:
    ICalFormatImpl *mImpl;

    QString mTimeZoneId;
    bool mUtc;

    class Private;
    Private *d;
};

}

#endif
