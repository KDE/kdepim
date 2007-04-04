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

QStringList Conversion::objectTypes() const
{
  Q_ASSERT( mEnvironment );

  OSyncFormatEnv *formatEnv = osync_conv_env_new( mEnvironment );
  Q_ASSERT( formatEnv );

  QStringList types;	
  for ( int i = 0; i < osync_conv_num_objtypes( formatEnv ); i++ ) {
    OSyncObjType *type = osync_conv_nth_objtype( formatEnv, i );
    types.append( QString::fromUtf8( osync_objtype_get_name( type ) ) );
  }

  osync_conv_env_free( formatEnv );

  return types;
}
