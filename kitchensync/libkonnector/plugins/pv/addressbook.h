/***************************************************************************
                        addressbook.h  -  description
                             -------------------
    begin                : Wed Oct 23 2002
    copyright            : (C) 2002 by Maurus Erni
    email                : erni@pocketviewer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef addressbook_h
#define addressbook_h

#include <qdom.h>

#include <kabc/resourcefile.h>
#include <kabc/phonenumber.h>
#include <kabc/address.h>

#include <addressbooksyncee.h>

/** This is a class to handle conversions of the addressbook. The addressbook
  * can be converted from a QDomNode to a AddressBookSyncee* and vice versa.
  * @author Maurus Erni
  */

namespace PVHelper
{
  class AddressBook
  {
    public:
      /**
        * Converts a QDomNode to an AddressBookSyncee*.
        * @param node The node (part of an XML document) to be converted
        * @return KSync::AddressBookSyncee* The converted addressbook
        */
      static KSync::AddressBookSyncee* toAddressBookSyncee(QDomNode& node);

      /**
        * Converts an AddressBookSyncee* to a QString which represents a
        * DOM node.
        * @param syncee The syncee to be converted
        * @return QString The converted addressbook as an XML string
        */
      static QString toXML(KSync::AddressBookSyncee* syncee);

      /* xxx not used yet. first meta sync has to be implemented!
      static KSync::AddressBookSyncee* doMeta(KSync::AddressBookSyncee* AdrSyncOld,
                                               KSync::AddressBookSyncee* AdrSyncNew);*/
    private:
      /**
        * Converts a QString representing the address to a KABC::Address
        * @param strAddr The QString address to be converted
        * @param type The type of the address (e.g. home, work)
        * @return KABC::Address The converted address
        */
      static KABC::Address strToAddress(const QString& strAddr, KABC::Address::Type type);

      /**
        * Converts an KABC::Address to a QString
        * @param KABC::Address The address to be converted
        * @return QSting The converted address
        */
      static QString addressToStr(const KABC::Address& addr);
  };
}

#endif
