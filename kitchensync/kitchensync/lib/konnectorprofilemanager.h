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
#ifndef KSYNC_KONNECTOR_PROFILE_MANAGER
#define KSYNC_KONNECTOR_PROFILE_MANAGER

#include "konnectorprofile.h"

namespace KSync {

/**
 * Manages loading and saving of profiles
 * Manages current Profiles... adding, removing
 * and finding them
 */
class KonnectorProfileManager
{
  public:
    KonnectorProfileManager(const KonnectorProfile::ValueList& );
    KonnectorProfileManager();
    ~KonnectorProfileManager();

    /**
     * find searches for @param id inside the uid and the name
     * of profiles
     * @return a KonnectorProfile
     */
    KonnectorProfile find( const QString& id );

    /**
     * @return a KonnectorProfile for Device
     */
    KonnectorProfile find( const Device& dev );

    /**
     * @return the list of KonnectorProfiles
     */
    KonnectorProfile::ValueList list() const;

    /**
     * set the internal ProfileList to @param list
     * @param list the list of KonnectorProfiles
     */
    void setList( const KonnectorProfile::ValueList& list );

    /**
     * @return the currently activated KonnectorProfile
     */
    KonnectorProfile current() const;

    /**
     * set the current KonnectorProfile
     * @param prof The profile
     */
    void setCurrent( const KonnectorProfile& prof );

    /**
     * add a KonnectorProfile
     * @param prof The KonnectorProfile
     */
    void add( const KonnectorProfile& prof );

    /**
     * replace the KonnectorProfile
     * @param prof the KonnectorProfile
     */
    void replace( const KonnectorProfile& prof );

    /**
     * remove the KonnectorProfile from the internal list
     * @param prof The KonnectorProfile
     */
    void remove( const KonnectorProfile& prof );

    /**
     * clear the list of profiles
     */
    void clear();

    /**
     * @return the number of KonnectorProfiles
     */
    uint count()const;

    /**
     * @return the KonnectorProfile at index
     * @param index the index of the KonnectorProfile
     */
    KonnectorProfile profile( uint index )const;

    /**
     * load
     */
    void load();

    /**
     * save
     */
    void save();

  private:
    KonnectorProfile::ValueList m_list;
    KonnectorProfile m_current;

};

}

#endif
