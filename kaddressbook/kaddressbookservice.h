/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KADDRESSBOOKSERVICE_H
#define KADDRESSBOOKSERVICE_H

#include <libkdepim/interfaces/AddressBookServiceIface.h>

namespace KAB {
class Core;
}

class KAddressBookService : virtual public KPIM::AddressBookServiceIface
{
  public:
    KAddressBookService( KAB::Core *core );

    /**
      This method will add a vcard to the address book.

      @param vCard The vCard in string representation.
     */
    void importVCard( const QString& vCard );

    /**
      This method will add a vcard to the address book.

      @param url The url where the vcard is located.
     */
    void importVCard( const KURL& url );

  private:
    KAB::Core *mCore;
};

#endif
