/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "contactslist.h"
#include <kdebug.h>

using namespace KMobileTools;

class ContactsListPrivate {
    public:
        ContactsListPrivate() {}
};

ContactsList::ContactsList()
    : KABC::Addressee::List(), d(new ContactsListPrivate)
{
}

ContactsList::ContactsList(const KABC::Addressee::List& addresseeList)
    : KABC::Addressee::List(), d(new ContactsListPrivate)
{
    KABC::Addressee::List::ConstIterator it;
    for (it=addresseeList.begin(); it!=addresseeList.end(); ++it)
        append(KABC::Addressee(*it));
}


ContactsList::~ContactsList()
{
    delete d;
}

ContactsList& ContactsList::operator =(const KMobileTools::ContactsList& cl)
{
    *d=*(cl.d);
    return *this;
}



/*!
    \fn KMobileTools::ContactsList::findAddressee(int memslot, int index)
 */
KABC::Addressee KMobileTools::ContactsList::findAddressee(int memslot, const QString &index)
{
    KABC::Addressee retval;
    for(KABC::Addressee::List::ConstIterator it=begin(); it!=end(); ++it)
        if( (*it).custom("KMobileTools","memslot").toInt() == memslot && (*it).custom("KMobileTools","index") == index) return *it;
    return KABC::Addressee();
}


/*!
    \fn KMobileTools::ContactsList::findAddressee(const QString &posInfos)
 */
KABC::Addressee KMobileTools::ContactsList::findAddressee(const QString &posInfos)
{
    if( posInfos.contains('-') != 1 ) return KABC::Addressee();
    int pos=posInfos.indexOf('-');
    int memslot=posInfos.left(pos).toInt();
    if( ! memslot || ! posInfos.mid(pos+1).length() ) return KABC::Addressee();
    return findAddressee(memslot, posInfos.mid(pos+1));
}
