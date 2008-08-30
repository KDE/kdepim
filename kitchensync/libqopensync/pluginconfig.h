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

#ifndef QSYNC_PLUGINCONFIG_H
#define QSYNC_PLUGINCONFIG_H

#include <libqopensync/qopensync_export.h>
#include <libqopensync/pluginadvancedoption.h>
#include <libqopensync/pluginauthentication.h>
#include <libqopensync/pluginconnection.h>
#include <libqopensync/pluginlocalization.h>
#include <libqopensync/pluginresource.h>

#include <QtCore/QString>

class OSyncPluginConfig;

namespace QSync {

class QSYNC_EXPORT PluginConfig
{
    friend class Member;

  public:
    PluginConfig();
    ~PluginConfig();

    /**
      Returns whether the object is a valid plugin config.
     */
    bool isValid() const;

    /**
      Returns the list of advanced options.
     */
    PluginAdvancedOption::List advancedOptions() const;

    /**
      Returns the advanced option for the given name.
     */
    PluginAdvancedOption advancedOption( const QString &name ) const;

    /**
      Adds an advanced option to the config.
     */
    void addAdvancedOption( const PluginAdvancedOption &option );

    /**
      Removes an advanced option from the config.
     */
    void removeAdvancedOption( const PluginAdvancedOption &option );

    /**
      Sets the authentication of the config.
     */
    void setAuthentication( const PluginAuthentication &authentication );

    /**
      Returns the authentication of the config.
     */
    PluginAuthentication authentication() const;

    /**
      Sets the localization of the config.
     */
    void setLocalization( const PluginLocalization &localization );

    /**
      Returns the localization of the config.
     */
    PluginLocalization localization() const;

    /**
      Returns the list of resources of the config.
     */
    PluginResource::List resources() const;

    /**
      Returns the active resource for the given object type.
     */
    PluginResource resource( const QString &objectType ) const;

    /**
      Adds a resource to the config.
     */
    void addResource( const PluginResource &resource );

    /**
      Removes a resource from the config.
     */
    void removeResource( const PluginResource &resource );

    /**
      Sets the connection of the config.
     */
    void setConnection( const PluginConnection &connection );

    /**
      Returns the connection of the config.
     */
    PluginConnection connection() const;

  private:
    OSyncPluginConfig *mPluginConfig;
};

}

#endif
