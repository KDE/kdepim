#include <qfile.h>

#include <ksimpleconfig.h>
#include <kdebug.h>

#include "addressbook.h"

#include "simpleformat.h"

using namespace KABC;

bool SimpleFormat::load( AddressBook *addressBook, const QString &fileName )
{
  kdDebug() << "SimpleFormat::load(): " << fileName << endl;

  KSimpleConfig cfg( fileName );
  
  QStringList uids = cfg.groupList();
  QStringList::ConstIterator it;
  for( it = uids.begin(); it != uids.end(); ++it ) {
    if ( (*it) == "<default>" ) continue;
    cfg.setGroup( *it );
    Addressee a;
    a.setUid( *it );
    a.setName( cfg.readEntry( "name" ) );
    a.setFormattedName( cfg.readEntry( "formattedName" ) );
    a.setEmail( cfg.readEntry( "email" ) );
    QStringList phoneNumbers = cfg.readListEntry( "phonenumbers" );
    QStringList::ConstIterator it2;
    for( it2 = phoneNumbers.begin(); it2 != phoneNumbers.end(); ++it2 ) {
      PhoneNumber n;
      n.setNumber( cfg.readEntry( "phonenumber" + (*it2) ) );
      n.setType( PhoneNumber::Type((*it2).toInt()));
      a.setPhoneNumber( n );
    }
    addressBook->setAddressee( a );
  }

  return true;
}

bool SimpleFormat::save( AddressBook *addressBook, const QString &fileName )
{
  kdDebug() << "SimpleFormat::save(): " << fileName << endl;

  Addressee::List addressees = addressBook->addressees();

  QFile::remove( fileName );

  KSimpleConfig cfg( fileName );

  Addressee::List::ConstIterator it;
  for ( it = addressees.begin(); it != addressees.end(); ++it ) {
    cfg.setGroup( (*it).uid() );
    cfg.writeEntry( "name", (*it).name() );
    cfg.writeEntry( "formattedName", (*it).formattedName() );
    cfg.writeEntry( "email", (*it).email() );

    QStringList phoneNumberList;    
    PhoneNumber::List phoneNumbers = (*it).phoneNumbers();
    PhoneNumber::List::ConstIterator it2;
    for( it2 = phoneNumbers.begin(); it2 != phoneNumbers.end(); ++it2 ) {
      cfg.writeEntry( "phonenumber" + QString::number( int((*it2).type()) ),
                      (*it2).number() );
      phoneNumberList.append( QString::number( int((*it2).type()) ) );
    }
    cfg.writeEntry( "phonenumbers", phoneNumberList );
  }

  return true;
}
