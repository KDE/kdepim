/* This file is part of KDE PIM
    Copyright (C) 1999 Don Sanders <dsanders@kde.org>

    License: GNU GPL
*/

#include "entry.h"
#include <qdict.h>
#include <qfile.h>
#include <qregexp.h>
#include <klocale.h>
#include <kabapi.h>

////////////////////////////
// ContactEntryList methods

ContactEntryList::ContactEntryList()
{
  addrBook = new KabAPI; // KabApi is a dialog;
  CHECK_PTR(addrBook);
  if(addrBook->init()!=AddressBook::NoError)
  { // this connects to the default address book and opens it:
    debug( "Error initializing the connection to your KAB address book." );
    addrBook = 0;
  } 
  else {
    debug ("KMKernel::init: KabApi initialized.");
  }

  KabKey key;
  AddressBook::Entry entry;
  ContactEntry *item;
  int num = addrBook->addressbook()->noOfEntries();

  for (int i = 0; i < num; ++i) {
    if (AddressBook::NoError != addrBook->addressbook()->getKey( i, key )) {
      return;
    }
    if (AddressBook::NoError != addrBook->addressbook()->getEntry( key, entry ))
    {
      return;
    }
    item = KabEntryToContactEntry( entry );
    ceDict.insert( key.getKey(), item );   
  }
}

ContactEntryList::~ContactEntryList()
{
  QStringList::Iterator it;
  KabKey key;
  for( it = removedKeys.begin(); it != removedKeys.end(); ++it ) {
    key.setKey( (*it).local8Bit() );
    addrBook->addressbook()->remove( key ); // check rc
  }
  if (addrBook->addressbook()->save("", true)!=AddressBook::NoError)
    debug( "Error occurred trying to update database" );
}

QString ContactEntryList::insert( ContactEntry *item )
{
  // need to get a write lock on the address book
  KabKey key;
  AddressBook::Entry empty;
  AddressBook::Entry entry = ContactEntryToKabEntry( item, empty );  
  if (AddressBook::NoError != addrBook->addressbook()->add( entry, key, true )) {
    debug( "Error occurred trying to insert entry" );
    // TODO show a message box here
  }
  if (addrBook->addressbook()->save("", true)!=AddressBook::NoError)
    debug( "Error occurred trying to update database" );

  ceDict.insert( key.getKey(), item );
  return key.getKey();
}

void ContactEntryList::unremove( const QString &key, ContactEntry *item )
{
  ceDict.insert( key, item );
  removedKeys.remove( key );
}

void ContactEntryList::remove( const QString &key )
{
  removedKeys.append( key );
  ceDict.remove( key );
}

ContactEntry* ContactEntryList::find( const QString &key )
{
  return ceDict.find( key );
}

void ContactEntryList::replace( const QString &key, ContactEntry *item )
{
  // need to get a write lock on the address book
  AddressBook::Entry old;
  KabKey kabKey;
  kabKey.setKey( key.local8Bit() );
  if (AddressBook::NoError != addrBook->addressbook()->getEntry( kabKey, old )) {
    debug( "Error occurred trying to update entry" );
    // TODO show a message box here
    return;
  }

  if (addrBook->addressbook()->save("", true)!=AddressBook::NoError)
    debug( "Error occurred trying to update database" );

  AddressBook::Entry entry = ContactEntryToKabEntry( item, old );
  addrBook->addressbook()->change( kabKey, entry ); // must check rc
  ceDict.replace( key, item );
}

