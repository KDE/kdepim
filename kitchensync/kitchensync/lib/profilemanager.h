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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KSYNC_PROFILE_MANAGER_H
#define KSYNC_PROFILE_MANAGER_H

#include <qstring.h>

#include <profile.h>

namespace KSync {

    /**
     * ProfileManager keeps track of the Profiles
     * It allows you to retrieve and set the current.
     * remove and add new/old Profiles
     * Load and Save from KConfig
     */
    class ProfileManager  {
    public:
        /**
         * Constructs an Empty ProfileManager
         */
        ProfileManager();

        /**
         * Constructs a profile manager from a Profile List.
         */
        ProfileManager( const Profile::ValueList& list );

        /**
         * Destructs a profile manager
         */
        ~ProfileManager();

        /**
         * returns the current active Profile
         */
        Profile currentProfile()const;

        /**
         * sets the current Profile
         */
        void setCurrentProfile( const Profile& profile );

        /**
         * returns a list of all active profiles
         */
        Profile::ValueList profiles()const;

        /**
         * set the Manager to use a list of Profiles
         */
        void setProfiles( const Profile::ValueList& list );

        /**
         * is finding a Profile by name
         */
        Profile byName( const QString& name );

        /**
         * returns a profile list of of Profiles matching name
         */
        Profile::ValueList byName2( const QString& name );

        /*
         * profile at index
         */
        Profile profile( int index )const;

        /**
         * the count of elements
         */
        int count()const;

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
        void addProfile( const Profile& );

        /**
         * replaces a profile
         */
        void replaceProfile( const Profile& );

        /**
         * removes a Profile
         */
        void removeProfile( const Profile& );

    private:
        Profile m_cur;
        Profile::ValueList m_list;
    };
};


#endif
