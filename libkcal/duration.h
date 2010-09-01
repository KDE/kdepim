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
#ifndef KCAL_DURATION_H
#define KCAL_DURATION_H

#include <tqdatetime.h>

#include "libkcal_export.h"

namespace KCal {

/**
  This class represents a duration.
*/
class LIBKCAL_EXPORT Duration
{
  public:
    /**
      The unit of time used to define the duration.
    */
    enum Type {
      Seconds,   /**< duration is a number of seconds */
      Days       /**< duration is a number of days */
    };

      /**
      Constructs a duration of 0 seconds.
    */
    Duration();

    /**
      Constructs a duration from @p start to @p end.

      If the time of day in @p start and @p end is equal, and their time
      specifications (i.e. time zone etc.) are the same, the duration will be
      set in terms of days. Otherwise, the duration will be set in terms of
      seconds.

      @param start is the time the duration begins.
      @param end is the time the duration ends.
    */
    Duration( const TQDateTime &start, const TQDateTime &end );

    /**
      Constructs a duration from @p start to @p end.

      If @p type is Days, and the time of day in @p start's time zone differs
      between @p start and @p end, the duration will be rounded down to the
      nearest whole number of days.

      @param start is the time the duration begins.
      @param end is the time the duration ends.
      @param type the unit of time to use (seconds or days)
    */
    Duration( const TQDateTime &start, const TQDateTime &end, Type type );

    /**
      Constructs a duration with a number of seconds or days.

      @param duration the number of seconds or days in the duration
      @param type the unit of time to use (seconds or days)
    */
    Duration( int duration, Type type = Seconds ); //krazy:exclude=explicit

    /**
      Constructs a duration by copying another duration object.

      @param duration is the duration to copy.
    */
    Duration( const Duration &duration );

    /**
      Sets this duration equal to @p duration.

      @param duration is the duration to copy.
    */
    Duration &operator=( const Duration &duration );

    /**
      Returns true if this duration is non-zero.
    */
    operator bool() const;

    /**
      Returns true if this duration is zero.
    */
    bool operator!() const  { return !operator bool(); }

    /**
      Returns true if this duration is smaller than the @p other.
      @param other is the other duration to compare.
    */
    bool operator<( const Duration &other ) const;

    /**
      Returns true if this duration is smaller than or equal to the @p other.
      @param other is the other duration to compare.
    */
    bool operator<=( const Duration &other ) const
    { return !other.operator<( *this ); }


    /**
      Returns true if this duration is greater than the @p other.
      @param other is the other duration to compare.
    */
    bool operator>( const Duration &other ) const
    { return other.operator<( *this ); }

    /**
      Returns true if this duration is greater than or equal to the @p other.
      @param other is the other duration to compare.
    */
    bool operator>=( const Duration &other ) const
    { return !operator<( other ); }

    /**
      Returns true if this duration is equal to the @p other.
      Daily and non-daily durations are always considered unequal, since a
      day's duration may differ from 24 hours if it happens to span a daylight
      saving time change.
      @param other the other duration to compare
    */
    bool operator==( const Duration &other ) const;

    /**
      Returns true if this duration is not equal to the @p other.
      Daily and non-daily durations are always considered unequal, since a
      day's duration may differ from 24 hours if it happens to span a daylight
      saving time change.
      @param other is the other duration to compare.
    */
    bool operator!=( const Duration &other ) const
    { return !operator==( other ); }

    /**
      Adds another duration to this one.
      If one is in terms of days and the other in terms of seconds,
      the result is in terms of seconds.
      @param other the other duration to add
    */
    Duration &operator+=( const Duration &other );

    /**
      Adds two durations.
      If one is in terms of days and the other in terms of seconds,
      the result is in terms of seconds.

      @param other the other duration to add
      @return combined duration
    */
    Duration operator+( const Duration &other ) const
    { return Duration( *this ) += other; }

    /**
      Returns the negative of this duration.
    */
    Duration operator-() const;

    /**
      Subtracts another duration from this one.
      If one is in terms of days and the other in terms of seconds,
      the result is in terms of seconds.

      @param other the other duration to subtract
    */
    Duration &operator-=( const Duration &other );

    /**
      Returns the difference between another duration and this.
      If one is in terms of days and the other in terms of seconds,
      the result is in terms of seconds.

      @param other the other duration to subtract
      @return difference in durations
    */
    Duration operator-( const Duration &other ) const
    { return Duration( *this ) += other; }

    /**
      Multiplies this duration by a value.
      @param value value to multiply by
    */
    Duration &operator*=( int value );

    /**
      Multiplies a duration by a value.

      @param value value to multiply by
      @return resultant duration
    */
    Duration operator*( int value ) const
    { return Duration( *this ) *= value; }

    /**
      Divides this duration by a value.
      @param value value to divide by
    */
    Duration &operator/=( int value );

    /**
      Divides a duration by a value.

      @param value value to divide by
      @return resultant duration
    */
    Duration operator/( int value ) const
    { return Duration( *this ) /= value; }

    /**
      Computes a duration end time by adding the number of seconds or
      days in the duration to the specified @p start time.

      @param start is the start time.
      @return end time.
    */
    TQDateTime end( const TQDateTime &start ) const;

    /**
      Returns the time units (seconds or days) used to specify the duration.
    */
    Type type() const;

    /**
      Returns whether the duration is specified in terms of days rather
      than seconds.
    */
    bool isDaily() const;

    /**
      Returns the length of the duration in seconds.
    */
    int asSeconds() const;

    /**
      Returns the length of the duration in days. If the duration is
      not an exact number of days, it is rounded down to return the
      number of whole days.
    */
    int asDays() const;

    /**
      Returns the length of the duration in seconds or days.

      @return if isDaily(), duration in days, else duration in seconds
    */
    int value() const;

  private:
    int seconds() const { return mDaily ? mDuration * 86400 : mDuration; }
    int mDuration;
    bool mDaily;

    class Private;
    Private *d;
};

}

#endif
