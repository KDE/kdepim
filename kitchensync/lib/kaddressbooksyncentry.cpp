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

#include "kaddressbooksyncentry.h"

KAddressbookSyncEntry::KAddressbookSyncEntry()
{
    m_addressb = 0;
}
KAddressbookSyncEntry::~KAddressbookSyncEntry()
{
    delete m_addressb;
}

KAddressbookSyncEntry::KAddressbookSyncEntry(KABC::AddressBook *adr )
{
    m_addressb = adr;
}
KABC::AddressBook* KAddressbookSyncEntry::addressbook()
{
    return m_addressb;
}
void KAddressbookSyncEntry::setAddressbook(KABC::AddressBook *book )
{
    m_addressb = book;
}
QString KAddressbookSyncEntry::name()
{
    return m_name;
}

void KAddressbookSyncEntry::setName(const QString &name )
{
    m_name = name;
}

QString KAddressbookSyncEntry::id()
{
    if(m_addressb ==0 )
	return QString::null;
    return m_addressb->identifier();
}

void KAddressbookSyncEntry::setId(const QString & )
{
//
}

QString KAddressbookSyncEntry::oldId()
{
    return m_oldId;
}

void KAddressbookSyncEntry::setOldId(const QString &id )
{
    m_oldId = id;
}


QString KAddressbookSyncEntry::timestamp()
{
    return m_time;
}

void KAddressbookSyncEntry::setTimestamp(const QString &time )
{
    m_time = time;
}

bool KAddressbookSyncEntry::equals(KSyncEntry *other )
{
    if( m_time == other->timestamp() && id() == other->id() )
	return true;
    return false;
}







