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

#include "profilefilemanager.h"
#include "profilemanager.h"

using namespace KSync;

ProfileManager::ProfileManager() {

}
ProfileManager::ProfileManager( const Profile::ValueList& list )
    : m_list( list ) {
}
ProfileManager::~ProfileManager() {

}

Profile ProfileManager::currentProfile() const {
    return m_cur;
}
void ProfileManager::setCurrentProfile( const Profile& prof) {
    m_cur = prof;
}
Profile::ValueList ProfileManager::profiles() const {
    return m_list;
}
void ProfileManager::setProfiles( const Profile::ValueList& list ) {
    m_list = list;
    m_cur = Profile(); // invalidate
}
Profile ProfileManager::byName(const QString& name ) {
    Profile prof;

    Profile::ValueList::Iterator it;
    for ( it = m_list.begin(); it != m_list.end(); ++it ) {
        if ( (*it).name() == name ) {
            prof = (*it);
            break;
        }
    };
    return prof;
}
Profile::ValueList ProfileManager::byName2( const QString& name ) {
    Profile::ValueList list;
    Profile::ValueList::Iterator it;
    for ( it = m_list.begin(); it != m_list.end(); ++it ) {
        if ( (*it).name() == name )
            list.append( (*it) );
    };
    return list;
}
/*
 * Load from KConfig
 * there is a Global group for the current Profile ( id )
 * and one Group for each Profile
 *
 */
void ProfileManager::load() {
    ProfileFileManager man;
    m_list = man.load();
}

void ProfileManager::save() {
    ProfileFileManager man;
    man.save( m_list );
}
void ProfileManager::addProfile( const Profile& prof ) {
    m_list.append( prof );
}
void ProfileManager::replaceProfile( const Profile& prod ) {
    m_list.remove( prod );
    m_list.append( prod );
}
void ProfileManager::removeProfile( const Profile& prof ) {
    m_list.remove( prof );
}
Profile ProfileManager::profile( int i)const {
    return m_list[i];
}
int ProfileManager::count() const {
    return m_list.count();
}
