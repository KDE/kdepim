/* This file is part of KDE PIM
    Copyright (C) 1999 Don Sanders <sanders@kde.org>

    kabc port
    Copyright (C) 2001,2002 Cornelius Schumacher <schumacher.kde.org>

    License: BSD
*/

#include "contactentrylist.h"
#include <qdict.h>
#include <qfile.h>
#include <qregexp.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kprocess.h>

#include <kabc/stdaddressbook.h>

ContactEntryList::ContactEntryList()
{
  mAddressBook = KABC::StdAddressBook::self();
  ceDict.setAutoDelete( true );
  refresh();
}

ContactEntryList::~ContactEntryList()
{
  commit();
}

void ContactEntryList::refresh()
{
  mAddressBook->load();

  ceDict.clear();

  KABC::AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    ContactEntry *item = KabEntryToContactEntry( *it );
    ceDict.insert( (*it).uid(), item );
  }
}

void ContactEntryList::commit()
{
  if ( !KABC::StdAddressBook::save() ) {
    KMessageBox::sorry( 0, i18n("Couldn't save addressbook") );
  }
}

QString ContactEntryList::insert( ContactEntry *item )
{
  KABC::Addressee a = ContactEntryToKabEntry( item );
  item->setAddressee( a );

  mAddressBook->insertAddressee( a );
  if ( !KABC::StdAddressBook::save() ) {
    KMessageBox::sorry( 0, i18n("Couldn't save addressbook") );
  }

  ceDict.insert( a.uid(), item );
  return a.uid();
}

void ContactEntryList::unremove( const QString &key, ContactEntry *item )
{
  KABC::Addressee a = ContactEntryToKabEntry( item );
  a.setUid( key );
  mAddressBook->insertAddressee( a );
  ceDict.insert( key, item );
}

void ContactEntryList::remove( const QString &key )
{
  mAddressBook->removeAddressee( mAddressBook->findByUid( key ) );
  ceDict.remove( key );
}

ContactEntry* ContactEntryList::find( const QString &key )
{
  return ceDict.find( key );
}

void ContactEntryList::replace( const QString &key, ContactEntry *item )
{
  KABC::Addressee a = ContactEntryToKabEntry( item );
  a.setUid( key );
  item->setAddressee( a );
  
  mAddressBook->insertAddressee( a );
  ceDict.replace( key, item );

  if ( !KABC::StdAddressBook::save() ) {
    KMessageBox::sorry( 0, i18n("Couldn't save addressbook") );
  }
}