ContactEntry* ContactEntryList::KabEntryToContactEntry( AddressBook::Entry entry )
{
  ContactEntry *ce = new ContactEntry();

  // Guess some values based on KAB fields (may be overidden)
  AddressBook::Entry::Address ad;
  if ((entry.noOfAddresses() > 0) &&
      (AddressBook::NoError == entry.getAddress(0, ad))) {
    ce->insert( "X-BusinessAddress",  
		new QString( ad.address + "\n" + 
			     ad.town + ", " + 
			     ad.state + " " + 
			     ad.zip + "\n" + 
			     ad.country + "\n" ));
    ce->insert( "ORG", new QString( ad.org ));
    ce->insert( "X-Department", new QString( ad.orgUnit ));  
  }
  ce->insert( "X-FileAs", new QString( entry.fn ));
  ce->insert( "ROLE", new QString( entry.title ));

  // Get all Kontact Fields
  QStringList::Iterator it;
  for( it = entry.custom.begin(); it != entry.custom.end(); ++it )
    if ((*it).find( "KMail:1." ) == 0) {
      QTextIStream iStream(&(*it));
      QString dummy = iStream.readLine();
      ce->load( iStream );
    }

  // Use KAB fields where applicable (takes precedence)
  //  ce->insert( "N", new QString( entry.fn ));
  QString n = entry.nameprefix + " " +
    entry.firstname + " " +
    entry.middlename + " " +
    entry.lastname;

  ce->replace( "N", new QString( n.simplifyWhiteSpace() ));
  ce->replace( "X-Title",  new QString( entry.nameprefix ));
  ce->replace( "X-FirstName",  new QString( entry.firstname ));
  ce->replace( "X-MiddleName",  new QString( entry.middlename ));
  ce->replace( "X-LastName",  new QString( entry.lastname ));
  if (entry.emails.count() > 0)
    ce->replace( "EMAIL", new QString( entry.emails[0] ));
  if (entry.emails.count() > 1)
    ce->replace( "X-E-mail2", new QString( entry.emails[1]) );
  if (entry.emails.count() > 2)
    ce->replace( "X-E-mail3", new QString( entry.emails[2]) );
  ce->replace( "X-Notes",  new QString( entry.comment ));
  ce->replace( "WEBPAGE",  new QString( entry.URLs[0] ));
  ce->replace( "X-Userfield1", new QString( entry.user1 ));
  ce->replace( "X-Userfield2", new QString( entry.user2 ));
  ce->replace( "X-Userfield3", new QString( entry.user3 ));
  ce->replace( "X-Userfield4", new QString( entry.user4 ));

  for( it = entry.telephone.begin(); it != entry.telephone.end(); ++it )
  {
    QString type = *it;
    ++it;
    if (it == entry.telephone.end()) // sanity check
      break;

    if (type == "0")
      ce->replace( "X-BusinessPhone", new QString( *it ));
    else if (type == "1")
      ce->replace( "X-MobilePhone",  new QString( *it ));
    else if (type == "2")
      ce->replace( "X-BusinessFax",  new QString( *it ));
    else if (type == "4")
      ce->replace( "X-OtherPhone",  new QString( *it ));
  }

  if ((entry.noOfAddresses() > 0) &&
      (AddressBook::NoError == entry.getAddress(0, ad))) {
    ce->replace( "X-BusinessAddressCity", new QString( ad.town )); 
    ce->replace( "X-BusinessAddressCountry", new QString( ad.country )); 
    ce->replace( "X-BusinessAddressState", new QString( ad.state )); 
    ce->replace( "X-BusinessAddressStreet", new QString( ad.address )); 
    ce->replace( "X-BusinessAddressPostalCode", new QString( ad.zip )); 
  }

  return ce;
}

