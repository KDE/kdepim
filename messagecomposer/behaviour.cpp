/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "behaviour.h"

using namespace MessageComposer;

Behaviour::Behaviour()
  : d( new Private )
{
  // Default behaviour for sending.
  d->actions[ UseGui ] = true;
  d->actions[ UseCrypto ] = true;
  d->actions[ UseWrapping ] = true;
  d->actions[ UseFallbackCharset ] = false;
  d->actions[ WarnBadCharset ] = true;
  d->actions[ WarnZeroRecipients ] = true;
  d->actions[ CustomHeaders ] = false;
}

Behaviour::~Behaviour()
{
  // d is a QSharedDataPointer.
}

bool Behaviour::isActionEnabled( Action action ) const
{
  Q_ASSERT( action >= 0 && action < LastAction );
  return d->actions[ action ];
}

void Behaviour::enableAction( Action action, bool enable )
{
  Q_ASSERT( action >= 0 && action < LastAction );
  d->actions[ action ] = enable;
}

void Behaviour::disableAction( Action action )
{
  Q_ASSERT( action >= 0 && action < LastAction );
  d->actions[ action ] = false;
}



//static
Behaviour Behaviour::behaviourForSending()
{
  static Behaviour beh;
  // A default-constructed Behaviour has default sending behaviour.
  return beh;
}

//static
Behaviour Behaviour::behaviourForPrinting()
{
  static bool init = false;
  static Behaviour beh;
  if( !init ) {
    beh.d->actions[ UseGui ] = true;
    beh.d->actions[ UseCrypto ] = false;
    beh.d->actions[ UseWrapping ] = true;
    beh.d->actions[ UseFallbackCharset ] = false;
    beh.d->actions[ WarnBadCharset ] = true;
    beh.d->actions[ WarnZeroRecipients ] = false;
    beh.d->actions[ CustomHeaders ] = false;
    init = true;
  }
  return beh;
}

//static
Behaviour Behaviour::behaviourForAutosaving()
{
  static bool init = false;
  static Behaviour beh;
  if( !init ) {
    beh.d->actions[ UseGui ] = false;
    beh.d->actions[ UseCrypto ] = false;
    beh.d->actions[ UseWrapping ] = false;
    beh.d->actions[ UseFallbackCharset ] = true;
    beh.d->actions[ WarnBadCharset ] = false;
    beh.d->actions[ WarnZeroRecipients ] = false;
    beh.d->actions[ CustomHeaders ] = true;
    init = true;
  }
  return beh;
}

//static
Behaviour Behaviour::behaviourForSavingLocally()
{
  static bool init = false;
  static Behaviour beh;
  if( !init ) {
    beh.d->actions[ UseGui ] = true;
    beh.d->actions[ UseCrypto ] = false;
    beh.d->actions[ UseWrapping ] = false;
    beh.d->actions[ UseFallbackCharset ] = false;
    beh.d->actions[ WarnBadCharset ] = true;
    beh.d->actions[ WarnZeroRecipients ] = false;
    beh.d->actions[ CustomHeaders ] = true;
    init = true;
  }
  return beh;
}

