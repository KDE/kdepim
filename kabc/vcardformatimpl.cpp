#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>

#include <VCard.h>

#include "addressbook.h"

#include "vcardformatimpl.h"

using namespace KABC;
using namespace VCARD;

bool VCardFormatImpl::load( AddressBook *addressBook, const QString &fileName )
{
  QCString data;

  QFile f( fileName );
  if ( f.open(IO_ReadOnly) ) {
    QTextStream t( &f );
    data = t.read().latin1();
    f.close();
  } else {
    return false;
  }
  
  addressBook->clear();

  VCardEntity e( data );
  
  VCardListIterator it( e.cardList() );

  for (; it.current(); ++it) {
    VCard v(*it.current());
    QList<ContentLine> contentLines = v.contentLineList();
    ContentLine *cl;

    Addressee a;

    for( cl = contentLines.first(); cl; cl = contentLines.next() ) {
      QString n = cl->name();
      if ( n.startsWith( "X-" ) ) {
        n = n.mid( 2 );
        int posDash = n.find( "-" );
        kdDebug() << "---n: " << n << " posDash: " << posDash << endl;        
        a.insertCustom( QString::fromUtf8( n.left( posDash ) ),
                        QString::fromUtf8( n.mid( posDash + 1 ) ),
                        QString::fromUtf8( cl->value()->asString() ) );
        continue;
      }
    
      EntityType type = cl->entityType();
      switch( type ) {

        case EntityUID:
          a.setUid( readTextValue( cl ) );
          break;

        case EntityEmail:
          a.insertEmail( readTextValue( cl ) );
          break;

        case EntityName:
          a.setName( readTextValue( cl ) );
          break;

        case EntityFullName:
          a.setFormattedName( readTextValue( cl ) );
          break;

        case EntityURL:
          a.setUrl( KURL( readTextValue( cl ) ) );
          break;

        case EntityNickname:
          a.setNickName( readTextValue( cl ) );
          break;

        case EntityLabel:
//          a.setLabel( readTextValue( cl ) );
          break;

        case EntityMailer:
          a.setMailer( readTextValue( cl ) );
          break;

        case EntityTitle:
          a.setTitle( readTextValue( cl ) );
          break;

        case EntityRole:
          a.setRole( readTextValue( cl ) );
          break;

        case EntityOrganisation:
          a.setOrganization( readTextValue( cl ) );
          break;

        case EntityNote:
          a.setNote( readTextValue( cl ) );
          break;

        case EntityProductID:
          a.setProductId( readTextValue( cl ) );
          break;

        case EntitySortString:
          a.setSortString( readTextValue( cl ) );
          break;

        case EntityN:
          readNValue( cl, a );
          break;

        case EntityAddress:
          a.insertAddress( readAddressValue( cl ) );
          break;

        case EntityTelephone:
          a.insertPhoneNumber( readTelephoneValue( cl ) );
          break;

        case EntityCategories:
          a.setCategories( QStringList::split( ",", readTextValue( cl ) ) );
          break;
          
        default:
          kdDebug() << "VCardFormat::load(): Unsupported entity: "
                    << int( type ) << endl;
          break;
      }
    }
  
    addressBook->insertAddressee( a );
  }
  
  return true;
}

bool VCardFormatImpl::save( AddressBook *addressBook, const QString &fileName )
{
  kdDebug() << "VCardFormat::save(): " << fileName << endl;

  VCardEntity vcards;
  VCardList vcardlist;
  vcardlist.setAutoDelete( true );
  ContentLine cl;
  QString value;

  AddressBook::Iterator it;
  for ( it = addressBook->begin(); it != addressBook->end(); ++it ) {
    VCard *v = new VCard;

    addTextValue( v, EntityName, (*it).name() );
    addTextValue( v, EntityUID, (*it).uid() );
    addTextValue( v, EntityFullName, (*it).formattedName() );
    
    QStringList emails = (*it).emails();
    QStringList::ConstIterator it4;
    for( it4 = emails.begin(); it4 != emails.end(); ++it4 ) {
      addTextValue( v, EntityEmail, *it4 );
    }

    QStringList customs = (*it).customs();
    QStringList::ConstIterator it5;
    for( it5 = customs.begin(); it5 != customs.end(); ++it5 ) {
      addCustomValue( v, *it5 );
    }

    addTextValue( v, EntityURL, (*it).url().url() );

    addNValue( v, *it );

    addTextValue( v, EntityNickname, (*it).nickName() );
//    addTextValue( v, EntityLabel, (*it).label() );
    addTextValue( v, EntityMailer, (*it).mailer() );
    addTextValue( v, EntityTitle, (*it).title() );
    addTextValue( v, EntityRole, (*it).role() );
    addTextValue( v, EntityOrganisation, (*it).organization() );
    addTextValue( v, EntityNote, (*it).note() );
    addTextValue( v, EntityProductID, (*it).productId() );
    addTextValue( v, EntitySortString, (*it).sortString() );

    Address::List addresses = (*it).addresses();
    Address::List::ConstIterator it3;
    for( it3 = addresses.begin(); it3 != addresses.end(); ++it3 ) {
      addAddressValue( v, *it3 );
    }

    PhoneNumber::List phoneNumbers = (*it).phoneNumbers();
    PhoneNumber::List::ConstIterator it2;
    for( it2 = phoneNumbers.begin(); it2 != phoneNumbers.end(); ++it2 ) {
      addTelephoneValue( v, *it2 );
    }

    addTextValue( v, EntityCategories, (*it).categories().join(",") );

    vcardlist.append( v );
  }

  vcards.setCardList( vcardlist );

  QFile f( fileName );
  if ( f.open(IO_WriteOnly) ) {
    QTextStream t( &f );
    t << vcards.asString();
    f.close();
  } else {
    return false;
  }

  return true;
}