ContactEntry* ContactEntryList::KabEntryToContactEntry( const KABC::Addressee &entry )
{
  ContactEntry *ce = new ContactEntry();
  ce->setLoading(true);

  ce->setAddressee( entry );

  ce->insert( "ORG", new QString( entry.organization() ));
  ce->insert( "X-FileAs", new QString( entry.formattedName() ));
  ce->insert( "ROLE", new QString( entry.role() ));
  ce->insert( "NICKNAME", new QString( entry.nickName() ));

  ce->insert( "X-Profession", new QString( entry.custom( "KADDRESSBOOK", "Profession" ) ) );
  ce->insert( "X-Department", new QString( entry.custom( "KADDRESSBOOK", "Department" ) ) );
  ce->insert( "X-ManagersName", new QString( entry.custom( "KADDRESSBOOK", "ManagersName" ) ) );
  ce->insert( "X-AssistantsName", new QString( entry.custom( "KADDRESSBOOK", "AssistantsName" ) ) );
  ce->insert( "X-Office", new QString( entry.custom( "KADDRESSBOOK", "Office" ) ) );
  ce->insert( "X-SpousesName", new QString( entry.custom( "KADDRESSBOOK", "SpousesName" ) ) );
  ce->insert( "X-Userfield1", new QString( entry.custom( "KADDRESSBOOK", "Userfield1" ) ) );
  ce->insert( "X-Userfield2", new QString( entry.custom( "KADDRESSBOOK", "Userfield2" ) ) );
  ce->insert( "X-Userfield3", new QString( entry.custom( "KADDRESSBOOK", "Userfield3" ) ) );
  ce->insert( "X-Userfield4", new QString( entry.custom( "KADDRESSBOOK", "Userfield4" ) ) );

  QString n = entry.prefix() + " " +
    entry.givenName() + " " +
    entry.additionalName() + " " +
    entry.familyName() + " " +
    entry.suffix();

  ce->insert( "N",new QString (entry.realName()) );
  ce->insert( "X-Title",  new QString( entry.prefix() ));
  ce->insert( "X-FirstName",  new QString( entry.givenName() ));
  ce->insert( "X-MiddleName",  new QString( entry.additionalName() ));
  ce->insert( "X-LastName",  new QString( entry.familyName() ));
  ce->insert( "X-Suffix",  new QString( entry.suffix() ));

  QStringList emails = entry.emails();
  if (emails.count() > 0)
    ce->insert( "EMAIL", new QString( emails[0] ));
  if (emails.count() > 1)
    ce->insert( "X-E-mail2", new QString( emails[1]) );
  if (emails.count() > 2)
    ce->insert( "X-E-mail3", new QString( emails[2]) );
  ce->insert( "X-Notes",  new QString( entry.note() ));
  ce->insert( "WEBPAGE",  new QString( entry.url().url() ));

  readPhoneNumber( entry, KABC::PhoneNumber::Pref, ce, "X-PrimaryPhone" );
  readPhoneNumber( entry, KABC::PhoneNumber::Work, ce, "X-BusinessPhone" );
  readPhoneNumber( entry, KABC::PhoneNumber::Home, ce, "X-HomePhone" );
  readPhoneNumber( entry, KABC::PhoneNumber::Cell, ce, "X-MobilePhone" );
  readPhoneNumber( entry, KABC::PhoneNumber::Fax | KABC::PhoneNumber::Work,
                   ce, "X-BusinessFax" );
  readPhoneNumber( entry, KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home,
                   ce, "X-HomeFax" );
  readPhoneNumber( entry, KABC::PhoneNumber::Fax, ce, "X-OtherFax" );
  readPhoneNumber( entry, KABC::PhoneNumber::Car, ce, "X-CarPhone" );
  readPhoneNumber( entry, KABC::PhoneNumber::Pager, ce, "X-Pager" );
  readPhoneNumber( entry, KABC::PhoneNumber::Isdn, ce, "X-ISDN" );
  readPhoneNumber( entry, 0 , ce, "X-OtherPhone" );

  readAddress( entry.address( KABC::Address::Work ), ce, "Business" );
  readAddress( entry.address( KABC::Address::Home ), ce, "Home" );
  readAddress( entry.address( 0 ), ce, "Other" );

  if ( entry.birthday().isValid() ) {
    ce->insert( "BDAY", new QString( KGlobal::locale()->formatDate( entry.birthday().date() ) ) );
  }

  QStringList customs = entry.customs();
  QStringList::ConstIterator it;
  for( it = customs.begin(); it != customs.end(); ++it ) {
//    kdDebug() << "--- '" << *it << "'" << endl;
    if ( (*it).startsWith( "KADDRESSBOOK-" ) ) {
      QString str = (*it).mid( 13 );
      int pos = str.find( ":" );
      QString fieldName = str.left( pos );
      QString fieldValue = str.mid( pos + 1 );
//      kdDebug() << "  " << fieldName << ": " << fieldValue << endl;
      ce->insert( fieldName, new QString( fieldValue ) );
    }
  }

  //ce->setModified(false);  // in future, delete this; here to fix old bug...
  ce->setLoading(false);
  return ce;
}

void ContactEntryList::readAddress( KABC::Address ad, ContactEntry *ce,
                                    const QString &type )
{
  if ( ad.isEmpty() ) return;

  ce->insert( "X-" + type + "Address",
              new QString( ad.street() + "\n" +
                           ad.locality() + ", " +
                           ad.region() + " " +
                           ad.postalCode() + "\n" +
                           ad.country() + "\n" ));
  ce->replace( "X-" + type + "AddressCity", new QString( ad.locality() ));
  ce->replace( "X-" + type + "AddressCountry", new QString( ad.country() ));
  ce->replace( "X-" + type + "AddressState", new QString( ad.region() ));
  ce->replace( "X-" + type + "AddressStreet", new QString( ad.street() ));
  ce->replace( "X-" + type + "AddressPostalCode", new QString( ad.postalCode() ));
  ce->replace( "X-" + type + "AddressPOBox", new QString( ad.postOfficeBox() ));
}

void ContactEntryList::readPhoneNumber( KABC::Addressee a, int kabcType,
                                        ContactEntry *ce, const QString &type )
{
  QString value = a.phoneNumber( kabcType ).number();
  if ( !value.isEmpty() ) {
    ce->insert( type, new QString( value ) );
  }
}

