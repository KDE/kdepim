#include <kdebug.h>
#include <kabc/resourcefile.h>

#include "addressbooksyncee.h"

using namespace KSync;

AddressBookSyncEntry::AddressBookSyncEntry( const KABC::Addressee &a ) :
    SyncEntry()
{
    mAddressee = a;
}
AddressBookSyncEntry::AddressBookSyncEntry( const AddressBookSyncEntry& entry )
: SyncEntry( entry ) {
    mAddressee = entry.mAddressee;
}
QString AddressBookSyncEntry::name()
{
  return mAddressee.realName();
}

QString AddressBookSyncEntry::id()
{
  return mAddressee.uid();
}
SyncEntry* AddressBookSyncEntry::clone() {
    return new AddressBookSyncEntry( *this );
}
QString AddressBookSyncEntry::timestamp()
{
  return QString::null;
}
QString AddressBookSyncEntry::type() const{
    return QString::fromLatin1("AddressBookSyncEntry");
}
bool AddressBookSyncEntry::equals( SyncEntry *entry )
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
//  mAddressBook = new KABC::AddressBook;

  mEntries.setAutoDelete(true);
}

AddressBookSyncee::~AddressBookSyncee()
{
//  delete mAddressBook;
}
/*
 * FIX ME. Read Addressbook and CreateEntries
 */
bool AddressBookSyncee::read()
{
//  ( void * ) new KABC::ResourceFile( mAddressBook, filename() );
//  return mAddressBook->load();
    return true;
}
/**
 * FIX ME. Create Addressbook insert Addressees and save it
 */
bool AddressBookSyncee::write()
{
//  KABC::Ticket *ticket = mAddressBook->requestSaveTicket();
//  if ( !ticket ) return false;
//  return mAddressBook->save( ticket );
    return true;
}


AddressBookSyncEntry *AddressBookSyncee::firstEntry()
{
  return mEntries.first();
}

AddressBookSyncEntry *AddressBookSyncee::nextEntry()
{
  return mEntries.next();
}

#if 0 // fix me later - zecke
AddressBookSyncEntry *AddressBookSyncee::findEntry(const QString &id)
{
  Event *event = mEntries.find(id);
  return createEntry(event);
}
#endif

void AddressBookSyncee::addEntry( SyncEntry *entry )
{
  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>(entry);
  if (!abEntry) {
    kdDebug() << "AddressBookSyncee::addEntry(): SyncEntry has wrong type."
              << endl;
  } else {
      abEntry->setSyncee( this ); // set the parent
      mEntries.append( abEntry);
  }
}

void AddressBookSyncee::removeEntry( SyncEntry *entry )
{
  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>(entry);
  if ( !abEntry ) {
    kdDebug() << "AddressBookSyncee::removeEntry(): SyncEntry has wrong type."
              << endl;
  } else {
      mEntries.remove( abEntry );
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
/**
 * clone it now - could be inside the Syncee but then we would have to cast
 * -zecke
 *
 */
Syncee* AddressBookSyncee::clone() {
    AddressBookSyncEntry* entry;
    SyncEntry* cloneE;
    AddressBookSyncee* clone = new AddressBookSyncee();

    for ( entry = mEntries.first(); entry != 0; entry = mEntries.next() ) {
        cloneE = entry->clone();
        clone->addEntry( cloneE ); // mSyncee gets updatet
    }
    return clone;
}
SyncEntry::PtrList AddressBookSyncee::added() {
    return find( SyncEntry::Added );
}
SyncEntry::PtrList AddressBookSyncee::modified() {
    return find( SyncEntry::Modified );
}
SyncEntry::PtrList AddressBookSyncee::removed() {
    return find( SyncEntry::Removed );
}
SyncEntry::PtrList AddressBookSyncee::find( int state ) {
    QPtrList<SyncEntry> found;
    AddressBookSyncEntry* entry;
    for ( entry = mEntries.first(); entry != 0; entry = mEntries.next() ) {
        if ( entry->state() == state )
            found.append( entry );
    }

    return found;
}
QString AddressBookSyncee::type() const {
    return QString::fromLatin1("AddressBookSyncee");
}