void VCardFormatImpl::addCustomValue( VCard *v, const QString &txt )
{
  if ( txt.isEmpty() ) return;

  ContentLine cl;
  cl.setName( "X-" + txt.left( txt.find( ":" ) ).utf8() );
  cl.setValue( new TextValue( txt.mid( txt.find( ":" ) + 1 ).utf8() ) );
  v->add(cl);
}

void VCardFormatImpl::addTextValue( VCard *v, EntityType type, const QString &txt )
{
  if ( txt.isEmpty() ) return;

  ContentLine cl;
  cl.setName(EntityTypeToParamName( type ) );
  cl.setValue( new TextValue( txt.utf8() ) );
  v->add(cl);
}

void VCardFormatImpl::addAddressValue( VCard *vcard, const Address &a )
{
  kdDebug() << "VCardFormatImpl::addAddressValue() " << endl;
  a.dump();

  ContentLine cl;
  cl.setName(EntityTypeToParamName( EntityAddress ) );

  AdrValue *v = new AdrValue;
  v->setPOBox( a.postOfficeBox().utf8() );
  v->setExtAddress( a.extended().utf8() );
  v->setStreet( a.street().utf8() );
  v->setLocality( a.locality().utf8() );
  v->setRegion( a.region().utf8() );
  v->setPostCode( a.postalCode().utf8() );
  v->setCountryName( a.country().utf8() );
  cl.setValue( v );

  ParamList params;
  if ( a.type() & Address::Dom ) params.append( new Param( "TYPE", "dom" ) );
  if ( a.type() & Address::Intl ) params.append( new Param( "TYPE", "intl" ) );
  if ( a.type() & Address::Parcel ) params.append( new Param( "TYPE", "parcel" ) );
  if ( a.type() & Address::Postal ) params.append( new Param( "TYPE", "postal" ) );
  if ( a.type() & Address::Work ) params.append( new Param( "TYPE", "work" ) );
  if ( a.type() & Address::Home ) params.append( new Param( "TYPE", "home" ) );
  if ( a.type() & Address::Pref ) params.append( new Param( "TYPE", "pref" ) );
  cl.setParamList( params );

  vcard->add(cl);

  kdDebug() << "VCardFormatImpl::addAddressValue() done" << endl;
}

Address VCardFormatImpl::readAddressValue( ContentLine *cl )
{
  Address a;
  AdrValue *v = (AdrValue *)cl->value();
  a.setPostOfficeBox( QString::fromUtf8( v->poBox() ) );
  a.setExtended( QString::fromUtf8( v->extAddress() ) );
  a.setStreet( QString::fromUtf8( v->street() ) );
  a.setLocality( QString::fromUtf8( v->locality() ) );
  a.setRegion( QString::fromUtf8( v->region() ) );
  a.setPostalCode( QString::fromUtf8( v->postCode() ) );
  a.setCountry( QString::fromUtf8( v->countryName() ) );

  int type = 0;
  ParamList params = cl->paramList();
  ParamListIterator it( params );
  for( ; it.current(); ++it ) {
    if ( (*it)->name() == "TYPE" ) {
      if ( (*it)->value() == "dom" ) type |= Address::Dom;
      else if ( (*it)->value() == "intl" ) type |= Address::Intl;
      else if ( (*it)->value() == "parcel" ) type |= Address::Parcel;
      else if ( (*it)->value() == "postal" ) type |= Address::Postal;
      else if ( (*it)->value() == "work" ) type |= Address::Work;
      else if ( (*it)->value() == "home" ) type |= Address::Home;
      else if ( (*it)->value() == "pref" ) type |= Address::Pref;
    }
  }
  a.setType( type );

  return a;
}

