#include <qfile.h>
#include <qtextstream.h>

#include <ksimpleconfig.h>
#include <kdebug.h>

#include <VCard.h>

#include "addressbook.h"

#include "vcardformat.h"

using namespace KABC;
using namespace VCARD;

bool VCardFormat::load( AddressBook *addressBook, const QString &fileName )
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

    Addressee a;

    if (v.has(EntityUID)) {
      QCString s = v.contentLine(EntityUID)->value()->asString();
      a.setUid( QString::fromUtf8( s ) );
    }

    if (v.has(EntityEmail)) {
      QCString s = v.contentLine(EntityEmail)->value()->asString();
      a.setEmail( QString::fromUtf8( s ) );
    }

    if (v.has(EntityName)) {
      QCString s = v.contentLine(EntityName)->value()->asString();
      a.setName( QString::fromUtf8( s ) );
    }

    if (v.has(EntityFullName)) {
      QCString s = v.contentLine(EntityFullName)->value()->asString();
      a.setFormattedName( QString::fromUtf8( s ) );
    }
  
    if (v.has(EntityURL)) {
      QCString s = v.contentLine(EntityURL)->value()->asString();
      a.setUrl( KURL( QString::fromUtf8( s ) ) );
    }
  
    if (v.has(EntityTelephone)) {
      ContentLine *cl = v.contentLine(EntityTelephone);
      TelValue *value = (TelValue *)cl->value();
      ParamList paramList = cl->paramList();
      PhoneNumber::Type type = PhoneNumber::Home;
      if (paramList.count() > 0 ) {
        QCString typeString = paramList.first()->asString();
        if ( typeString == "home" ) {
          type = PhoneNumber::Home;
        } else if( typeString == "work" ) {
          type = PhoneNumber::Office;
        }
      }
      a.insertPhoneNumber( PhoneNumber( QString::fromUtf8( value->asString() ), type ) );
    }
  
    addressBook->insertAddressee( a );
  }
  
  return true;
}

bool VCardFormat::save( AddressBook *addressBook, const QString &fileName )
{
  kdDebug() << "VCardFormat::save(): " << fileName << endl;

  VCardEntity vcards;
  VCardList vcardlist;
  vcardlist.setAutoDelete( true );
  ContentLine cl;

  AddressBook::Iterator it;
  for ( it = addressBook->begin(); it != addressBook->end(); ++it ) {
    VCard *v = new VCard;

    cl.setName( EntityTypeToParamName( EntityName ) );
    cl.setValue( new TextValue( (*it).name().utf8() ) );
    v->add( cl );

    cl.clear();
    cl.setName( EntityTypeToParamName( EntityUID ) );
    cl.setValue( new TextValue( (*it).uid().utf8() ) );
    v->add( cl );

    cl.clear();
    cl.setName(EntityTypeToParamName( EntityFullName ) );
    cl.setValue( new TextValue( (*it).formattedName().utf8() ) );
    v->add(cl);

    QCString str = (*it).email().utf8();
    if ( !str.isEmpty() ) {
      cl.clear();
      cl.setName(EntityTypeToParamName( EntityEmail ) );
      cl.setValue( new TextValue( str ) );
      v->add(cl);
    }

    KURL url  = (*it).url();
    if ( !url.isEmpty() ) {
      cl.clear();
      cl.setName(EntityTypeToParamName( EntityURL ) );
      cl.setValue( new TextValue( url.url().utf8() ) );
      v->add(cl);
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
        p.append( new TelParam( paramValue.utf8() ) );
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
