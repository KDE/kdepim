/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
#ifndef KSYNC_ADDRESSBOOKSYNCEE_H
#define KSYNC_ADDRESSBOOKSYNCEE_H

#include "syncentry.h"
#include "syncee.h"

#include <kabc/addressbook.h>

namespace KSync {

class AddressBookSyncEntry : public SyncEntry
{
  public:
    typedef QPtrList<AddressBookSyncEntry> PtrList;
    AddressBookSyncEntry( const KABC::Addressee & = KABC::Addressee() );
    AddressBookSyncEntry( const AddressBookSyncEntry& );

    QString name();
    QString id();
    void setId(const QString& id );
    QString timestamp();
    QString type()const;
    bool mergeWith( SyncEntry* );

    SyncEntry* clone();
    bool equals( SyncEntry *entry );

    KABC::Addressee addressee() { return mAddressee; }
    QString resource()const;
    void setResource( const QString& str );

  private:
    KABC::Addressee mAddressee;
    QString m_res;
    struct Data;
    Data* data;
};

/**
  This class provides an implementation of the @KSyncee interface for KSync. It
  provides syncing of AddressBook files.
*/
class AddressBookSyncee : public Syncee
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
        PreferedNumber,
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
    AddressBookSyncee();
    ~AddressBookSyncee();

    AddressBookSyncEntry *firstEntry();
    AddressBookSyncEntry *nextEntry();

//    AddressBookSyncEntry *findEntry(const QString &id);

    void addEntry(SyncEntry *);
    void removeEntry(SyncEntry *);

    bool read();
    bool write();
    SyncEntry::PtrList added();
    SyncEntry::PtrList modified();
    SyncEntry::PtrList removed();
    Syncee* clone();
    QString type() const;
    QString newId()const;

  private:
    AddressBookSyncEntry *createEntry( const KABC::Addressee & );
    SyncEntry::PtrList find( int state);


    QPtrList<AddressBookSyncEntry> mEntries;
};

}

#endif
