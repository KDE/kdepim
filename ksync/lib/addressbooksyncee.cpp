#include <kdebug.h>
#include <kabc/resource.h>
#include <kresources/manager.h>

#include "addressbooksyncee.h"

AddressBookSyncEntry::AddressBookSyncEntry( const KABC::Addressee &a ) :
  mAddressee( a )
{
}

QString AddressBookSyncEntry::name()
{
  return mAddressee.realName();
}

QString AddressBookSyncEntry::id()
{
  return mAddressee.uid();
}

QString AddressBookSyncEntry::timestamp()
{
  return QString::null;
}

bool AddressBookSyncEntry::equals( KSyncEntry *entry )
{
  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>(entry);
  if ( !abEntry ) {
    kdDebug() << "AddressBookSyncee::equals(): Wrong type." << endl;
    return false;
  }

  if ( mAddressee == abEntry->addressee() ) {
    kdDebug() << "AddressBookSyncEntry::equals(): '" << entry->name() << "':"
              << "equal" << endl;    
    return true;
  } else {
    kdDebug() << "AddressBookSyncEntry::equals(): '" << entry->name() << "':"
              << "not equal" << endl;
    return false;
  }
}

AddressBookSyncee::AddressBookSyncee()
{
  mAddressBook = new KABC::AddressBook;

  mEntries.setAutoDelete(true);
}

AddressBookSyncee::~AddressBookSyncee()
{
  delete mAddressBook;
}

bool AddressBookSyncee::read()
{
  KRES::Manager<KABC::Resource> manager( "contact" );
  KABC::Resource *resource = manager.createResource( "file" );
  mAddressBook->addResource( resource );
  return mAddressBook->load();
}

bool AddressBookSyncee::write()
{
  KABC::Ticket *ticket = mAddressBook->requestSaveTicket();
  if ( !ticket ) return false;
  return mAddressBook->save( ticket );
}


AddressBookSyncEntry *AddressBookSyncee::firstEntry()
{
  mAddressBookIterator = mAddressBook->begin();
  return createEntry( *mAddressBookIterator );
}

AddressBookSyncEntry *AddressBookSyncee::nextEntry()
{
  ++mAddressBookIterator;
  return createEntry( *mAddressBookIterator );
}

#if 0
AddressBookSyncEntry *AddressBookSyncee::findEntry(const QString &id)
{
  Event *event = mAddressBook->getEvent(id);
  return createEntry(event);
}
#endif

void AddressBookSyncee::addEntry( KSyncEntry *entry )
{
  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>(entry);
  if (!abEntry) {
    kdDebug() << "AddressBookSyncee::addEntry(): SyncEntry has wrong type."
              << endl;
  } else {
    mAddressBook->insertAddressee( abEntry->addressee() );
  }
}

void AddressBookSyncee::removeEntry( KSyncEntry *entry )
{
  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>(entry);
  if ( !abEntry ) {
    kdDebug() << "AddressBookSyncee::removeEntry(): SyncEntry has wrong type."
              << endl;
  } else {
    mAddressBook->removeAddressee( abEntry->addressee() );
  }
}

AddressBookSyncEntry *AddressBookSyncee::createEntry( const KABC::Addressee &a )
{
  if ( !a.isEmpty() ) {
    AddressBookSyncEntry *entry = new AddressBookSyncEntry( a );
    mEntries.append( entry );
    return entry;
  } else {
    return 0;
  }  
}
