/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
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

#include "vcaldrag.h"

#include "vcalformat.h"

using namespace KCal;

VCalDrag::VCalDrag( Calendar *cal, QWidget *parent, const char *name )
  : QStoredDrag( "text/x-vCalendar", parent, name )
{
  VCalFormat format;
  setEncodedData( format.toString( cal ).utf8() );
}

bool VCalDrag::canDecode( QMimeSource *me )
{
  return me->provides( "text/x-vCalendar" );
}

bool VCalDrag::decode( QMimeSource *de, Calendar *cal )
{
  bool success = false;

  QByteArray payload = de->encodedData( "text/x-vCalendar" );
  if ( payload.size() ) {
    QString txt = QString::fromUtf8( payload.data() );

    VCalFormat format;
    success = format.fromString( cal, txt );
  }

  return success;
}

