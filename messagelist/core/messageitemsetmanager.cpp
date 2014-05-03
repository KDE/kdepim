/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "core/messageitemsetmanager.h"
#include "core/messageitem.h"

using namespace MessageList::Core;

MessageItemSetManager::MessageItemSetManager()
{
    mSets = new QHash< MessageItemSetReference, QHash< MessageItem *, MessageItem * > * >();
}

MessageItemSetManager::~MessageItemSetManager()
{
    clearAllSets();
    delete mSets;
}

void MessageItemSetManager::clearAllSets()
{
    QHash< MessageItemSetReference, QHash< MessageItem *, MessageItem * > * >::ConstIterator end( mSets->constEnd() );
    for ( QHash< MessageItemSetReference, QHash< MessageItem *, MessageItem * > * >::ConstIterator it = mSets->constBegin(); it != end; ++it )
        delete ( *it );

    mSets->clear();
}

int MessageItemSetManager::setCount() const
{
    return mSets->count();
}


void MessageItemSetManager::removeSet( MessageItemSetReference ref )
{
    QHash< MessageItem *, MessageItem * > * set = mSets->value( static_cast< unsigned long int >( ref ), 0 );
    if ( !set )
        return;
    mSets->remove( static_cast< unsigned long int >( ref ) );
    delete set;
}

QList< MessageItem * > MessageItemSetManager::messageItems( MessageItemSetReference ref )
{
    QList< MessageItem * > ret;
    QHash< MessageItem *, MessageItem * > * set = mSets->value( static_cast< unsigned long int >( ref ), 0 );
    if ( !set )
        return ret;

    QHash< MessageItem *, MessageItem * >::ConstIterator end( set->constEnd() );
    for ( QHash< MessageItem *, MessageItem * >::ConstIterator it = set->constBegin(); it != end; ++it )
        ret.append( *it );

    return ret;
}

void MessageItemSetManager::removeMessageItemFromAllSets( MessageItem * mi )
{
    QList< MessageItemSetReference > setsToBeKilled;
    QHash< MessageItemSetReference, QHash< MessageItem *, MessageItem * > * >::ConstIterator end( mSets->end());

    for ( QHash< MessageItemSetReference, QHash< MessageItem *, MessageItem * > * >::ConstIterator it = mSets->constBegin(); it != end; ++it )
    {
        ( *it )->remove( mi );
        if ( ( *it )->isEmpty() )
            setsToBeKilled.append( it.key() );
    }

    QList< MessageItemSetReference >::ConstIterator endit2( setsToBeKilled.constEnd() );
    for ( QList< MessageItemSetReference >::ConstIterator it2 = setsToBeKilled.constBegin(); it2 != endit2; ++it2 )
        removeSet( *it2 );
}

MessageItemSetReference MessageItemSetManager::createSet()
{
    static unsigned long int uNextId = 0;
    uNextId++;

    QHash< MessageItem *, MessageItem * > * set = mSets->value( uNextId, 0 );

    // Here we assume that having 2^32 sets is impossible
    // (that would be _at_least_ 2^32 * 8 bytes of allocated memory which is
    // quite a lot...)

    while( set )
    {
        uNextId++;
        set = mSets->value( uNextId, 0 );
    }

    set = new QHash< MessageItem *, MessageItem * >();
    mSets->insert( uNextId, set );

    return static_cast< MessageItemSetReference >( uNextId );
}

bool MessageItemSetManager::addMessageItem( MessageItemSetReference ref, MessageItem * mi )
{
    QHash< MessageItem *, MessageItem * > * set = mSets->value( static_cast< unsigned long int >( ref ), 0 );
    if ( !set )
        return false;

    set->insert( mi, mi );

    return true;
}

