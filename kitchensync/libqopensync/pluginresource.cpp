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

void PluginResource::setEnabled( bool enabled )
{
  osync_plugin_resource_enable( mPluginResource, enabled ? TRUE : FALSE );
}

bool PluginResource::enabled() const
{
  return (osync_plugin_resource_is_enabled( mPluginResource ) == TRUE);
}

void PluginResource::setName( const QString &name )
{
  osync_plugin_resource_set_name( mPluginResource, name.toUtf8().data() );
}

QString PluginResource::name() const
{
  return QString::fromUtf8( osync_plugin_resource_get_name( mPluginResource ) );
}

void PluginResource::setMimeType( const QString &mimeType )
{
  osync_plugin_resource_set_mime( mPluginResource, mimeType.toUtf8().data() );
}

QString PluginResource::mimeType() const
{
  return QString::fromUtf8( osync_plugin_resource_get_mime( mPluginResource ) );
}

ObjectFormatSink::List PluginResource::objectFormatSinks() const
{
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
  osync_plugin_resource_add_objformat_sink( mPluginResource, sink.mObjectFormatSink );
}

void PluginResource::removeObjectFormatSink( const ObjectFormatSink &sink )
{
  osync_plugin_resource_remove_objformat_sink( mPluginResource, sink.mObjectFormatSink );
}

void PluginResource::setObjectType( const QString &objectType )
{
  osync_plugin_resource_set_objtype( mPluginResource, objectType.toUtf8().data() );
}

QString PluginResource::objectType() const
{
  return QString::fromUtf8( osync_plugin_resource_get_objtype( mPluginResource ) );
}

void PluginResource::setPath( const QString &path )
{
  osync_plugin_resource_set_path( mPluginResource, path.toUtf8().data() );
}

QString PluginResource::path() const
{
  return QString::fromUtf8( osync_plugin_resource_get_path( mPluginResource ) );
}

void PluginResource::setUrl( const QString &url )
{
  osync_plugin_resource_set_url( mPluginResource, url.toUtf8().data() );
}

QString PluginResource::url() const
{
  return QString::fromUtf8( osync_plugin_resource_get_url( mPluginResource ) );
}
