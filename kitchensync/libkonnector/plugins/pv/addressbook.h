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
#include <qbuffer.h>

#include <addressbooksyncee.h>

namespace PVHelper
{
  class AddressBook
  {
    public:
      static KSync::AddressBookSyncee* toAddressBookSyncee(QDomNode& node);

      static QByteArray toXML(KSync::AddressBookSyncee* syncee);
  };
}

#endif
