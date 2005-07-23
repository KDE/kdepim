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

#include "profilemanager.h"

using namespace KSync;

ProfileManager::ProfileManager()
{
}

ProfileManager::ProfileManager( const Profile::List &list )
    : mProfiles( list )
{
}

ProfileManager::~ProfileManager()
{
}

Profile ProfileManager::currentProfile() const
{
    return mCurrentProfile;
}

void ProfileManager::setCurrentProfile( const Profile &prof )
{
    mCurrentProfile = prof;
}

Profile::List ProfileManager::profiles() const
{
    return mProfiles;
}

void ProfileManager::setProfiles( const Profile::List &list )
{
    mProfiles = list;
    mCurrentProfile = Profile(); // invalidate
}

Profile ProfileManager::byName( const QString &name )
{
    Profile prof;

    Profile::List::Iterator it;
    for ( it = mProfiles.begin(); it != mProfiles.end(); ++it ) {
        if ( (*it).name() == name ) {
            prof = (*it);
            break;
        }
    }
    return prof;
}

Profile::List ProfileManager::byName2( const QString& name )
{
    Profile::List list;
    Profile::List::Iterator it;
    for ( it = mProfiles.begin(); it != mProfiles.end(); ++it ) {
        if ( (*it).name() == name )
            list.append( (*it) );
    }
    return list;
}

/*
 * Load from KConfig
 * there is a Global group for the current Profile ( id )
 * and one Group for each Profile
 *
 */
void ProfileManager::load()
{
    mProfiles = mProfileConfig.load();
}

void ProfileManager::save()
{
    mProfileConfig.save( mProfiles );
}

void ProfileManager::addProfile( const Profile &prof )
{
    mProfiles.append( prof );
}

void ProfileManager::replaceProfile( const Profile &prod )
{
    mProfiles.remove( prod );
    mProfiles.append( prod );
}

void ProfileManager::removeProfile( const Profile &prof )
{
    mProfiles.remove( prof );
}

Profile ProfileManager::profile( int i ) const
{
    return mProfiles[i];
}

int ProfileManager::count() const
{
    return mProfiles.count();
}
