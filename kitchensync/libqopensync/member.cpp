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
#include <opensync/opensync-group.h>

#include <stdlib.h>

#include "member.h"
#include "result.h"

using namespace QSync;

Member::Member()
  : mMember( 0 )
{
}

Member::~Member()
{
}

bool Member::isValid() const
{
  OSyncError *error = 0;

  if ( !mMember ) {
    return false;
  }

  if ( !osync_member_load( mMember, configurationDirectory().toUtf8(), &error ) ) {
    qDebug( "Plugin %s is not valid: %s", qPrintable( pluginName() ), osync_error_print( &error ) );
    osync_error_unref( &error );
    return false;
  }

  return true;
}

QString Member::configurationDirectory() const
{
  Q_ASSERT( mMember );

  return QString::fromLatin1( osync_member_get_configdir( mMember ) );
}

QString Member::pluginName() const
{
  Q_ASSERT( mMember );

  return QString::fromLatin1( osync_member_get_pluginname( mMember ) );
}

int Member::id() const
{
  Q_ASSERT( mMember );

  return osync_member_get_id( mMember );
}

void Member::setName( const QString &name )
{
  Q_ASSERT( mMember );

  osync_member_set_name( mMember, (const char*)name.toUtf8() );
}

QString Member::name() const
{
  Q_ASSERT( mMember );

  return QString::fromUtf8( osync_member_get_name( mMember ) );
}

void Member::setConfiguration( const QByteArray &configurationData )
{
  Q_ASSERT( mMember );

//  FIXME: tokoe
//  osync_member_set_config( mMember, configurationData.data() );
}

Result Member::configuration( QByteArray &configurationData, bool useDefault ) const
{
  Q_ASSERT( mMember );

  const char *data = "  ";
  int size;

  OSyncError *error = 0;
  if ( useDefault ) {
//  FIXME: tokoe
//    data = osync_member_get_config_or_default( mMember, &error );
  } else {
//  FIXME: tokoe
//    data = osync_member_get_config( mMember, &error );
  }

  if ( !data ) {
    return Result( &error );
  } else {
    size = strlen( data );
    configurationData.resize( size );
    memcpy( configurationData.data(), data, size );

    return Result();
  }
}

Result Member::save() const
{
  Q_ASSERT( mMember );

  OSyncError *error = 0;
  if ( !osync_member_save( mMember, &error ) ) {
    return Result( &error );
  } else {
    return Result();
  }
}

Result Member::instance() const
{
  OSyncError *error = 0;
  if ( !osync_member_load( mMember, configurationDirectory().toUtf8(), &error ) ) {
    return Result( &error );
  } else {
    return Result();
  }
}

bool Member::operator==( const Member &member ) const
{
  return mMember == member.mMember;
}

Result Member::cleanup() const
{
  Q_ASSERT( mMember );

  OSyncError *error = 0;
  if ( !osync_member_delete( mMember, &error ) )
    return Result( &error );
  else
    return Result();
}
