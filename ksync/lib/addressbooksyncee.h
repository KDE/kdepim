#ifndef ADDRESSBOOKSYNCEE_H
#define ADDRESSBOOKSYNCEE_H

#include <kabc/addressbook.h>

#include "ksyncer.h"

class AddressBookSyncEntry : public KSyncEntry
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
  This class provides an implementation of the @KSyncee interface for KSync. It
  provides syncing of AddressBook files.
*/
class AddressBookSyncee : public KSyncee
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
