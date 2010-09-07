/*
    This file is part of libqopensync.

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

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
#include <opensync/opensync-format.h>

#include "conversion.h"

using namespace QSync;

Conversion::Conversion()
  : mEnvironment( 0 )
{
}

Conversion::~Conversion()
{
}

bool Conversion::isValid() const
{
  return mEnvironment != 0;
}

TQStringList Conversion::objectTypes() const
{
  Q_ASSERT( mEnvironment );

  OSyncError *error = NULL;
  OSyncFormatEnv *formatEnv = osync_format_env_new( &error );
  Q_ASSERT( formatEnv );

  osync_format_env_load_plugins(formatEnv, NULL, &error);

  TQStringList types;	

  for (int i = 0; i < osync_format_env_num_objformats(formatEnv); i++) {
    OSyncObjFormat *format = osync_format_env_nth_objformat(formatEnv, i);
    types.append( TQString::fromUtf8( osync_objformat_get_objtype(format) ) );
 }

  osync_format_env_free( formatEnv );

  return types;
}
