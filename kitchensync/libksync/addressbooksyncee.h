/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2004 Cornelius Schumacher <schumacher@kde.org>
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

class AddressBookMerger;
class AddressBookSyncEntry : public SyncEntry
{
    friend class AddressBookMerger;
    friend class AddressBookSyncee;
  public:
    typedef QPtrList<AddressBookSyncEntry> PtrList;

    AddressBookSyncEntry( Syncee* parent );
    AddressBookSyncEntry( const KABC::Addressee &, Syncee *parent );
    AddressBookSyncEntry( const AddressBookSyncEntry & );

    QString name();
    QString id();
    void setId( const QString &id );
    QString timestamp();
    bool mergeWith( SyncEntry * );

    AddressBookSyncEntry *clone();
    bool equals( SyncEntry *entry );

    KABC::Addressee addressee() { return mAddressee; }
    QString resource() const;
    void setResource( const QString &str );

    KPIM::DiffAlgo* diffAlgo( SyncEntry*, SyncEntry* );

  private:
    void setAddressee( const KABC::Addressee& addr );
    KABC::Addressee mAddressee;
    QString m_res;
    struct Data;
    Data *data;
};

/**
  This class provides an implementation of the @see KSyncee interface for KSync.
  It provides syncing of AddressBook files.
*/
class AddressBookSyncee : public Syncee
{
  public:

    AddressBookSyncee( AddressBookMerger* m = 0);
    AddressBookSyncee( KABC::AddressBook *, AddressBookMerger* m = 0 );
    ~AddressBookSyncee();

    void reset();

    AddressBookSyncEntry *firstEntry();
    AddressBookSyncEntry *nextEntry();


    void addEntry( SyncEntry * );
    void removeEntry( SyncEntry * );

    QString type() const;
    QString generateNewId() const;

    bool writeBackup( const QString & ) { return false; }
    bool restoreBackup( const QString & ) { return false; }

  private:
    AddressBookSyncEntry *createEntry( const KABC::Addressee & );
    SyncEntry::PtrList find( int state );

    QPtrList<AddressBookSyncEntry> mEntries;

    KABC::AddressBook *mAddressBook;
    bool mOwnAddressBook : 1;
};

}

#endif
