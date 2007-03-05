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
#include <stdlib.h>

#include "member.h"

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

  if ( !mMember )
    return false;

  if ( !osync_member_instance_plugin( mMember, pluginName().utf8(), &error ) ) {
    qDebug( "Plugin %s is not valid: %s", pluginName().latin1(), osync_error_print( &error ) );
    osync_error_free( &error );
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

Plugin Member::plugin() const
{
  Q_ASSERT( mMember );

  Plugin plugin;

  OSyncPlugin *oplugin = osync_member_get_plugin( mMember );
  if ( oplugin )
    plugin.mPlugin = oplugin;

  return plugin;
}

int Member::id() const
{
  Q_ASSERT( mMember );

  return osync_member_get_id( mMember );
}

void Member::setName( const QString &name )
{
  Q_ASSERT( mMember );

  osync_member_set_name( mMember, (const char*)name.utf8() );
}

QString Member::name() const
{
  Q_ASSERT( mMember );

  return QString::fromUtf8( osync_member_get_name( mMember ) );
}

void Member::setConfiguration( const QByteArray &configurationData )
{
  Q_ASSERT( mMember );

  osync_member_set_config( mMember, configurationData.data(), configurationData.size() );
}

Result Member::configuration( QByteArray &configurationData, bool useDefault )
{
  Q_ASSERT( mMember );

  char *data;
  int size;

  OSyncError *error = 0;
  osync_bool ok = false;
  if ( useDefault )
    ok = osync_member_get_config_or_default( mMember, &data, &size, &error );
  else
    ok = osync_member_get_config( mMember, &data, &size, &error );

  if ( !ok ) {
    return Result( &error );
  } else {
    configurationData.resize( size );
    memcpy( configurationData.data(), data, size );

    return Result();
  }
}

Result Member::save()
{
  Q_ASSERT( mMember );

  OSyncError *error = 0;
  if ( !osync_member_save( mMember, &error ) )
    return Result( &error );
  else
    return Result();
}

Result Member::instance( const Plugin &plugin )
{
  OSyncError *error = 0;
  if ( !osync_member_instance_plugin( mMember, plugin.name().utf8(), &error ) )
    return Result( &error );
  else
    return Result();
}

bool Member::operator==( const Member &member ) const
{
  return mMember == member.mMember;
}

QString Member::scanDevices( const QString &query )
{
  Q_ASSERT( mMember );

  OSyncError *error = 0;
  char *data = (char*)osync_member_call_plugin( mMember, "scan_devices", const_cast<char*>( query.utf8().data() ), &error );
  if ( error != 0 ) {
    osync_error_free( &error );
    return QString();
  } else {
    QString xml = QString::fromUtf8( data );
    free( data );
    return xml;
  }
}

bool Member::testConnection( const QString &configuration )
{
  Q_ASSERT( mMember );

  OSyncError *error = 0;
  int *result = (int*)osync_member_call_plugin( mMember, "test_connection", const_cast<char*>( configuration.utf8().data() ), &error );
  if ( error != 0 ) {
    osync_error_free( &error );
    return false;
  } else {
    bool value = ( *result == 1 ? true : false );
    free( result );
    return value;
  }
}
