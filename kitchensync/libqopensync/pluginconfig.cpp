/*
    This file is part of libqopensync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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

#include "pluginconfig.h"

using namespace QSync;

PluginConfig::PluginConfig()
  : mPluginConfig( 0 )
{
}

PluginConfig::~PluginConfig()
{
}

bool PluginConfig::isValid() const
{
  return (mPluginConfig != 0);
}

bool PluginConfig::isOptionSupported( ConfigOptions options ) const
{
  Q_ASSERT( mPluginConfig );

  OSyncPluginConfigSupportedFlag flag;
  switch ( options ) {
    case AuthenticationOption: flag = OPENSYNC_PLUGIN_CONFIG_AUTHENTICATION; break;
    case LocalizationOption: flag = OPENSYNC_PLUGIN_CONFIG_LOCALIZATION; break;
    case ConnectionOption: flag = OPENSYNC_PLUGIN_CONFIG_CONNECTION; break;
    case AdvancedOption: flag = OPENSYNC_PLUGIN_CONFIG_ADVANCEDOPTION; break;
    case ResourceOption:
    default: flag = OPENSYNC_PLUGIN_CONFIG_RESOURCES; break;
  }

  return (osync_plugin_config_is_supported( mPluginConfig, flag ) == TRUE);
}

PluginAdvancedOption::List PluginConfig::advancedOptions() const
{
  Q_ASSERT( mPluginConfig );

  PluginAdvancedOption::List options;

  OSyncList *list = osync_plugin_config_get_advancedoptions( mPluginConfig );
  for ( ; list; list = list->next ) {
    PluginAdvancedOption option;
    option.mPluginAdvancedOption = (OSyncPluginAdvancedOption*)list->data;

    options.append( option );
  }

  return options;
}

PluginAdvancedOption PluginConfig::advancedOption( const QString &name ) const
{
  Q_ASSERT( mPluginConfig );

  PluginAdvancedOption option;
  option.mPluginAdvancedOption = osync_plugin_config_get_advancedoption_value_by_name( mPluginConfig,
                                                                                         name.toUtf8().data() );

  return option;
}

void PluginConfig::addAdvancedOption( const PluginAdvancedOption &option )
{
  Q_ASSERT( mPluginConfig );

  osync_plugin_config_add_advancedoption( mPluginConfig, option.mPluginAdvancedOption );
}

void PluginConfig::removeAdvancedOption( const PluginAdvancedOption &option )
{
  Q_ASSERT( mPluginConfig );

  osync_plugin_config_remove_advancedoption( mPluginConfig, option.mPluginAdvancedOption );
}

void PluginConfig::setAuthentication( const PluginAuthentication &authentication )
{
  Q_ASSERT( mPluginConfig );

  osync_plugin_config_set_authentication( mPluginConfig, authentication.mPluginAuthentication );
}

PluginAuthentication PluginConfig::authentication() const
{
  Q_ASSERT( mPluginConfig );

  PluginAuthentication authentication;
  authentication.mPluginAuthentication = osync_plugin_config_get_authentication( mPluginConfig );

  return authentication;
}

void PluginConfig::setLocalization( const PluginLocalization &localization )
{
  Q_ASSERT( mPluginConfig );

  osync_plugin_config_set_localization( mPluginConfig, localization.mPluginLocalization );
}

PluginLocalization PluginConfig::localization() const
{
  Q_ASSERT( mPluginConfig );

  PluginLocalization localization;
  localization.mPluginLocalization = osync_plugin_config_get_localization( mPluginConfig );

  return localization;
}

PluginResource::List PluginConfig::resources() const
{
  Q_ASSERT( mPluginConfig );

  PluginResource::List resources;

  OSyncList *list = osync_plugin_config_get_resources( mPluginConfig );
  for ( ; list; list = list->next ) {
    PluginResource resource;
    resource.mPluginResource = (OSyncPluginResource*)list->data;

    resources.append( resource );
  }

  return resources;
}

PluginResource PluginConfig::resource( const QString &objectType ) const
{
  Q_ASSERT( mPluginConfig );

  PluginResource resource;
  resource.mPluginResource = osync_plugin_config_find_active_resource( mPluginConfig, objectType.toUtf8().data() );

  return resource;
}

void PluginConfig::addResource( const PluginResource &resource )
{
  Q_ASSERT( mPluginConfig );

  osync_plugin_config_add_resource( mPluginConfig, resource.mPluginResource );
}

void PluginConfig::removeResource( const PluginResource &resource )
{
  Q_ASSERT( mPluginConfig );

  osync_plugin_config_remove_resource( mPluginConfig, resource.mPluginResource );
}

void PluginConfig::setConnection( const PluginConnection &connection )
{
  Q_ASSERT( mPluginConfig );

  osync_plugin_config_set_connection( mPluginConfig, connection.mPluginConnection );
}

PluginConnection PluginConfig::connection() const
{
  Q_ASSERT( mPluginConfig );

  PluginConnection connection;
  connection.mPluginConnection = osync_plugin_config_get_connection( mPluginConfig );

  return connection;
}
