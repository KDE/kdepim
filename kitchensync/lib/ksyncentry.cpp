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

}
KSyncEntry::~KSyncEntry()
{

}
void KSyncEntry::insertId( const QString &type,
                           const QString &konnectorId,
                           const QString &kdeId )
{
    QMap<QString,  QValueList<Kontainer > >::Iterator it;
    it = m_maps.find( type );
    if ( it == m_maps.end() ) { // not inserted yet anything
        QValueList<Kontainer > list;
        list.append( Kontainer(konnectorId,  kdeId) );
        m_maps.replace( type, list);
    }else {
        it.data().append(Kontainer( konnectorId,  kdeId) );
    }
}
QValueList<Kontainer> KSyncEntry::ids(const QString &type )const
{
    QValueList<Kontainer> id;
    QMap<QString,  QValueList<Kontainer > >::ConstIterator it;
    it = m_maps.find( type );
    if ( it != m_maps.end() )
        id = it.data();
    return id;
}
