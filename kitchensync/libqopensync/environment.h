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

#ifndef OSYNC_ENVIRONMENT_H
#define OSYNC_ENVIRONMENT_H

#include <qstring.h>

#include <libqopensync/group.h>
#include <libqopensync/plugin.h>
#include <libqopensync/result.h>
#include <libqopensync/conversion.h>

struct OSyncEnv;

namespace QSync {

class Environment
{
  public:
    Environment();
    ~Environment();

    class GroupIterator
    {
      friend class Environment;

      public:
        GroupIterator( Environment *environment )
          : mEnvironment( environment ), mPos( -1 )
        {
        }

        GroupIterator( const GroupIterator &it )
        {
          mEnvironment = it.mEnvironment;
          mPos = it.mPos;
        }

        Group operator*() 
        {
          return mEnvironment->groupAt( mPos );
        }

        GroupIterator &operator++() { mPos++; return *this; }
        GroupIterator &operator++( int ) { mPos++; return *this; }
        GroupIterator &operator--() { mPos--; return *this; }
        GroupIterator &operator--( int ) { mPos--; return *this; }
        bool operator==( const GroupIterator &it ) { return mEnvironment == it.mEnvironment && mPos == it.mPos; }
        bool operator!=( const GroupIterator &it ) { return mEnvironment == it.mEnvironment && mPos != it.mPos; }

      private:
        Environment *mEnvironment;
        int mPos;
    };

    class PluginIterator
    {
      friend class Environment;

      public:
        PluginIterator( Environment *environment )
          : mEnvironment( environment ), mPos( -1 )
        {
        }

        PluginIterator( const PluginIterator &it )
        {
          mEnvironment = it.mEnvironment;
          mPos = it.mPos;
        }

        Plugin operator*() 
        {
          return mEnvironment->pluginAt( mPos );
        }

        PluginIterator &operator++() { mPos++; return *this; }
        PluginIterator &operator++( int ) { mPos++; return *this; }
        PluginIterator &operator--() { mPos--; return *this; }
        PluginIterator &operator--( int ) { mPos--; return *this; }
        bool operator==( const PluginIterator &it ) { return mEnvironment == it.mEnvironment && mPos == it.mPos; }
        bool operator!=( const PluginIterator &it ) { return mEnvironment == it.mEnvironment && mPos != it.mPos; }

      private:
        Environment *mEnvironment;
        int mPos;
    };

    /**
      Returns an iterator pointing to the first item in the group list.
      This iterator equals groupEnd() if the group list is empty.
     */
    GroupIterator groupBegin();

    /**
      Returns an iterator pointing past the last item in the group list.
      This iterator equals groupBegin() if the group list is empty.
     */
    GroupIterator groupEnd();

    /**
      Returns an iterator pointing to the first item in the plugin list.
      This iterator equals pluginEnd() if the group list is empty.
     */
    PluginIterator pluginBegin();

    /**
      Returns an iterator pointing past the last item in the plugin list.
      This iterator equals pluginBegin() if the plugin list is empty.
     */
    PluginIterator pluginEnd();

    /**
      Initializes the environment ( e.g. loads the groups and plugins ).
      Has to be called before the groups or plugins can be accessed.
     */
    Result initialize();

    /**
      Finalizes the environment ( e.g. unloads the groups and plugins ).
      Should be the last call before the object is deleted.
     */
    Result finalize();

    /**
      Returns the number of groups.
     */
    int groupCount() const;

    /**
      Returns the group at position @param pos.
     */
    Group groupAt( int pos ) const;

    /**
      Returns a group by name or an invalid group when the group with this
      name doesn't exists.
     */
    Group groupByName( const QString &name ) const;

    /**
      Adds a new group to the environment.

      @returns the new group.
     */
    Group addGroup();

    /**
      Removes a group from the environment.
     */
    Result removeGroup( const Group &group );

    /**
      Returns the number of plugins.
     */
    int pluginCount() const;

    /**
      Returns the plugin at position @param pos.
     */
    Plugin pluginAt( int pos ) const;

    /**
      Returns a plugin by name or an invalid plugin when the plugin with this
      name doesn't exists.
     */
    Plugin pluginByName( const QString &name ) const;

    /**
      Returns the conversion object of this environment.
     */
    Conversion conversion() const;

  private:
    OSyncEnv *mEnvironment;
};

}

#endif