void VCardFormatImpl::addNValue( VCard *vcard, const Addressee &a )
{
  ContentLine cl;
  cl.setName(EntityTypeToParamName( EntityN ) );
  NValue *v = new NValue;
  v->setFamily( a.familyName().utf8() );
  v->setGiven( a.givenName().utf8() );
  v->setMiddle( a.additionalName().utf8() );
  v->setPrefix( a.prefix().utf8() );
  v->setSuffix( a.suffix().utf8() );
  
  cl.setValue( v );
  vcard->add(cl);
}

void VCardFormatImpl::readNValue( ContentLine *cl, Addressee &a )
{
  NValue *v = (NValue *)cl->value();
  a.setFamilyName( QString::fromUtf8( v->family() ) );
  a.setGivenName( QString::fromUtf8( v->given() ) );
  a.setAdditionalName( QString::fromUtf8( v->middle() ) );
  a.setPrefix( QString::fromUtf8( v->prefix() ) );
  a.setSuffix( QString::fromUtf8( v->suffix() ) );
}

void VCardFormatImpl::addTelephoneValue( VCard *v, const PhoneNumber &p )
{
  ContentLine cl;
  cl.setName(EntityTypeToParamName(EntityTelephone));
  cl.setValue(new TelValue( p.number().utf8() ));

  ParamList params;
  if( p.type() & PhoneNumber::Home ) params.append( new Param( "TYPE", "home" ) );
  if( p.type() & PhoneNumber::Work ) params.append( new Param( "TYPE", "work" ) );
  if( p.type() & PhoneNumber::Msg ) params.append( new Param( "TYPE", "msg" ) );
  if( p.type() & PhoneNumber::Pref ) params.append( new Param( "TYPE", "pref" ) );
  if( p.type() & PhoneNumber::Voice ) params.append( new Param( "TYPE", "voice" ) );
  if( p.type() & PhoneNumber::Fax ) params.append( new Param( "TYPE", "fax" ) );
  if( p.type() & PhoneNumber::Cell ) params.append( new Param( "TYPE", "cell" ) );
  if( p.type() & PhoneNumber::Video ) params.append( new Param( "TYPE", "video" ) );
  if( p.type() & PhoneNumber::Bbs ) params.append( new Param( "TYPE", "bbs" ) );
  if( p.type() & PhoneNumber::Modem ) params.append( new Param( "TYPE", "modem" ) );
  if( p.type() & PhoneNumber::Car ) params.append( new Param( "TYPE", "car" ) );
  if( p.type() & PhoneNumber::Isdn ) params.append( new Param( "TYPE", "isdn" ) );
  if( p.type() & PhoneNumber::Pcs ) params.append( new Param( "TYPE", "pcs" ) );
  if( p.type() & PhoneNumber::Pager ) params.append( new Param( "TYPE", "pager" ) );
  cl.setParamList( params );

  v->add(cl);
}

PhoneNumber VCardFormatImpl::readTelephoneValue( ContentLine *cl )
{
  PhoneNumber p;
  TelValue *value = (TelValue *)cl->value();
  p.setNumber( QString::fromUtf8( value->asString() ) );

  int type = 0;
  ParamList params = cl->paramList();
  ParamListIterator it( params );
  for( ; it.current(); ++it ) {
    if ( (*it)->name() == "TYPE" ) {
      if ( (*it)->value() == "home" ) type |= PhoneNumber::Home;
      else if ( (*it)->value() == "work" ) type |= PhoneNumber::Work;
      else if ( (*it)->value() == "msg" ) type |= PhoneNumber::Msg;
      else if ( (*it)->value() == "pref" ) type |= PhoneNumber::Pref;
      else if ( (*it)->value() == "voice" ) type |= PhoneNumber::Voice;
      else if ( (*it)->value() == "fax" ) type |= PhoneNumber::Fax;
      else if ( (*it)->value() == "cell" ) type |= PhoneNumber::Cell;
      else if ( (*it)->value() == "video" ) type |= PhoneNumber::Video;
      else if ( (*it)->value() == "bbs" ) type |= PhoneNumber::Bbs;
      else if ( (*it)->value() == "modem" ) type |= PhoneNumber::Modem;
      else if ( (*it)->value() == "car" ) type |= PhoneNumber::Car;
      else if ( (*it)->value() == "isdn" ) type |= PhoneNumber::Isdn;
      else if ( (*it)->value() == "pcs" ) type |= PhoneNumber::Pcs;
      else if ( (*it)->value() == "pager" ) type |= PhoneNumber::Pager;
    }
  }
  p.setType( type );

  return p;
}

QString VCardFormatImpl::readTextValue( ContentLine *cl )
{
  return QString::fromUtf8( cl->value()->asString() );
}