KABC::Addressee ContactEntryList::ContactEntryToKabEntry( ContactEntry *entry )
{
#if 0
  kdDebug() << "--- BEGIN:ENTRY ---" << endl;
  entry->debug();
  kdDebug() << "--- END:ENTRY ---" << endl;
#endif

  KABC::Addressee a;

  QString value;

  a.setFormattedName( entry->findRef( "N" ) );
  a.setPrefix( entry->findRef( "X-Title" ) );
  a.setGivenName( entry->findRef( "X-FirstName" ) );
  a.setAdditionalName( entry->findRef( "X-MiddleName" ) );
  a.setFamilyName( entry->findRef( "X-LastName" ) );
  a.setSuffix( entry->findRef( "X-Suffix" ) );

  a.setNote( entry->findRef( "X-Notes" ) );
  a.setRole( entry->findRef( "ROLE" ) );
  a.setOrganization( entry->findRef( "ORG" ) );
  a.setNickName( entry->findRef( "NICKNAME" ) );

  value = entry->findRef( "WEBPAGE" );
  if ( !value.isNull() ) {
    a.setUrl( KURL( value ) );
  }

  a.insertEmail( entry->findRef( "EMAIL" ), true );
  a.insertEmail( entry->findRef( "X-E-mail2" ) );
  a.insertEmail( entry->findRef( "X-E-mail3" ) );

  writePhoneNumber( a, entry, "X-PrimaryPhone", KABC::PhoneNumber::Pref );
  writePhoneNumber( a, entry, "X-HomePhone", KABC::PhoneNumber::Home );
  writePhoneNumber( a, entry, "X-BusinessPhone", KABC::PhoneNumber::Work );
  writePhoneNumber( a, entry, "X-MobilePhone", KABC::PhoneNumber::Cell );
  writePhoneNumber( a, entry, "X-BusinessFax", KABC::PhoneNumber::Work |
                                               KABC::PhoneNumber::Fax );
  writePhoneNumber( a, entry, "X-HomeFax", KABC::PhoneNumber::Home |
                                           KABC::PhoneNumber::Fax );
  writePhoneNumber( a, entry, "X-OtherFax", KABC::PhoneNumber::Fax );
  writePhoneNumber( a, entry, "X-CarPhone", KABC::PhoneNumber::Car );
  writePhoneNumber( a, entry, "X-Pager", KABC::PhoneNumber::Pager );
  writePhoneNumber( a, entry, "X-ISDN", KABC::PhoneNumber::Isdn );
  writePhoneNumber( a, entry, "X-OtherPhone", 0 );

  writeCustom( a, entry, "X-HomePhone2" );
  writeCustom( a, entry, "X-BusinessPhone2" );
  writeCustom( a, entry, "X-TtyTddPhone" );
  writeCustom( a, entry, "X-RadioPhone" );
  writeCustom( a, entry, "X-Telex" );
  writeCustom( a, entry, "X-AssistantsPhone" );
  writeCustom( a, entry, "X-Callback" );
  writeCustom( a, entry, "X-CompanyMainPhone" );

  writeAddress( a, entry, "Business", KABC::Address::Work );
  writeAddress( a, entry, "Home", KABC::Address::Home );
  writeAddress( a, entry, "Other", 0 );

  QString birthdayString = entry->findRef( "BDAY" );
  bool ok;
  QDate birthday = KGlobal::locale()->readDate( birthdayString, &ok );
  if ( ok ) {
    a.setBirthday( birthday );
  } else {
    kdDebug() << "birthday string not valid: '" << birthdayString << "'" << endl;
  }

  writeCustom( a, entry, "X-Profession" );
  writeCustom( a, entry, "X-Department" );
  writeCustom( a, entry, "X-ManagersName" );
  writeCustom( a, entry, "X-AssistantsName" );
  writeCustom( a, entry, "X-Office" );
  writeCustom( a, entry, "X-SpousesName" );
  writeCustom( a, entry, "X-Userfield1" );
  writeCustom( a, entry, "X-Userfield2" );
  writeCustom( a, entry, "X-Userfield3" );
  writeCustom( a, entry, "X-Userfield4" );

  writeCustom( a, entry, "X-Account" );
  writeCustom( a, entry, "X-Anniversary" );
  writeCustom( a, entry, "X-Attachment" );
  writeCustom( a, entry, "X-BillingInformation" );
  writeCustom( a, entry, "X-BusinessHomePage" );
  writeCustom( a, entry, "X-Categories" );
  writeCustom( a, entry, "X-Children" );
  writeCustom( a, entry, "X-City" );
  writeCustom( a, entry, "X-ComputerNetworkName" );
  writeCustom( a, entry, "X-Country" );
  writeCustom( a, entry, "X-Created" );
  writeCustom( a, entry, "X-CustomerID" );
  writeCustom( a, entry, "X-Folder" );
  writeCustom( a, entry, "X-FTPSite" );
  writeCustom( a, entry, "X-Gender" );
  writeCustom( a, entry, "X-GovernmentIDNumber" );
  writeCustom( a, entry, "X-Hobbies" );
  writeCustom( a, entry, "X-Icon" );
  writeCustom( a, entry, "X-InFolder" );
  writeCustom( a, entry, "X-Initials" );
  writeCustom( a, entry, "X-Journal" );
  writeCustom( a, entry, "X-Language" );
  writeCustom( a, entry, "X-Location" );
  writeCustom( a, entry, "X-MailingAddress" );
  writeCustom( a, entry, "X-MessageClass" );
  writeCustom( a, entry, "X-Mileage" );
  writeCustom( a, entry, "X-Modified" );
  writeCustom( a, entry, "X-OfficeLocation" );
  writeCustom( a, entry, "X-OrganizationalIDNumber" );
  writeCustom( a, entry, "X-PersonalHomePage" );
  writeCustom( a, entry, "X-POBox" );
  writeCustom( a, entry, "X-Profession" );
  writeCustom( a, entry, "X-Read" );
  writeCustom( a, entry, "X-ReferredBy" );
  writeCustom( a, entry, "X-Sensitivity" );
  writeCustom( a, entry, "X-Size" );
  writeCustom( a, entry, "X-State" );
  writeCustom( a, entry, "X-StreetAddress" );
  writeCustom( a, entry, "X-Subject" );
  writeCustom( a, entry, "X-ZIPPostalCode" );

  QStringList customs = entry->custom();
  QStringList::ConstIterator it;
  for( it = customs.begin(); it != customs.end(); ++it ) {
    writeCustom( a, entry, *it );
  }

  return a;
}

