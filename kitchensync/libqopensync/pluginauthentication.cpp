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

#include "pluginauthentication.h"

using namespace QSync;

PluginAuthentication::PluginAuthentication()
  : mPluginAuthentication( 0 )
{
}

PluginAuthentication::~PluginAuthentication()
{
}

bool PluginAuthentication::isValid() const
{
  return (mPluginAuthentication != 0);
}

void PluginAuthentication::setUserName( const QString &userName )
{
  osync_plugin_authentication_set_username( mPluginAuthentication, userName.toUtf8().data() );
}

QString PluginAuthentication::userName() const
{
  return QString::fromUtf8( osync_plugin_authentication_get_username( mPluginAuthentication ) );
}

void PluginAuthentication::setPassword( const QString &password )
{
  osync_plugin_authentication_set_password( mPluginAuthentication, password.toUtf8().data() );
}

QString PluginAuthentication::password() const
{
  return QString::fromUtf8( osync_plugin_authentication_get_password( mPluginAuthentication ) );
}

void PluginAuthentication::setReference( const QString &reference )
{
  osync_plugin_authentication_set_reference( mPluginAuthentication, reference.toUtf8().data() );
}

QString PluginAuthentication::reference() const
{
  return QString::fromUtf8( osync_plugin_authentication_get_reference( mPluginAuthentication ) );
}
