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
#ifndef KCAL_EVENT_H
#define KCAL_EVENT_H

#include "incidence.h"
#include <kdepimmacros.h>

namespace KCal {

/**
  This class provides an Event in the sense of RFC2445.
*/
class KDE_EXPORT Event : public Incidence
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
      Return end time as string formatted according to the users locale
      settings.
    */
    QString dtEndTimeStr() const;
    /**
      Return end date as string formatted according to the users locale
      settings.
      
      @param shortfmt if true return string in short format, if false return
                      long format
    */
    QString dtEndDateStr( bool shortfmt = true ) const;
    /**
      Return end date and time as string formatted according to the users locale
      settings.
    */
    QString dtEndStr() const;

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