void ContactEntryList::writeCustom( KABC::Addressee &a, ContactEntry *entry,
                                    const QString &type )
{
  QString value = entry->findRef( type );
  if ( !value.isEmpty() ) {
    a.insertCustom( "KADDRESSBOOK", type, entry->findRef( type ) );
  }
}


void ContactEntryList::writeAddress( KABC::Addressee &a, ContactEntry *entry,
                                     const QString &type, int kabcType )
{
  KABC::Address address;
  address.setStreet( entry->findRef( "X-" + type + "AddressStreet" ) );
  address.setRegion( entry->findRef( "X-" + type + "AddressState" ) );
  address.setPostalCode( entry->findRef( "X-" + type + "AddressPostalCode" ) );
  address.setLocality( entry->findRef( "X-" + type + "AddressCity" ) );
  address.setCountry( entry->findRef( "X-" + type + "AddressCountry" ) );
  address.setPostOfficeBox( entry->findRef( "X-" + type + "AddressPOBox" ) );
  address.setType( kabcType );
  if ( !address.isEmpty() ) {
    a.insertAddress( address );
  }
}

void ContactEntryList::writePhoneNumber( KABC::Addressee &a, ContactEntry *entry,
                                         const QString &type, int kabcType )
{
  QString value = entry->findRef( type );
  if ( !value.isNull() ) {
    a.insertPhoneNumber( KABC::PhoneNumber( value, kabcType ) );
  }
}

QStringList ContactEntryList::keys() const
{
  QStringList entryKeys;
  QDictIterator<ContactEntry> it(ceDict);
  while (it.current()) {
    entryKeys.append( it.currentKey() );
    ++it;
  }

  return entryKeys;
}

QDict<ContactEntry> ContactEntryList::getDict() const
{
    return ceDict;
}

void ContactEntryList::emptyTrash()
{
    for (QDictIterator<ContactEntry> it(ceDict);it.current();++it)
    {
	if (it.current()->inTrash())
	    remove(it.currentKey());
    }
}
