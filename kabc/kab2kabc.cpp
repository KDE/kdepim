#include <kaboutdata.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kabapi.h>

#include "addressbook.h"
#include "stdaddressbook.h"

using namespace KABC;

int main(int argc,char **argv)
{
  KAboutData aboutData("kab2kabc",I18N_NOOP("Kab to Kabc Converter"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;

  kdDebug() << "Converting old-style kab addressbook to "
               "new-style kabc addressbook." << endl;

  KabAPI kab(0);
  if (kab.init() != ::AddressBook::NoError) {
    kdDebug() << "Error initing kab" << endl;
    exit(1);
  }

  KABC::AddressBook *kabcBook = StdAddressBook::self();
  kabcBook->clear();

  KabKey key;
  ::AddressBook::Entry entry;

  int num = kab.addressbook()->noOfEntries();
  
  kdDebug() << "kab Addressbook has " << num << " entries." << endl;
  
  for (int i = 0; i < num; ++i) {
    if (::AddressBook::NoError != kab.addressbook()->getKey(i,key)) {
      kdDebug() << "Error getting key for index " << i << " from kab." << endl;
      continue;
    }
    if (::AddressBook::NoError != kab.addressbook()->getEntry(key,entry))
    {
      kdDebug() << "Error getting entry for index " << i << " from kab." << endl;
      continue;
    }

    Addressee a;

    // TODO: Check for existing kabc id in kab addressbook
    // TODO: Add kabc id to kab addressbook
    
    a.setTitle( entry.title );
    a.setFormattedName( entry.fn );
    a.setPrefix( entry.nameprefix );
    a.setGivenName( entry.firstname );
    a.setAdditionalName( entry.middlename );
    a.setFamilyName( entry.lastname );
    a.setBirthday( entry.birthday );

    QStringList::ConstIterator emailIt;
    for( emailIt = entry.emails.begin(); emailIt != entry.emails.end(); ++emailIt ) {
      a.insertEmail( *emailIt );
    }

    kdDebug() << "Phone: " << entry.telephone.join(",");
    QStringList::ConstIterator phoneIt;
    for( phoneIt = entry.telephone.begin(); phoneIt != entry.telephone.end(); ++phoneIt ) {
      int kabType = (*phoneIt++).toInt();
      QString number = *phoneIt;
      int type = 0;
      if ( kabType == ::AddressBook::Fixed ) type = PhoneNumber::Voice;
      else if ( kabType == ::AddressBook::Mobile ) type = PhoneNumber::Cell | PhoneNumber::Voice;
      else if ( kabType == ::AddressBook::Fax ) type = PhoneNumber::Fax;
      else if ( kabType == ::AddressBook::Modem ) type = PhoneNumber::Modem;
      a.insertPhoneNumber( PhoneNumber( number, type ) );
    }

    if ( entry.URLs.count() > 0 ) {
      a.setUrl( entry.URLs.first() );
      if ( entry.URLs.count() > 1 ) {
        kdWarning() << "More than one URL. Ignoring all but the first." << endl;
      }
    }

    int noAdr = entry.noOfAddresses();
    for( int j = 0; j < noAdr; ++j ) {
      ::AddressBook::Entry::Address kabAddress;
      entry.getAddress( j, kabAddress );
      
      Address adr;
      
      adr.setStreet( kabAddress.address );
      adr.setPostalCode( kabAddress.zip );
      adr.setLocality( kabAddress.town );
      adr.setCountry( kabAddress.country );
      adr.setRegion( kabAddress.state );
    
      // headline
      // position
      // org
      // orgUnit
      // orgSubUnit
      // deliveryLabel
      
      a.insertAddress( adr );
    }

    QString note = entry.comment;
    
    if ( !entry.user1.isEmpty() ) note += "\nUser1: " + entry.user1;
    if ( !entry.user2.isEmpty() ) note += "\nUser2: " + entry.user2;
    if ( !entry.user3.isEmpty() ) note += "\nUser3: " + entry.user3;
    if ( !entry.user4.isEmpty() ) note += "\nUser4: " + entry.user4;
    
    if ( !entry.keywords.count() == 0 ) note += "\nKeywords: " + entry.keywords.join( ", " );
    
    QStringList::ConstIterator talkIt;
    for( talkIt = entry.talk.begin(); talkIt != entry.talk.end(); ++talkIt ) {
      note += "\nTalk: " + (*talkIt);
    }
    
    a.setNote( note );

    a.setPrefix( entry.rank + a.prefix() );  // Add rank to prefix
    
    a.setCategories( entry.categories );

    // TODO: QStringList entry.custom;

    kdDebug() << "Addressee: " << a.familyName() << endl;

    kabcBook->insertAddressee( a );
  }

  StdAddressBook::save();
  
  kdDebug() << "Saved kabc addressbook to '" << kabcBook->fileName() << "'" << endl;
}
