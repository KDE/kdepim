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

#ifndef QSYNC_GROUP_H
#define QSYNC_GROUP_H

#include <tqdatetime.h>
#include <tqstringlist.h>

class OSyncGroup;

namespace QSync {

class Filter;
class Member;
class Plugin;
class Result;

/**
  @internal
 */
class Group
{
  friend class Engine;
  friend class GroupEnv;

  public:
    enum LockType
    {
      LockOk,
      Locked,
      LockStale
    };

    Group();
    ~Group();

    /**
      Returns whether the object is a valid group.
     */
    bool isValid() const;

    /**
      Sets the name of the group.
     */
    void setName( const TQString &name );

    /**
      Returns the name of the group.
     */
    TQString name() const;

    /**
      Sets the time of the last successfull synchronization.
     */
    void setLastSynchronization( const TQDateTime &dateTime );

    /**
      Returns the time of the last successfull synchronization.
     */
    TQDateTime lastSynchronization() const;

    /**
      Locks the group.

      @returns The the result of the locking request.
     */
    LockType lock();

    /**
      Unlocks the group.
     */
    void unlock();

    /**
      Adds a new member to the group.

      @returns the new member.
     */
    Member addMember( const QSync::Plugin &plugin );

    /**
      Removes a member from the group.
     */
    void removeMember( const Member &member );

    /**
      Returns the number of members.
     */
    int memberCount() const;

    /**
      Returns the member at position @param pos.
     */
    Member memberAt( int pos ) const;

    /**
      Returns the number of filters.
     */
    int filterCount() const;

    /**
      Returns the filter at position @param pos.
     */
    Filter filterAt( int pos );

    /**
      Set, if the object type with given name is enabled for synchronisation for
      this group.
     */
    void setObjectTypeEnabled( const TQString &objectType, bool enabled );

    /**
      Returns whether the object type with given name is enabled for synchronisation for
      this group.
     */
    bool isObjectTypeEnabled( const TQString &objectType ) const;

    /**
      Sets whether this group uses the merger for synchronization.
     */
    void setUseMerger( bool use );

    /**
      Returns whether this group uses the merger for synchronization.
     */
    bool useMerger() const;

    /**
      Sets whether this group uses the converter for synchronization.
     */
    void setUseConverter( bool use );

    /**
      Returns whether this group uses the converter for synchronization.
     */
    bool useConverter() const;

    /**
      Saves the configuration to hard disc.
     */
    Result save();

    bool operator==( const Group &group ) const { return mGroup == group.mGroup; }

    /**
      Removes all group configurations from the hard disc.
     */
    Result cleanup() const;

  private:
    OSyncGroup *mGroup;
};

}

#endif
