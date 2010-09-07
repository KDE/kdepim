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

#ifndef OSYNC_GROUPENV_H
#define OSYNC_GROUPENV_H

#include <tqstring.h>

struct OSyncGroupEnv;

namespace QSync {

class Group;
class Result;

class GroupEnv
{
  public:
    GroupEnv();
    ~GroupEnv();

    /**
      Initializes the environment ( e.g. loads the groups and plugins ).
      Has to be called before the groups or plugins can be accessed.
     */
    Result initialize();

    /**
      Finalizes the environment ( e.g. unloads the groups and plugins ).
      Should be the last call before the object is deleted.
     */
    void finalize();

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
    Group groupByName( const TQString &name ) const;

    /**
      Adds a new group to the environment.

      @returns the new group.
     */
    Group addGroup( const TQString &name );

    /**
      Removes a group from the environment.
     */
    void removeGroup( const Group &group );

  private:
    OSyncGroupEnv *mGroupEnv;
};

}

#endif
