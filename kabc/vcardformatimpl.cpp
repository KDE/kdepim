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
          {
            QStringList str = QStringList::split( ";", readTextValue( cl ) );
            QStringList::ConstIterator it = str.begin();
            if ( it != str.end() ) {
              a.setFamilyName( *it++ );
            }
            if ( it != str.end() ) {
              a.setGivenName( *it++ );
            }
            if ( it != str.end() ) {
              a.setAdditionalName( *it++ );
            }
            if ( it != str.end() ) {
              a.setPrefix( *it++ );
            }
            if ( it != str.end() ) {
              a.setSuffix( *it );
            }
          }
          break;

        case EntityTelephone:
          {
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

    value = (*it).familyName() + ";" + (*it).givenName() + ";" +
            (*it).additionalName() + ";" + (*it).prefix() + ";" +
            (*it).suffix();
    addTextValue( v, EntityN, value );

    addTextValue( v, EntityNickname, (*it).nickName() );
//    addTextValue( v, EntityLabel, (*it).label() );
    addTextValue( v, EntityMailer, (*it).mailer() );
    addTextValue( v, EntityTitle, (*it).title() );
    addTextValue( v, EntityRole, (*it).role() );
    addTextValue( v, EntityOrganisation, (*it).organization() );
    addTextValue( v, EntityNote, (*it).note() );
    addTextValue( v, EntityProductID, (*it).productId() );
    addTextValue( v, EntitySortString, (*it).sortString() );

    

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

void VCardFormatImpl::addTextValue( VCard *v, EntityType type, const QString &txt )
{
  if ( txt.isEmpty() ) return;

  ContentLine cl;
  cl.setName(EntityTypeToParamName( type ) );
  cl.setValue( new TextValue( txt.utf8() ) );
  v->add(cl);
}

QString VCardFormatImpl::readTextValue( ContentLine *cl )
{
  return QString::fromUtf8( cl->value()->asString() );
}
