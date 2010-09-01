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
#ifndef KCAL_PERIOD_H
#define KCAL_PERIOD_H

#include <tqdatetime.h>
#include "libkcal_export.h"

#include "duration.h"

namespace KCal {

/**
  This class represents a period of time. The period can be defined by either a
  start time and an end time or by a start time and a duration.
*/
class KDE_EXPORT Period
{
  public:
    Period();
    Period( const TQDateTime &start, const TQDateTime &end );
    Period( const TQDateTime &start, const Duration &duration );

    /** Returns true if this element is smaller than the @param other one */
    bool operator<( const Period& other );

    /**
      Returns true if this period is equal to the @p other one.
      Even if their start and end times are the same, two periods are
      considered not equal if one is defined in terms of a duration and the
      other in terms of a start and end time.

      @param other the other period to compare
    */
    bool operator==( const Period &other ) const;

    /**
      Returns true if this period is not equal to the @p other one.

      @param other the other period to compare
      @see operator==()
    */
    bool operator!=( const Period &other ) const  { return !operator==( other ); }

    TQDateTime start() const;
    TQDateTime end() const;
    Duration duration();

    bool hasDuration()const;

    TQString summary() const;
    void setSummary( const TQString &summary );
    TQString location() const;
    void setLocation( const TQString &location );

  private:
    TQDateTime mStart;
    TQDateTime mEnd;

    bool mHasDuration;
    TQString mSummary;
    TQString mLocation;

    class Private;
    Private *d;
};

}

#endif
