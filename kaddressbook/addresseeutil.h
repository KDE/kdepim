/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

#ifndef ADDRESSEEUTIL_H
#define ADDRESSEEUTIL_H

#include <qstring.h>
#include <kabc/addressee.h>

/**
  This class provides some utility methods for transposing an
  addressee to different types (ie: clipboard). This class
  is probably just temporary until these function stablize and move
  to KABC.

  NOTE: Currently these methods are not implemented properly. The
  vCard parser in KABC needs to be updated and there is no way to get from
  KABC::Addressee to vCard.
*/
class AddresseeUtil
{
  public:
    /**
      Same as above function, except that an entire list of KABC::Addressee
      objects will be converted to vCard and put in the string.
     */
    static QString addresseesToClipboard( const KABC::Addressee::List &addrList );

    /**
      Convert a string from the clipboard into a list of addressee objects.
      If the clipboard text was not a valid vcard, an empty list
      will be returned.
     */
    static KABC::Addressee::List clipboardToAddressees( const QString &clipboard );

    /**
      Converts the list of addressee objects into a list of email addresses.
     */
    static QString addresseesToEmails( const KABC::Addressee::List &addrList );

  private:
    AddresseeUtil() {}
    ~AddresseeUtil() {}
};

#endif
