
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <ksync_sync.h>
#include "sync.h"

using namespace KitchenSync;

SyncAddressbook::SyncAddressbook( QObject* obj,  const char* name ,  const QStringList&  )
    : SyncPlugin( obj,  name )
{

};
SyncAddressbook::~SyncAddressbook()
{

}
SyncReturn SyncAddressbook::sync(int mode,  KSyncEntry* in,
                                 KSyncEntry* in2 )
{
    blackIds1.clear();
    m_mode = mode;
    if (in->type() != QString::fromLatin1("KAddressbookSyncEntry") )
        return SyncReturn();
    if (in2->type() != QString::fromLatin1("KAddressbookSyncEntry") )
        return SyncReturn();
    KAddressbookSyncEntry* entry1 = (KAddressbookSyncEntry*) in;
    KAddressbookSyncEntry* entry2 = (KAddressbookSyncEntry*) in2;
    kdDebug() << "Entry1 SyncMode " << entry1->syncMode() << " is first sync " << entry1->firstSync() << endl;
    kdDebug() << "Entry2 SyncMode " << entry2->syncMode() << " is first sybc " << entry2->firstSync() <<  endl;
    KABC::AddressBook *book = new KABC::AddressBook();
    m_entry = new KAddressbookSyncEntry();
    m_entry->setAddressbook( book );
    if (entry2->syncMode() == KSyncEntry::SYNC_NORMAL && entry1->syncMode() == KSyncEntry::SYNC_NORMAL ) {
        syncNormal( entry1,  entry2 );
    }else if ( entry2->syncMode() == KSyncEntry::SYNC_NORMAL && entry1->syncMode() == KSyncEntry::SYNC_META ) {
        syncNormal(entry1,  entry2 );
    }else if ( entry2->syncMode() == KSyncEntry::SYNC_META && entry1->syncMode() == KSyncEntry::SYNC_NORMAL ) {
        syncNormal( entry1,  entry2 );
    }else if ( entry1->syncMode() == KSyncEntry::SYNC_META && entry1->syncMode() == KSyncEntry::SYNC_META ) {
        if ( entry1->firstSync() || entry2->firstSync() ) {
//            m_mode = 10;
            syncNormal( entry1,  entry2 );
        }else{
            syncMeta( entry1,  entry2 );
        }
    }
    KSyncEntry::List synced;
    KSyncEntry::List in3;
    KSyncEntry::List in4;
    synced.append(m_entry );
    SyncReturn ret( synced, in3,  in4 );
    return ret;

}
void SyncAddressbook::syncAsync(int /*mode*/,  KSyncEntry* /*in1*/,
                                KSyncEntry */*in2*/ )
{

}
void SyncAddressbook::syncMeta( KAddressbookSyncEntry *entry1,
                                KAddressbookSyncEntry *entry2 )
{
    kdDebug() << "Sync Meta " << endl;
  // add added
    QValueList<KABC::Addressee> list1 = entry1->added();
    QValueList<KABC::Addressee> list2 = entry2->added();
    QValueList<KABC::Addressee>::Iterator it1;
    QValueList<KABC::Addressee>::Iterator it2;
    syncAdded( list1, list2 );
    syncAdded( list2, list1 );
  // set modified
    list1 = entry1->modified();
    list2 = entry2->modified();
    QValueList<KABC::Addressee> list3 = entry2->deleted();
    bool found;
    for ( it1 = list1.begin(); it1 != list1.end(); ++it1 ) {
        found = false;
        for (it2 = list2.begin(); it2  != list2.end(); ++it2 ) { // check if modified
            if ( (*it1).uid() == (*it2).uid() ) { // ok both sides modified it
                found = true;
                blackIds1 << (*it1).uid();
//                blackIds1 << (*it2).uid(); ids are the same
                // deconflict FIXME
                m_entry->addressbook()->insertAddressee( (*it1) );
                break;
            }
            // check if deleted in the other
        }
        // check if deleted in the other
        for (it2 = list3.begin(); it2 != list3.end(); ++it2 ) {
            if ( (*it1).uid() == (*it2).uid() ) {
                blackIds1 << (*it1).uid();
                m_entry->addressbook()->insertAddressee( (*it1) );
                break;
            }
        }
    }
    // check list2 for modified and removed
    list3 = entry1->deleted();
    for (it2 = list2.begin(); it2 != list2.end(); ++it2 ) {
        for (it1 = list3.begin(); it1 != list3.end(); ++it1 ) {
            if ((*it1).uid() == (*it2).uid() ) {
                m_entry->addressbook()->insertAddressee((*it2) );
                blackIds1 << (*it1).uid();
                break;
            }
        }
    }
    // check for removed
    list1 = entry1->deleted();
    list2 = entry2->deleted();
    for (it1 = list1.begin(); it1 != list1.end(); ++it1 ) {
        blackIds1  << (*it1).uid();
    }
    for (it2 = list2.begin(); it2 != list2.end(); ++it2 ) {
        blackIds1 << (*it2).uid();
    }
  // apply rest
    KABC::AddressBook::Iterator itAB1;
    KABC::AddressBook::Iterator itAB2;
    for ( itAB1 = entry1->addressbook()->begin(); itAB1 != entry1->addressbook()->end(); ++itAB1 ) {
        if ( !blackIds1.contains( (*itAB1).uid() ) ) {
            blackIds1 << "Adding " ;
            kdDebug() << "adding" << (*itAB1).givenName() << endl;
            kdDebug() << "not containing" << endl;
            m_entry->addressbook()->insertAddressee( (*itAB1) );
        }
    }
}
void SyncAddressbook::syncNormal( KAddressbookSyncEntry* entry1,
                                  KAddressbookSyncEntry* entry2 )
{
    kdDebug() << "Sync Normal" << endl;
    syncAdded( entry1->addressbook(), entry2->addressbook() );
    syncAdded( entry2->addressbook(), entry1->addressbook() );
}
void SyncAddressbook::syncAdded( const QValueList<KABC::Addressee>& added,
                                 const QValueList<KABC::Addressee>& added2 )
{
    kdDebug() << "Sync Added new" << endl;
    kdDebug() << "added1 " << added.isEmpty() << endl;
    kdDebug() << "added2 " << added2.isEmpty() << endl;
    QValueList<KABC::Addressee>::ConstIterator it1;
    QValueList<KABC::Addressee>::ConstIterator it2;
    bool found;
    for ( it1 = added.begin(); it1 != added.end(); ++it1 ) {
        kdDebug() << "name " << (*it1).givenName() << endl;
        found = false;
        blackIds1 << (*it1).uid();
        if ( (*it1).uid().startsWith("Konnector-") ) {
            kdDebug() << "Added Konnector-" << endl;
            found = true;
            QString id = kapp->randomString(10 );
            m_entry->insertId( "addressbook",  (*it1).uid(),  id );
            KABC::Addressee adr = (*it1);
            adr.setUid( id );
            m_entry->addressbook()->insertAddressee( adr );
            continue;
        }
        for ( it2 = added2.begin(); it2 != added2.end(); ++it2 ) {
            found = false;
            if ( (*it1).uid() == (*it2).uid() ) { // conflict resolve
                blackIds1 << (*it2).uid();
                kdDebug() << "Id present " << endl;
                found = true;
                switch( m_mode ) {
                case SYNC_FIRSTOVERRIDE:
                    m_entry->addressbook()->insertAddressee( (*it1) );
                    break;
                case SYNC_SECONDOVERRIDE:
                    m_entry->addressbook()->insertAddressee( (*it2) );
                    break;
                case 10:
                    m_entry->addressbook()->insertAddressee( (*it1) );
                    m_entry->addressbook()->insertAddressee( (*it2) );
                    break;
                case SYNC_INTERACTIVE:
                default: {
                    QString text = i18n("Which entry do you want to take precedence?\n");
                    text += i18n("Entry 1: '%1'\n").arg( (*it1).sortString() );
                    text += i18n("Entry 2: '%1'\n").arg( (*it2).sortString() );
                    int result = KMessageBox::questionYesNo(0l,  text,  i18n("Resolve Conflict"),
                                                            i18n("Entry 1"),  i18n("Entry 2") );
                    if ( result == KMessageBox::Yes ) {
                        m_entry->addressbook()->insertAddressee( (*it1) );
                    }else if ( result == KMessageBox::No ) {
                        m_entry->addressbook()->insertAddressee( (*it2) );
                    }
                    break;

                };

                } // off switch
            }

        }
        if (!found ) {
                kdDebug() << "Adding new entry " << (*it1).givenName() << endl;
                m_entry->addressbook()->insertAddressee( (*it1 ) );
        }
    }
}
void SyncAddressbook::syncAdded( const KABC::AddressBook *added,
                                 const KABC::AddressBook *added2 )
{
    kdDebug() << "syncAdded" << endl;
    KABC::AddressBook::ConstIterator it1;
    KABC::AddressBook::ConstIterator it2;
    bool found;
    for ( it1 = added->begin(); it1 != added->end(); ++it1 ) {
        found = false;
        blackIds1 << (*it1).uid();
        if ( (*it1).uid().startsWith("Konnector-") ) {
            kdDebug() << "Added Konnector " << endl;
            found = true;
            QString id = kapp->randomString(10 );
            m_entry->insertId( "addressbook",  (*it1).uid(),  id );
            KABC::Addressee adr = (*it1);
            adr.setUid( id );
            m_entry->addressbook()->insertAddressee( adr );
            continue;
        }
        for ( it2 = added2->begin(); it2 != added2->end(); ++it2 ) {
            found = false;
            if ( (*it1).uid() == (*it2).uid() ) { // conflict resolve
                blackIds1 << (*it2).uid();
                found = true;
                switch( m_mode ) {
                case SYNC_FIRSTOVERRIDE:
                    m_entry->addressbook()->insertAddressee( (*it1) );
                    break;
                case SYNC_SECONDOVERRIDE:
                    m_entry->addressbook()->insertAddressee( (*it2) );
                    break;
                case 10:
                    m_entry->addressbook()->insertAddressee( (*it1) );
                    m_entry->addressbook()->insertAddressee( (*it2) );
                    break;
                case SYNC_INTERACTIVE:
                default: {
                    QString text = i18n("Which entry do you want to take precedence?\n");
                    text += i18n("Entry 1: '%1'\n").arg( (*it1).sortString() );
                    text += i18n("Entry 2: '%1'\n").arg( (*it2).sortString() );
                    int result = KMessageBox::questionYesNo(0l,  text,  i18n("Resolve Conflict"),
                                                            i18n("Entry 1"),  i18n("Entry 2") );
                    if ( result == KMessageBox::Yes ) {
                        m_entry->addressbook()->insertAddressee( (*it1) );
                    }else if ( result == KMessageBox::No ) {
                        m_entry->addressbook()->insertAddressee( (*it2) );
                    }
                    break;

                };

                } // off switch
            }
            if (!found ) {
                kdDebug() << "not found adding" << endl;
                m_entry->addressbook()->insertAddressee( (*it1 ) );
            }
        }
    }
}
