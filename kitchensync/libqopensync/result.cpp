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

#include "result.h"

#include <opensync/opensync.h>

using namespace QSync;

Result::Result()
  : mType( NoError )
{
}

Result::Result( Type type )
  : mType( type )
{
}

Result::Result( OSyncError **error, bool deleteError )
{
  OSyncErrorType otype = osync_error_get_type( error );
  Type type;

  switch ( otype ) {
    case OSYNC_NO_ERROR:
      type = NoError;
      break;
    default:
    case OSYNC_ERROR_GENERIC:
      type = GenericError;
      break;
    case OSYNC_ERROR_IO_ERROR:
      type = IOError;
      break;
    case OSYNC_ERROR_NOT_SUPPORTED:
      type = NotSupported;
      break;
    case OSYNC_ERROR_TIMEOUT:
      type = Timeout;
      break;
    case OSYNC_ERROR_DISCONNECTED:
      type = Disconnected;
      break;
    case OSYNC_ERROR_FILE_NOT_FOUND:
      type = FileNotFound;
      break;
    case OSYNC_ERROR_EXISTS:
      type = Exists;
      break;
    case OSYNC_ERROR_CONVERT:
      type = Convert;
      break;
    case OSYNC_ERROR_MISCONFIGURATION:
      type = Misconfiguration;
      break;
    case OSYNC_ERROR_INITIALIZATION:
      type = Initialization;
      break;
    case OSYNC_ERROR_PARAMETER:
      type = Parameter;
      break;
    case OSYNC_ERROR_EXPECTED:
      type = Expected;
      break;
    case OSYNC_ERROR_NO_CONNECTION:
      type = NoConnection;
      break;
    case OSYNC_ERROR_TEMPORARY:
      type = Temporary;
      break;
    case OSYNC_ERROR_LOCKED:
      type = Locked;
      break;
    case OSYNC_ERROR_PLUGIN_NOT_FOUND:
      type = PluginNotFound;
      break;
  }

  mType = type;
  mName = QString::fromUtf8( osync_error_get_name( error ) );
  mMessage = QString::fromUtf8( osync_error_print( error ) );

  if ( deleteError )
    osync_error_free( error );
}

Result::~Result()
{
}

void Result::setName( const QString &name )
{
  mName = name;
}

QString Result::name() const
{
  return mName;
}

void Result::setMessage( const QString &message )
{
  mMessage = message;
}

QString Result::message() const
{
  return mMessage;
}

void Result::setType( Type type )
{
  mType = type;
}

Result::Type Result::type() const
{
  return mType;
}

bool Result::isError() const
{
  return mType != NoError;
}

Result::operator bool () const
{
  return ( mType != NoError );
}

