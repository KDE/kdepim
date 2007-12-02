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

#include <kdebug.h>
#include <klocale.h>

#include "period.h"

using namespace KCal;

Period::Period()
{
  mHasDuration = false;
}

Period::Period( const QDateTime &start, const QDateTime &end )
{
  mStart = start;
  mEnd = end;
  mHasDuration = false;
}

Period::Period( const QDateTime &start, const Duration &duration )
{
  mStart = start;
  mEnd = duration.end( start );
  mHasDuration = true;
}


bool Period::operator<( const Period& other )
{
  return start() < other.start();
}

QDateTime Period::start() const
{
  return mStart;
}

QDateTime Period::end()const
{
  return mEnd;
}

Duration Period::duration()
{
  return Duration( mStart, mEnd );
}

bool Period::hasDuration()const
{
  return mHasDuration;
}

QString KCal::Period::summary() const
{
  return mSummary;
}

void KCal::Period::setSummary(const QString & summary)
{
  mSummary = summary;
}

QString KCal::Period::location() const
{
  return mLocation;
}

void KCal::Period::setLocation(const QString & location)
{
  mLocation = location;
}
