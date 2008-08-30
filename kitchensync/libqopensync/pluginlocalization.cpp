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

#include "pluginlocalization.h"

using namespace QSync;

PluginLocalization::PluginLocalization()
  : mPluginLocalization( 0 )
{
}

PluginLocalization::~PluginLocalization()
{
}

bool PluginLocalization::isValid() const
{
  return (mPluginLocalization != 0);
}

void PluginLocalization::setEncoding( const QString &encoding )
{
  Q_ASSERT( mPluginLocalization );

  osync_plugin_localization_set_encoding( mPluginLocalization, encoding.toLatin1().data() );
}

QString PluginLocalization::encoding() const
{
  Q_ASSERT( mPluginLocalization );

  return QString::fromLatin1( osync_plugin_localization_get_encoding( mPluginLocalization ) );
}

void PluginLocalization::setTimeZone( const QString &timezone )
{
  Q_ASSERT( mPluginLocalization );

  osync_plugin_localization_set_timezone( mPluginLocalization, timezone.toLatin1() );
}

QString PluginLocalization::timeZone() const
{
  Q_ASSERT( mPluginLocalization );

  return QString::fromLatin1( osync_plugin_localization_get_timezone( mPluginLocalization ) );
}

void PluginLocalization::setLanguage( const QString &language )
{
  Q_ASSERT( mPluginLocalization );

  osync_plugin_localization_set_language( mPluginLocalization, language.toLatin1().data() );
}

QString PluginLocalization::language() const
{
  Q_ASSERT( mPluginLocalization );

  return QString::fromLatin1( osync_plugin_localization_get_language( mPluginLocalization ) );
}

