#ifndef ADDRESSBOOKSYNCEE_H
#define ADDRESSBOOKSYNCEE_H

#include <kabc/addressbook.h>

#include "syncer.h"

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
