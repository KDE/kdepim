/*
    This file is part of libqopensync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2007 Daniel Gollub <dgollub@suse.de>

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
#include "result.h"

#include "pluginenv.h"

using namespace QSync;

PluginEnv::PluginEnv()
{
  OSyncError *error = 0;
  mPluginEnv = osync_plugin_env_new( &error );
}

PluginEnv::~PluginEnv()
{
  osync_plugin_env_free( mPluginEnv );
}

Result PluginEnv::initialize()
{
  OSyncError *error = 0;
  if ( !osync_plugin_env_load( mPluginEnv, NULL, &error ) )
    return Result( &error );
  else
    return Result();
}

Result PluginEnv::finalize()
{
  osync_plugin_env_free( mPluginEnv ); 
  return Result();
}

int PluginEnv::pluginCount() const
{
  return osync_plugin_env_num_plugins( mPluginEnv );
}

Plugin PluginEnv::pluginAt( int pos ) const
{
  Plugin plugin;

  if ( pos < 0 || pos >= pluginCount() )
    return plugin;

  OSyncPlugin *oplugin = osync_plugin_env_nth_plugin( mPluginEnv, pos );
  plugin.mPlugin = oplugin;

  return plugin;
}

Plugin PluginEnv::pluginByName( const TQString &name ) const
{
  Plugin plugin;

  OSyncPlugin *oplugin = osync_plugin_env_find_plugin( mPluginEnv, name.latin1() );
  if ( oplugin )
    plugin.mPlugin = oplugin;

  return plugin;
}

/*
Conversion PluginEnv::conversion() const
{
  Conversion conversion;
  conversion.mPluginEnv = mPluginEnv;

  return conversion;
}
*/
