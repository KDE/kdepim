/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003,2004 Holger Freyther <zecke@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KSYNC_ADDRESSBOOK_MERGER_H
#define KSYNC_ADDRESSBOOK_MERGER_H

#include "merger.h"

namespace KSync {
class KDE_EXPORT AddressBookMerger : public Merger 
{
public:
    enum Supports {
      FamilyName,
      GivenName,
      AdditionalName,
      Prefix,
      Suffix,
      NickName,
      Birthday,
      HomeAddress,
      BusinessAddress,
      TimeZone,
      Geo,
      Title,
      Role,
      Organization,
      Note,
      Url,
      Secrecy,
      Picture,
      Sound,
      Agent,
      HomeNumbers,
      OfficeNumbers,
      Messenger,
      PreferredNumber,
      Voice,
      Fax,
      Cell,
      Video,
      Mailbox,
      Modem,
      CarPhone,
      ISDN,
      PCS,
      Pager,
      HomeFax,
      WorkFax,
      OtherTel,
      Category,
      Custom,
      Keys,
      Logo,
      Email,
      Emails // more than one
    };

    AddressBookMerger(const QBitArray&);
    ~AddressBookMerger();

    bool merge( SyncEntry* entry, SyncEntry* other );
 private:
    QBitArray mSupports;
};
}

#endif
