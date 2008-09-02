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
#include <opensync/opensync-format.h>
#include <opensync/opensync-plugin.h>

#include "pluginresource.h"

using namespace QSync;

PluginResource::PluginResource()
  : mPluginResource( 0 )
{
}

PluginResource::~PluginResource()
{
}

bool PluginResource::isValid() const
{
  return (mPluginResource != 0);
}

bool PluginResource::isOptionSupported( ConfigOption option ) const
{
  Q_ASSERT( mPluginResource );

  OSyncPluginResourceOptionSupportedFlag flag = OSYNC_PLUGIN_RESOURCE_NAME;

  switch ( option ) {
    case NameOption: flag = OSYNC_PLUGIN_RESOURCE_NAME; break;
    case PathOption: flag = OSYNC_PLUGIN_RESOURCE_PATH; break;
    case UrlOption: flag = OSYNC_PLUGIN_RESOURCE_URL; break;
  }

  return (osync_plugin_resource_option_is_supported( mPluginResource, flag ) == TRUE);
}

void PluginResource::setEnabled( bool enabled )
{
  Q_ASSERT( mPluginResource );

  osync_plugin_resource_enable( mPluginResource, enabled ? TRUE : FALSE );
}

bool PluginResource::enabled() const
{
  Q_ASSERT( mPluginResource );

  return (osync_plugin_resource_is_enabled( mPluginResource ) == TRUE);
}

void PluginResource::setName( const QString &name )
{
  Q_ASSERT( mPluginResource );

  osync_plugin_resource_set_name( mPluginResource, name.toUtf8().data() );
}

QString PluginResource::name() const
{
  Q_ASSERT( mPluginResource );

  return QString::fromUtf8( osync_plugin_resource_get_name( mPluginResource ) );
}

void PluginResource::setMimeType( const QString &mimeType )
{
  Q_ASSERT( mPluginResource );

  osync_plugin_resource_set_mime( mPluginResource, mimeType.toUtf8().data() );
}

QString PluginResource::mimeType() const
{
  Q_ASSERT( mPluginResource );

  return QString::fromUtf8( osync_plugin_resource_get_mime( mPluginResource ) );
}

ObjectFormatSink::List PluginResource::objectFormatSinks() const
{
  Q_ASSERT( mPluginResource );

  ObjectFormatSink::List sinks;

  OSyncList *list = osync_plugin_resource_get_objformat_sinks( mPluginResource );
  for ( ; list; list = list->next ) {
    ObjectFormatSink sink;
    sink.mObjectFormatSink = (OSyncObjFormatSink*)list->data;
    sinks.append( sink );
  }

  return sinks;
}

void PluginResource::addObjectFormatSink( const ObjectFormatSink &sink )
{
  Q_ASSERT( mPluginResource );

  osync_plugin_resource_add_objformat_sink( mPluginResource, sink.mObjectFormatSink );
}

void PluginResource::removeObjectFormatSink( const ObjectFormatSink &sink )
{
  Q_ASSERT( mPluginResource );

  osync_plugin_resource_remove_objformat_sink( mPluginResource, sink.mObjectFormatSink );
}

void PluginResource::setObjectType( const QString &objectType )
{
  Q_ASSERT( mPluginResource );

  osync_plugin_resource_set_objtype( mPluginResource, objectType.toUtf8().data() );
}

QString PluginResource::objectType() const
{
  Q_ASSERT( mPluginResource );

  return QString::fromUtf8( osync_plugin_resource_get_objtype( mPluginResource ) );
}

void PluginResource::setPath( const QString &path )
{
  Q_ASSERT( mPluginResource );

  osync_plugin_resource_set_path( mPluginResource, path.toUtf8().data() );
}

QString PluginResource::path() const
{
  Q_ASSERT( mPluginResource );

  return QString::fromUtf8( osync_plugin_resource_get_path( mPluginResource ) );
}

void PluginResource::setUrl( const QString &url )
{
  Q_ASSERT( mPluginResource );

  osync_plugin_resource_set_url( mPluginResource, url.toUtf8().data() );
}

QString PluginResource::url() const
{
  Q_ASSERT( mPluginResource );

  return QString::fromUtf8( osync_plugin_resource_get_url( mPluginResource ) );
}
