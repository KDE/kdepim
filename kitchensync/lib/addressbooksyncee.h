#ifndef ADDRESSBOOKSYNCEE_H
#define ADDRESSBOOKSYNCEE_H

#include <kabc/addressbook.h>

#include "syncer.h"

namespace KSync {

    class AddressBookSyncEntry : public SyncEntry
    {
    public:
        enum Supports {
            Name = 0,
            FamilyName,
            GivenName,
            AdditionalName
        };
        typedef QPtrList<AddressBookSyncEntry> PtrList;
        AddressBookSyncEntry( const KABC::Addressee & = KABC::Addressee() );
        AddressBookSyncEntry( const AddressBookSyncEntry& );

        QString name();
        QString id();
        void setId(const QString& id );
        QString timestamp();
        QString type()const;

        SyncEntry* clone();
        bool equals( SyncEntry *entry );

        KABC::Addressee addressee() { return mAddressee; }

    private:
        KABC::Addressee mAddressee;
    };

/**
   This class provides an implementation of the @KSyncee interface for KSync. It
   provides syncing of AddressBook files.
*/
    class AddressBookSyncee : public Syncee
    {
    public:
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
