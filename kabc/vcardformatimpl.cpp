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
      EntityType type = cl->entityType();
      switch( type ) {

        case EntityUID:
          a.setUid( readTextValue( cl ) );
          break;

        case EntityEmail:
          a.setEmail( readTextValue( cl ) );
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

        case EntityTelephone:
          {
            TelValue *value = (TelValue *)cl->value();
            ParamList paramList = cl->paramList();
            PhoneNumber::Type type = PhoneNumber::Home;
            if (paramList.count() > 0 ) {
              Param *p = paramList.first();
              if (p->name() == "TYPE") {
                if ( p->value() == "home" ) {
                  type = PhoneNumber::Home;
                } else if( p->value() == "work" ) {
                  type = PhoneNumber::Office;
                }
              }
            }
            a.insertPhoneNumber( PhoneNumber( QString::fromUtf8( value->asString() ), type ) );
          }
          break;
          
        default:
          kdDebug() << "VCardFormat::load(): Unsupported entity: "
                    << int( type ) << endl;
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
    addTextValue( v, EntityEmail, (*it).email() );
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

    cl.clear();
    QStringList phoneNumberList;    
    PhoneNumber::List phoneNumbers = (*it).phoneNumbers();
    if ( phoneNumbers.count() > 0 ) {
      PhoneNumber::List::ConstIterator it2;
      for( it2 = phoneNumbers.begin(); it2 != phoneNumbers.end(); ++it2 ) {
        cl.setName(EntityTypeToParamName(EntityTelephone));
        cl.setValue(new TelValue( (*it2).number().utf8() ));
        ParamList p;
        QString paramValue;
        switch( (*it2).type() ) {
          default:
          case PhoneNumber::Home:
            paramValue = "home";
            break;
          case PhoneNumber::Office:
            paramValue = "work";
            break;
          case PhoneNumber::Mobile:
            paramValue = "cell";
            break;
          case PhoneNumber::Fax:
            paramValue = "fax";
            break;
        }
        p.append( new Param( "TYPE=" + paramValue.utf8() ) );
        cl.setParamList( p );
        v->add(cl);
      }
    }

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

QString VCardFormatImpl::readTextValue( ContentLine *cl )
{
  return QString::fromUtf8( cl->value()->asString() );
}
