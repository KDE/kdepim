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

namespace PVHelper
{
  class AddressBook
  {
    public:
      static KSync::AddressBookSyncee* toAddressBookSyncee(QDomNode& node);

      static QString toXML(KSync::AddressBookSyncee* syncee);
      
      /* xxx not used yet. first meta sync has to be implemented!      
      static KSync::AddressBookSyncee* doMeta(KSync::AddressBookSyncee* AdrSyncOld,
                                               KSync::AddressBookSyncee* AdrSyncNew);*/
    private:
      static KABC::Address strToAddress(const QString& strAddr, KABC::Address::Type type);
      static QString addressToStr(const KABC::Address& addr);
  };
}

#endif