AddressBook::Entry ContactEntryList::ContactEntryToKabEntry( ContactEntry *entry, AddressBook::Entry def )
{
  AddressBook::Entry kabentry = def;

  if (entry->find( "N" ))
    kabentry.fn = *(entry->find( "N" ));
  if (entry->find( "X-Title" ))
    kabentry.nameprefix = *(entry->find( "X-Title" ));
  if (entry->find( "X-FirstName" ))
    kabentry.firstname = *(entry->find( "X-FirstName" ));
  if (entry->find( "X-MiddleName" ))
    kabentry.middlename = *(entry->find( "X-MiddleName" ));
  if (entry->find( "X-LastName" ))
    kabentry.lastname = *(entry->find( "X-LastName" ));
  if (entry->find( "EMAIL" )) {
    if (kabentry.emails.count() < 1)
      kabentry.emails.append( "" );
    kabentry.emails[0] = *(entry->find( "EMAIL" ));
  }
  /*
  if (entry->find( "X-E-mail2" ))
    kabentry.emails[1] = *(entry->find( "X-E-mail2" ));
  if (entry->find( "X-E-mail3" ))
    kabentry.emails[2] = *(entry->find( "X-E-mail3" ));
  */
  if (entry->find( "X-Notes" ))
    kabentry.comment = *(entry->find( "X-Notes" ));
  if (entry->find( "WEBPAGE" )) {
    if (kabentry.URLs.count() < 1)
      kabentry.URLs.append( "" );
    kabentry.URLs[0] = *(entry->find( "WEBPAGE" ));
  }
  if (entry->find( "X-Userfield1" ))
    kabentry.user1 = *(entry->find( "X-Userfield1" ));
  if (entry->find( "X-Userfield2" ))
    kabentry.user2 = *(entry->find( "X-Userfield2" ));
  if (entry->find( "X-Userfield3" ))
    kabentry.user3 = *(entry->find( "X-Userfield3" ));
  if (entry->find( "X-Userfield4" ))
    kabentry.user4 = *(entry->find( "X-Userfield4" ));

  QStringList::Iterator it;
  bool phBusiness = false;
  bool phMobile = false;
  bool phFax = false;
  bool phOther = false;

  for( it = kabentry.telephone.begin(); it != kabentry.telephone.end(); ++it )
  {
    QString type = *it;
    ++it;
    if (it == kabentry.telephone.end()) // sanity check
      break;

    if ((type == "0") && (entry->find( "X-BusinessPhone" ))) {
      phBusiness = true;
      *it = *(entry->find( "X-BusinessPhone" ));
    }
    else if ((type == "1") && (entry->find( "X-MobilePhone" ))) {
      phMobile = true;
      *it = *(entry->find( "X-MobilePhone" ));
    }
    else if ((type == "2") && (entry->find( "X-BusinessFax" ))) {
      phFax = true;
      *it = *(entry->find( "X-BusinessFax" ));
    }
    else if ((type == "4") && (entry->find( "X-OtherPhone" ))) {
      phOther = true;
      *it = *(entry->find( "X-OtherPhone" ));
    }
  }
  if (!phBusiness  && (entry->find( "X-BusinessPhone" ))) {
    kabentry.telephone.append( "0" );
    kabentry.telephone.append( *(entry->find( "X-BusinessPhone" )) );
  }
  if (!phMobile && (entry->find( "X-MobilePhone" ))) {
    kabentry.telephone.append( "1" );
    kabentry.telephone.append( *(entry->find( "X-MobilePhone" )) );
  }
  if (!phFax && (entry->find( "X-BusinessFax" ))) {
    kabentry.telephone.append( "2" );
    kabentry.telephone.append( *(entry->find( "X-BusinessFax" )) );
  }
  if (!phOther && (entry->find( "X-OtherPhone" ))) {
    kabentry.telephone.append( "4" );
    kabentry.telephone.append( *(entry->find( "X-OtherPhone" )) );
  }

  QString result;
  QTextOStream oStream(&result);
  oStream << "KMail:1.0\n";
  entry->save( oStream );
  bool found = false;

  for( it = kabentry.custom.begin(); it != kabentry.custom.end(); ++it )
    if ((*it).find( "KMail:1." ) == 0) {
      *it = result;
      found = true;
      break;
    }
  if (!found)
    kabentry.custom.append( result );

  return kabentry;
}

QStringList ContactEntryList::keys()
{
  QStringList entryKeys;
  QDictIterator<ContactEntry> it(ceDict);
  while (it.current()) {
    entryKeys.append( it.currentKey() );
    ++it;
  }
  
  return entryKeys;
}


/*
ContactEntryList::ContactEntryList( const QString &filename )
{
  kkey = 0;
  setAutoDelete( true );
  load( filename );
}

QString ContactEntryList::key()
{
  ++kkey;
  return QString().setNum( kkey );
}

QString ContactEntryList::insert( ContactEntry *item )
{
  QString result = key();
  QDict<ContactEntry>::insert( result, item );
  return result;
}

void ContactEntryList::unremove( const QString &key, ContactEntry *item )
{
  QDict<ContactEntry>::insert( key, item );  
}

void ContactEntryList::save( const QString &filename )
{
  QFile f( filename );
  if ( !f.open( IO_WriteOnly ) )
    return;

  QTextStream t( &f );

  QDictIterator<ContactEntry> it(*this);
  while (it.current()) {
    it.current()->save( t );
    ++it;
  }

  f.close();
}

void ContactEntryList::load( const QString &filename )
{
  QFile f( filename );
  if ( !f.open( IO_ReadOnly ) )
    return;

  QTextStream t( &f );
  clear();

  while ( !t.eof() ) {
    QDict<ContactEntry>::insert( key(), new ContactEntry( t ));
    // connect up a signal to emit of this guy
  } 

  f.close();
}
*/

////////////////////////
// ContactEntry methods

ContactEntry::ContactEntry() 
{
  dict.setAutoDelete( true );
}

ContactEntry::ContactEntry( const ContactEntry &r )
  : QObject()
{
  QDictIterator<QString> it( r.dict );
  
  while ( it.current() ) {
    dict.replace( it.currentKey(), new QString( *it.current() ));
    ++it;
  }
}

