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

// $Id$

#include <kdebug.h>
#include <klocale.h>

#include "duration.h"

using namespace KCal;

Duration::Duration()
{
  mSeconds = 0;
}

Duration::Duration( const QDateTime &start, const QDateTime &end )
{
  mSeconds = start.secsTo( end );
}

Duration::Duration( int seconds )
{
  mSeconds = seconds;
}

QDateTime Duration::end( const QDateTime &start ) const
{
  return start.addSecs( mSeconds );
}

int Duration::asSeconds() const
{
  return mSeconds;
}
