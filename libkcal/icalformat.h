/*
    This file is part of libkcal.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef ICALFORMAT_H
#define ICALFORMAT_H
// $Id$

#include <qstring.h>

#include "scheduler.h"

#include "calformat.h"

namespace KCal {

class ICalFormatImpl;

/**
  This class implements the iCalendar format. It provides methods for
  loading/saving/converting iCalendar format data into the internal KOrganizer
  representation as Calendar and Events.

  @short iCalendar format implementation
*/
class ICalFormat : public CalFormat {
  public:
    /** Create new iCalendar format. */
    ICalFormat();
    virtual ~ICalFormat();

    /**
      Loads a calendar on disk in iCalendar format into calendar.
      Returns true if successful, else returns false. Provides more error
      information by exception().
      @param calendar Calendar object to be filled.
      @param fileName The name of the calendar file on disk.
    */
    bool load( Calendar *, const QString &fileName );
    /**
      Writes out the calendar to disk in iCalendar format. Returns true if
     successful and false on error.
     
     @param calendar The Calendar object to be written.
     @param fileName The name of the calendar file on disk.
    */
    bool save( Calendar *, const QString &fileName );

    /**
      Parse string and populate calendar with that information.
    */
    bool fromString( Calendar *, const QString & );  
    /**
      Return calendar information as string.
    */
    QString toString( Calendar * );
  
    /** Create a scheduling message for event \a e using method \m */
    QString createScheduleMessage(IncidenceBase *e,Scheduler::Method m);
    /** Parse scheduling message provided as string \s */
    ScheduleMessage *parseScheduleMessage( Calendar *, const QString &s);
    
    /** Set id of used time zone and whether this time zone is UTC or not. */
    void setTimeZone( const QString &id, bool utc );
    QString timeZoneId() const;
    bool utc() const;

  private:
    ICalFormatImpl *mImpl;

    QString mTimeZoneId;
    bool mUtc;
};

}

#endif
