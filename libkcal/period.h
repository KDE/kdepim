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
#ifndef KCAL_PERIOD_H
#define KCAL_PERIOD_H

#include <qdatetime.h>
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
    Period( const QDateTime &start, const QDateTime &end );
    Period( const QDateTime &start, const Duration &duration );

    QDateTime start() const;
    QDateTime end() const;
    Duration duration();

    bool hasDuration()const;

  private:
    QDateTime mStart;
    QDateTime mEnd;

    bool mHasDuration;

    class Private;
    Private *d;
};

}

#endif
