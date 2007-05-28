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
#ifndef CONTACTPTRLIST_H
#define CONTACTPTRLIST_H

#include <libkmobiletools/kmobiletools_export.h>

#include <kabc/addressee.h>

/**
@author Marco Gulino
*/
class ContactsListPrivate;
namespace KMobileTools
{
    typedef QListIterator<KABC::Addressee> ContactsListIterator;
    class KMOBILETOOLS_EXPORT ContactsList : public KABC::Addressee::List
    {
    public:
        ContactsList();
        ContactsList(const KABC::Addressee::List&);
        ~ContactsList();
    KABC::Addressee findAddressee(int memslot, const QString & index);
    KABC::Addressee findAddressee(const QString &posInfos);
    ContactsList& operator =(const KMobileTools::ContactsList&);
    private:
        ContactsListPrivate *const d;
    };
}
#endif
