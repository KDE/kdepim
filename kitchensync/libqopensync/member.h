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

#ifndef QSYNC_MEMBER_H
#define QSYNC_MEMBER_H

#include <libqopensync/qopensync_export.h>
#include <libqopensync/plugin.h>
#include <libqopensync/pluginconfig.h>
#include <libqopensync/result.h>

class OSyncMember;

namespace QSync {

class QSYNC_EXPORT Member
{
  friend class Group;
  friend class Engine;
  friend class SyncChange;
  friend class SyncMemberUpdate;
  friend class SyncChangeUpdate;

  public:
    Member();
    ~Member();

    /**
      Returns whether the member object is valid.
     */
    bool isValid() const;

    /**
      Returns the configuration directory.
     */
    QString configurationDirectory() const;

    /**
      Returns the name of the plugin, the member belongs to.
     */
    QString pluginName() const;

    /**
      Returns the id of the plugin.
    */
    int id() const;

    /**
      Sets the name of this member.
     */
    void setName( const QString &name );

    /**
      Returns the name of this member.
     */
    QString name() const;

    /**
      Returns whether the member has a configuration available.
     */
    bool hasConfiguration() const;

    /**
      Sets the configuration object of the member.
     */
    void setConfiguration( const PluginConfig &config );

    /**
      Returns the configuration object of the member.
     */
    PluginConfig configuration() const;

    /**
      Returns the configuration object of the member or the default if
      no configuration is set.
     */
    PluginConfig configurationOrDefault() const;

    /**
      Saves the changes to the configuration to hard disc.
     */
    Result save() const;

    /**
      Make this member an instance of the given plugin.
    */
    Result instance() const;

    bool operator==( const Member &member ) const;

    /**
     * Deletes the member's information from the hard disc.
     */
    Result cleanup() const;

  private:
    OSyncMember *mMember;
};

}

#endif
