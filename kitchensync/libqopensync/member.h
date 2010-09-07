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

#include <tqstring.h>

class OSyncMember;

namespace QSync {

class Result;

class Member
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
    TQString configurationDirectory() const;

    /**
      Returns the name of the plugin, the member belongs to.
     */
    TQString pluginName() const;

    /**
      Returns the id of the plugin.
    */
    int id() const;

    /**
      Sets the name of this member.
     */
    void setName( const TQString &name );

    /**
      Returns the name of this member.
     */
    TQString name() const;

    /**
      Sets the configuration data as byte array. The developer has to decide the
      type of the data ( e.g. xml, plain text, binary ).
     */
    void setConfiguration( const TQByteArray &configurationData );

    /**
      Gets the configuration data as byte array. The developer has to decide the
      type of the data ( e.g. xml, plain text, binary ).

      @param useDefault  If set to true, return default config, if no config
                         exists. If set to false, return error when no config
                         exists.

      @returns The result of this operation.
     */
    Result configuration( TQByteArray &configurationData,
      bool useDefault = true );

    /**
      Saves the changes to the configuration to hard disc.
     */
    Result save();

    /**
      Make this member an instance of the given plugin.
    */
    Result instance();

    bool operator==( const Member& ) const;

    /**
      This method can be used to query the plugin for scanning devices.
      The @param query is a plugin specific xml document as well as
      the return value.
     */
    TQString scanDevices( const TQString &query );

    /**
      This method can be used to test whether the plugin can connect
      to the device with the given configuration.
     */
    bool testConnection( const TQString &configuration );


    /**
     * Deletes the member's information from the hard disc.
     */
    Result cleanup() const;

  private:
    OSyncMember *mMember;
};

}

#endif

