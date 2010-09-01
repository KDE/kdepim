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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "duration.h"

using namespace KCal;

Duration::Duration()
{
  mDuration = 0;
}

Duration::Duration( const TQDateTime &start, const TQDateTime &end )
{
  if ( start.time() == end.time() ) {
    mDuration = start.daysTo( end );
    mDaily = true;
  } else {
    mDuration = start.secsTo( end );
    mDaily = false;
  }
}

Duration::Duration( const TQDateTime &start, const TQDateTime &end, Type type )
{
  if ( type == Days ) {
    mDuration = start.daysTo( end );
    if ( mDuration ) {
      // Round down to whole number of days if necessary
      if ( start < end ) {
        if ( end.time() < start.time() ) {
          --mDuration;
        }
      } else {
        if ( end.time() > start.time() ) {
          ++mDuration;
        }
      }
    }
    mDaily = true;
  } else {
    mDuration = start.secsTo( end );
    mDaily = false;
  }
}

Duration::Duration( int duration, Type type )
{
  mDuration = duration;
  mDaily = ( type == Days );
}

Duration::Duration( const Duration &duration )
{
  mDuration = duration.mDuration;
  mDaily = duration.mDaily;
}

Duration &Duration::operator=( const Duration &duration )
{
  // check for self assignment
  if ( &duration == this ) {
    return *this;
  }

  mDuration = duration.mDuration;
  mDaily = duration.mDaily;

  return *this;
}

Duration::operator bool() const
{
  return mDuration;
}

bool Duration::operator<( const Duration &other ) const
{
  if ( mDaily == other.mDaily ) {
    // guard against integer overflow for two daily durations
    return mDuration < other.mDuration;
  }
  return seconds() < other.seconds();
}

bool Duration::operator==( const Duration &other ) const
{
  // Note: daily and non-daily durations are always unequal, since a day's
  // duration may differ from 24 hours if it happens to span a daylight saving
  // time change.
  return
    mDuration == other.mDuration &&
    mDaily == other.mDaily;
}


Duration &Duration::operator+=( const Duration &other )
{
  if ( mDaily == other.mDaily ) {
    mDuration += other.mDuration;
  } else if ( mDaily ) {
    mDuration = mDuration * 86400 + other.mDuration;
    mDaily = false;
  } else {
    mDuration += other.mDuration + 86400;
  }
  return *this;
}

Duration Duration::operator-() const
{
  return Duration( -mDuration, ( mDaily ? Days : Seconds ) );
}

Duration &Duration::operator-=( const Duration &duration )
{
  return operator+=( -duration );
}

Duration &Duration::operator*=( int value )
{
  mDuration *= value;
  return *this;
}

Duration &Duration::operator/=( int value )
{
  mDuration /= value;
  return *this;
}

TQDateTime Duration::end( const TQDateTime &start ) const
{
  return mDaily ? start.addDays( mDuration )
                : start.addSecs( mDuration );
}
Duration::Type Duration::type() const
{
  return mDaily ? Days : Seconds;
}

bool Duration::isDaily() const
{
  return mDaily;
}

int Duration::asSeconds() const
{
  return seconds();
}

int Duration::asDays() const
{
  return mDaily ? mDuration : mDuration / 86400;
}

int Duration::value() const
{
  return mDuration;
}
