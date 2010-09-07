/*
    This file is part of libqopensync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#include <opensync/opensync.h>
#include <opensync/opensync-plugin.h>

#include "plugin.h"

using namespace QSync;

Plugin::Plugin()
  : mPlugin( 0 )
{
}

Plugin::~Plugin()
{
}

bool Plugin::isValid() const
{
  return ( mPlugin != 0 );
}

TQString Plugin::name() const
{
  Q_ASSERT( mPlugin );

  return TQString::fromLatin1( osync_plugin_get_name( mPlugin ) );
}

TQString Plugin::longName() const
{
  Q_ASSERT( mPlugin );

  return TQString::fromLatin1( osync_plugin_get_longname( mPlugin ) );
}

TQString Plugin::description() const
{
  Q_ASSERT( mPlugin );

  return TQString::fromLatin1( osync_plugin_get_description( mPlugin ) );
}

