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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KCAL_EVENT_H
#define KCAL_EVENT_H

#include "incidence.h"
#include <kdepimmacros.h>

namespace KCal {

/**
  This class provides an Event in the sense of RFC2445.
*/
class LIBKCAL_EXPORT Event : public Incidence
{
  public:
    /**
      Transparency of event.

      Opaque      - event appears in free/busy time
      Transparent - event doesn't appear in free/busy time
    */
    enum Transparency { Opaque, Transparent };

    typedef ListBase<Event> List;

    Event();
    Event( const Event & );
    ~Event();
    Event& operator=( const Event &e );
    bool operator==( const Event & ) const;

    QCString type() const { return "Event"; }

    /**
      Return copy of this Event. The caller owns the returned objet.
    */
    Event *clone();

    /**
      Set end date and time.
    */
    void setDtEnd(const QDateTime &dtEnd);
    /**
      Return end date and time.
    */
    virtual QDateTime dtEnd() const;
    /**
      Returns the day when the event ends. This might be different from
      dtEnd().date, since the end date/time is non-inclusive. So timed events
      ending at 0:00 have their end date on the day before.
    */
    QDate dateEnd() const;
    /**
      Return end time as string formatted according to the users locale
      settings.
      @deprecated use IncidenceFormatter::timeToString()
    */
    QString KDE_DEPRECATED dtEndTimeStr() const;
    /**
      Return end date as string formatted according to the users locale
      settings.

      @param shortfmt if true return string in short format, if false return
                      long format
      @deprecated use IncidenceFormatter::dateToString()
    */
    QString KDE_DEPRECATED dtEndDateStr( bool shortfmt = true ) const;
    /**
      Return end date and time as string formatted according to the users locale
      settings.
      @deprecated use IncidenceFormatter::dateTimeToString()
    */
    QString KDE_DEPRECATED dtEndStr() const;

    /**
      Set whether the event has an end date/time.
    */
    void setHasEndDate(bool);
    /**
      Return whether the event has an end date/time.
    */
    bool hasEndDate() const;

    /**
      Return true if the event spans multiple days, otherwise return false.
    */
    bool isMultiDay() const;

    /**
      Set the event's time transparency level.
    */
    void setTransparency( Transparency transparency );
    /**
      Return the event's time transparency level.
    */
    Transparency transparency() const;

    /**
      Set duration of this event.
    */
    void setDuration( int seconds );

    /** Check if an incidence slices an interval.
     *
     * An event slices an interval if it either starts before
     * this interval ( and has not ended before the interval )
     * or if it has a recurrance that starts in the interval and
     * has not ended.
     *
     * @param start Start time of the interval
     * @param end End time of the interval
     *
     * @returns true if the event in this interval.
     */
    bool slicesInterval( const QDateTime& startDt, const QDateTime& endDt );

  protected:
    /** Return the end date/time of the base incidence. */
    virtual QDateTime endDateRecurrenceBase() const { return dtEnd(); }
  private:
    bool accept( Visitor &v ) { return v.visit( this ); }

    QDateTime mDtEnd;
    bool mHasEndDate;
    Transparency mTransparency;

    class Private;
    Private *d;
};

}

#endif
