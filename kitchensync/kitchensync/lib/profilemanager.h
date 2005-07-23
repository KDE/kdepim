/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
#ifndef KSYNC_PROFILEMANAGER_H
#define KSYNC_PROFILEMANAGER_H

#include "profileconfig.h"

#include <profile.h>

#include <qstring.h>

namespace KSync {

/**
 * ProfileManager keeps track of the Profiles
 * It allows you to retrieve and set the current.
 * remove and add new/old Profiles
 * Load and Save from KConfig
 */
class ProfileManager
{
  public:
    /**
     * Constructs an Empty ProfileManager
     */
    ProfileManager();

    /**
     * Constructs a profile manager from a Profile List.
     */
    ProfileManager( const Profile::List &list );

    /**
     * Destructs a profile manager
     */
    ~ProfileManager();

    /**
     * returns the current active Profile
     */
    Profile currentProfile() const;

    /**
     * sets the current Profile
     */
    void setCurrentProfile( const Profile &profile );

    /**
     * returns a list of all active profiles
     */
    Profile::List profiles() const;

    /**
     * set the Manager to use a list of Profiles
     */
    void setProfiles( const Profile::List &list );

    /**
     * is finding a Profile by name
     */
    Profile byName( const QString &name );

    /**
     * returns a profile list of of Profiles matching name
     */
    // FIXME: byName2 is not a very expressive name
    Profile::List byName2( const QString &name );

    /*
     * profile at index
     */
    Profile profile( int index ) const;

    /**
     * the count of elements
     */
    int count() const;

    /**
     * loads a Profile List
     */
    void load();

    /**
     * saves current list including current Profile
     */
    void save();

    /**
     * add a Profile
     */
    void addProfile( const Profile & );

    /**
     * replaces a profile
     */
    void replaceProfile( const Profile & );

    /**
     * removes a Profile
     */
    void removeProfile( const Profile & );

  private:
    ProfileConfig mProfileConfig;
  
    Profile mCurrentProfile;
    Profile::List mProfiles;
};

}


#endif
