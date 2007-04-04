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

#include "environment.h"

#include <opensync/opensync.h>

using namespace QSync;

Environment::Environment()
{
  mEnvironment = osync_env_new();
}

Environment::~Environment()
{
  osync_env_free( mEnvironment );
}

Environment::GroupIterator Environment::groupBegin()
{
  GroupIterator it( this );
  it.mPos = 0;

  return it;
}

Environment::GroupIterator Environment::groupEnd()
{
  GroupIterator it( this );
  it.mPos = groupCount();

  return it;
}

Environment::PluginIterator Environment::pluginBegin()
{
  PluginIterator it( this );
  it.mPos = 0;

  return it;
}

Environment::PluginIterator Environment::pluginEnd()
{
  PluginIterator it( this );
  it.mPos = pluginCount();

  return it;
}

Result Environment::initialize()
{
  OSyncError *error = 0;
  if ( !osync_env_initialize( mEnvironment, &error ) )
    return Result( &error );
  else
    return Result();
}

Result Environment::finalize()
{
  OSyncError *error = 0;
  if ( !osync_env_finalize( mEnvironment, &error ) )
    return Result( &error);
  else
    return Result();
}

int Environment::groupCount() const
{
  return osync_env_num_groups( mEnvironment );
}

Group Environment::groupAt( int pos ) const
{
  Group group;

  if ( pos < 0 || pos >= groupCount() )
    return group;

  OSyncGroup *ogroup = osync_env_nth_group( mEnvironment, pos );
  group.mGroup = ogroup;

  return group;
}

Group Environment::groupByName( const QString &name ) const
{
  Group group;

  OSyncGroup *ogroup = osync_env_find_group( mEnvironment, name.latin1() );
  if ( ogroup )
    group.mGroup = ogroup;

  return group;
}

Group Environment::addGroup()
{
  Group group;

  OSyncGroup *ogroup = osync_group_new( mEnvironment );
  if ( ogroup )
    group.mGroup = ogroup;

  return group;
}

Result Environment::removeGroup( const Group &group )
{
  OSyncError *error = 0;
  if ( !osync_group_delete( group.mGroup, &error ) )
    return Result( &error );
  else
    return Result();
}

int Environment::pluginCount() const
{
  return osync_env_num_plugins( mEnvironment );
}

Plugin Environment::pluginAt( int pos ) const
{
  Plugin plugin;

  if ( pos < 0 || pos >= pluginCount() )
    return plugin;

  OSyncPlugin *oplugin = osync_env_nth_plugin( mEnvironment, pos );
  plugin.mPlugin = oplugin;

  return plugin;
}

Plugin Environment::pluginByName( const QString &name ) const
{
  Plugin plugin;

  OSyncPlugin *oplugin = osync_env_find_plugin( mEnvironment, name.latin1() );
  if ( oplugin )
    plugin.mPlugin = oplugin;

  return plugin;
}

Conversion Environment::conversion() const
{
  Conversion conversion;
  conversion.mEnvironment = mEnvironment;

  return conversion;
}
