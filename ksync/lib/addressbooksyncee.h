/*
    This file is part of ksync.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef ADDRESSBOOKSYNCEE_H
#define ADDRESSBOOKSYNCEE_H

#include <kabc/addressbook.h>
#include <kdepimmacros.h>
#include "ksyncer.h"

class KDE_EXPORT AddressBookSyncEntry : public KSyncEntry
{
  public:
    AddressBookSyncEntry( const KABC::Addressee & );

    QString name();
    QString id();
    QString timestamp();

    bool equals( KSyncEntry *entry );

    KABC::Addressee addressee() { return mAddressee; }

  private:
    KABC::Addressee mAddressee;
};

/**
  This class provides an implementation of the @see KSyncee interface for KSync.
  It provides syncing of AddressBook files.
*/
class KDE_EXPORT AddressBookSyncee : public KSyncee
{
  public:
    AddressBookSyncee();
    ~AddressBookSyncee();

    AddressBookSyncEntry *firstEntry();
    AddressBookSyncEntry *nextEntry();

//    AddressBookSyncEntry *findEntry(const QString &id);

    void addEntry(KSyncEntry *);
    void removeEntry(KSyncEntry *);

    bool read();
    bool write();

  private:
    AddressBookSyncEntry *createEntry( const KABC::Addressee & );

    KABC::AddressBook *mAddressBook;

    KABC::AddressBook::Iterator mAddressBookIterator;

    QPtrList<AddressBookSyncEntry> mEntries;
};

#endif