ContactEntry& ContactEntry::operator=( const ContactEntry &r )
{
  if (this != &r) {
    dict.clear();
    QDictIterator<QString> it( r.dict );
    
    while ( it.current() ) {
      dict.replace( it.currentKey(), new QString( *it.current() ));
      ++it;
    }
  }
  return *this;
}

/*
ContactEntry::ContactEntry( const QString &filename )
{
  dict.setAutoDelete( true );
  load( filename );
}
*/

ContactEntry::ContactEntry( QTextStream &t )
{
  dict.setAutoDelete( true );
  load( t );
}

QStringList ContactEntry::custom() const
{
  QStringList result;
  QDictIterator<QString> it( dict );

  while ( it.current() ) {
    if (it.currentKey().find( "X-CUSTOM-", 0 ) == 0)
      result << it.currentKey();
    ++it;
  }
  return result;
}

/*
void ContactEntry::save( const QString &filename )
{
  QFile f( filename );
  if ( !f.open( IO_WriteOnly ) )
    return;

  QTextStream t( &f );
  QDictIterator<QString> it( dict );

  while ( it.current() ) {
    if (it.currentKey().find( ".", 0 ) != 0) {
      t << it.currentKey() << "\n";
      t << *it.current() << "\n[EOR]\n";
    }
    ++it;
  }
  t << "[EOS]\n";
  f.close();
}
*/

void ContactEntry::save( QTextStream &t )
{
  QDictIterator<QString> it( dict );
  QRegExp reg("\n");

  while ( it.current() ) {
    if ((it.currentKey().find( ".", 0 ) != 0) &&
	(!(*it.current()).isEmpty())) {
      t << " " << it.currentKey() << "\n";
      QString tmp = *it.current();
      tmp.replace( reg, "\n " );
      t << " " << tmp << "\n[EOR]\n";
    }
    ++it;
  }
  t << "[EOS]\n";
}

/*
void ContactEntry::load( const QString &filename )
{
  dict.clear();

  QFile f( filename );
  if ( !f.open( IO_ReadOnly ) )
    return;

  QTextStream t( &f );

  while ( !t.eof() ) {
    QString name = t.readLine();
    if (name == "[EOS]")
      break;
    QString tmp = t.readLine();
    QString value = "";
    while (tmp != QString( "\n[EOR]" )) {
      value += tmp;
      tmp = "\n" + t.readLine();
    }
    if ((name != "") && (value != ""))
      dict.replace( name, new QString( value ));
  }
  f.close();
  emit changed();
}
*/

void ContactEntry::load( QTextStream &t )
{
  while (!t.eof()) {
    QString name = t.readLine();
    if (name == "[EOS]")
      break;
    name = name.mid(1);
    QString tmp = t.readLine();
    QString value = "";
    while (tmp != QString( "[EOR]" )) {
      if (!value.isEmpty())
	value += "\n";
      value += tmp.mid(1);
      tmp = t.readLine();
    }
    if ((name != "") && (value != ""))
      dict.replace( name, new QString( value ));
  }
  emit changed();
}

void ContactEntry::insert( const QString key, const QString *item )
{
  if (item && (*item == ""))
    return;
  dict.insert( key, item );
  emit changed();
}

void ContactEntry::replace( const QString key, const QString *item )
{
  QString *current = dict.find( key );
  if (item) {
    if (current) {
      if (*item != *current) {
	if (*item == "")
	  dict.remove( key ); // temporary?
	else
	  dict.replace( key, item );
	emit changed();
      }
    }
    else { // (item && !current)
      dict.replace( key, item );
      emit changed();
    }
  }
  else
    debug( QString( "Error:" ) + 
	   " ContactEntry::replace( const QString, const QString* ) " +
	   "passed null item" );
  /*
  if (item && (*item == ""))
    dict.remove( key );
  else
    dict.replace( key, item );
  emit changed();
  */
}

bool ContactEntry::remove( const QString key )
{
  if (dict.remove( key ))
    emit changed();
}

void ContactEntry::touch()
{
  emit changed();
}

const QString *ContactEntry::find ( const QString & key ) const
{
  return dict.find( key );
}

const QString *ContactEntry::operator[] ( const QString & key ) const
{
  return dict[key];
}

void ContactEntry::clear ()
{
  dict.clear();
  emit changed();
}
