/* This file is part of the KDE libraries
    Copyright (C) 2002 Holger Freyther <freyher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include <qdatetime.h>
#include "ksyncentry.h"


KSyncEntry::KSyncEntry()
{
    m_mode = SYNC_NORMAL;
    m_first = true;
}
KSyncEntry::~KSyncEntry()
{

}
void KSyncEntry::insertId( const QString &type,
                           const QString &konnectorId,
                           const QString &kdeId )
{
    QMap<QString,  KontainerList >::Iterator it;
    it = m_maps.find( type );
    if ( it == m_maps.end() ) { // not inserted yet anything
        KontainerList list;
        list.append( Kontainer(konnectorId,  kdeId) );
        m_maps.replace( type, list);
    }else {
        it.data().append(Kontainer( konnectorId,  kdeId) );
    }
}
KontainerList KSyncEntry::ids(const QString &type )const
{
    KontainerList id;
    QMap<QString,  KontainerList >::ConstIterator it;
    it = m_maps.find( type );
    if ( it != m_maps.end() )
        id = it.data();
    return id;
}
QString KSyncEntry::name()
{
    return QString::fromLatin1("KSyncEntry");
}
void KSyncEntry::setName( const QString& )
{
}
QString KSyncEntry::id()
{
    return QString::null;
}
void KSyncEntry::setId( const QString& )
{

}
QString KSyncEntry::oldId()
{
    return QString::null;
}
void KSyncEntry::setOldId( const QString& )
{

}
QString KSyncEntry::timestamp()
{
    return QString::null;
}
void KSyncEntry::setTimestamp( const QString& )
{

}
bool KSyncEntry::equals( KSyncEntry* )
{
    return false;
}
KSyncEntry* KSyncEntry::clone()
{
    return 0l;
}
