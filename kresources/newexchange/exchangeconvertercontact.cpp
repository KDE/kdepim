/*
    This file is part of the exchange resource.
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "exchangeconvertercontact.h"
#include <libkcal/freebusyurlstore.h>
#include <webdavhandler.h>
#include <kdebug.h>


using namespace KABC;

ExchangeConverterContact::ExchangeConverterContact()
{
}


#define propertyDAV( prop ) \
    WebdavHandler::addElement( doc, props, "d:" prop )
#define propertyNS( ns, prop ) \
    WebdavHandler::addElementNS( doc, props, ns, prop )
#define propertyContact( prop ) \
    WebdavHandler::addElement( doc, props, "c:" prop )
#define propertyCalendar( prop ) \
    WebdavHandler::addElement( doc, props, "cal:" prop )
#define propertyHTTPMail( prop ) \
    WebdavHandler::addElement( doc, props, "m:" prop )
#define property( prop ) \
    Webdavhandler::addElement( doc, props, prop )

void ExchangeConverterContact::createRequest( QDomDocument &doc, QDomElement &props )
{
  QDomAttr att_c = doc.createAttribute( "xmlns:c" );
  att_c.setValue( "urn:schemas:contacts:" );
  doc.documentElement().setAttributeNode( att_c );
  QDomAttr att_cal = doc.createAttribute( "xmlns:cal" );
  att_cal.setValue( "urn:schemas:calendar:" );
  doc.documentElement().setAttributeNode( att_cal );
  
  propertyDAV( "contentclass" );
  propertyDAV( "getcontenttype" );
  propertyNS( "http://schemas.microsoft.com/exchange/", "outlookmessageclass" );
  propertyDAV( "getetag" );
  propertyDAV( "href" );
  propertyDAV( "creationdate" );
  propertyDAV( "getlastmodified" );
  propertyDAV( "isreadonly" );
  propertyContact( "cn" );
//   propertyContact( "uid" );
  propertyDAV( "uid" );
  propertyContact( "fileas" );
  propertyContact( "cn" );
  propertyContact( "givenName" );
  propertyContact( "initials" );
  propertyContact( "middlename" );
  propertyContact( "namesuffix" );
  propertyContact( "personaltitle" );
  propertyContact( "sn" );
  propertyContact( "title" );
  propertyContact( "o" );
  propertyContact( "department" );
  propertyContact( "roomnumber" );
  propertyContact( "profession" );
  propertyContact( "manager" );
  propertyContact( "secretarycn" );
  propertyContact( "email1" );
  propertyContact( "email2" );
  propertyContact( "email3" );
  propertyContact( "personalHomePage" );
  propertyContact( "businesshomepage" );
  propertyContact( "fburl" );
  // TODO: Does this work?
//   propertyNS( "urn:schemas-microsoft-com:office:office", "Keywords" );
  propertyDAV( "sensitivity" );
  propertyContact( "telephoneNumber" );
  propertyContact( "telephonenumber2" );
  propertyContact( "officetelephonenumber" );
  propertyContact( "office2telephonenumber" );
  propertyContact( "secretaryphone" );
  propertyContact( "organizationmainphone" );
  propertyContact( "otherTelephone" );
  propertyContact( "homePhone" );
  propertyContact( "homephone2" );
  propertyContact( "mobile" );
  propertyContact( "othermobile" );
  propertyContact( "facsimiletelephonenumber" );
  propertyContact( "homefax" );
  propertyContact( "otherfax" );
  propertyContact( "pager" );
  propertyContact( "otherpager" );
  propertyContact( "internationalisdnnumber" );
  propertyContact( "callbackphone" );
  propertyContact( "telexnumber" );
  propertyContact( "ttytddphone" );
  propertyContact( "workaddress" );
  propertyContact( "street" );
  propertyContact( "postofficebox" );
  propertyContact( "l" );
  propertyContact( "postalcode" );
  propertyContact( "st" );
  propertyContact( "co" );
  propertyContact( "c" );
  propertyContact( "homepostaladdress" );
  propertyContact( "homeStreet" );
  propertyContact( "homepostofficebox" );
  propertyContact( "homeCity" );
  propertyContact( "homePostalCode" );
  propertyContact( "homeState" );
  propertyContact( "homeCountry" );
  propertyContact( "homeCountrycode" );
  propertyContact( "mailingpostaladdress" );
  propertyContact( "mailingstreet" );
  propertyContact( "mailingpostofficebox" );
  propertyContact( "mailingcity" );
  propertyContact( "mailingpostalcode" );
  propertyContact( "mailingstate" );
  propertyContact( "mailingcountry" );
  propertyContact( "mailingcountrycode" );
  propertyContact( "otherpostaladdress" );
  propertyContact( "otherstreet" );
  propertyContact( "otherpostofficebox" );
  propertyContact( "othercity" );
  propertyContact( "otherpostalcode" );
  propertyContact( "otherstate" );
  propertyContact( "othercountry" );
  propertyContact( "othercountrycode" );
  propertyContact( "nickname" );
  propertyContact( "spousecn" );
  propertyContact( "bday" );
  propertyContact( "weddinganniversary" );
  propertyCalendar( "geolatitude" );
  propertyCalendar( "geolongitude" );
  propertyContact( "mapurl" );
  propertyContact( "location" );
  propertyHTTPMail( "textdescription" );
  propertyHTTPMail( "date" );
  propertyContact( "usercertificate" );
}
#undef propertyDAV
#undef propertyNS
#undef propertyContact
#undef propertyCalendar
#undef propertyHTTPMail
#undef property


bool ExchangeConverterContact::extractAddress( const QDomElement &node, 
    Addressee &addressee, int type,
    const QString &street, const QString &pobox, const QString &location, 
    const QString &postalcode, const QString &state, const QString &country, 
    const QString &/*countycode*/ )
{
  bool haveAddr = false;
  Address addr( type );
  QString tmpstr;

  if ( WebdavHandler::extractString( node, street, tmpstr ) ) {
    addr.setStreet( tmpstr );
    haveAddr = true;
  }
  if ( WebdavHandler::extractString( node, pobox, tmpstr ) ) {
    addr.setPostOfficeBox( tmpstr );
    haveAddr = true;
  }
  if ( WebdavHandler::extractString( node, location, tmpstr ) ) {
    addr.setLocality( tmpstr );
    haveAddr = true;
  }
  if ( WebdavHandler::extractString( node, postalcode, tmpstr ) ) {
    addr.setPostalCode( tmpstr );
    haveAddr = true;
  }
  if ( WebdavHandler::extractString( node, state, tmpstr ) ) {
    addr.setRegion( tmpstr );
    haveAddr = true;
  }
  if ( WebdavHandler::extractString( node, country, tmpstr ) ) {
    addr.setCountry( tmpstr );
    haveAddr = true;
  }
//   if ( WebdavHandler::extractString( node, countrycode, tmpstr ) ) {
//     addr.setCountryCode( tmpstr );
//     haveAddr = true;
//   }
  if ( haveAddr ) {
    addressee.insertAddress( addr );
  }
  return haveAddr;
}


/**
For the complete list of Exchange <=> KABC field mappings see the file 
Person.mapping */
bool ExchangeConverterContact::readAddressee( const QDomElement &node, Addressee &addressee ) 
{
  QString tmpstr;
  long tmplng;
  
  // The UID is absolutely required!
  if ( WebdavHandler::extractString( node, "uid", tmpstr ) ) {
    addressee.setUid( tmpstr );
  } else {
    kdDebug()<<"Addressee does not have a UID"<<endl;
    return false;
  }
  if ( WebdavHandler::extractString( node, "getetag", tmpstr ) )
    addressee.insertCustom( "KDEPIM-Exchange-Resource", "fingerprint", tmpstr );
  if ( WebdavHandler::extractString( node, "href", tmpstr ) )
    addressee.insertCustom( "KDEPIM-Exchange-Resource", "href", tmpstr );
    
/* KDE4: addressee does not have any creation or modification date :-(( */
/* KDE4: read-only not supported by libkabc */

  // Name: first, family, pre/postfixes, common name
  if ( WebdavHandler::extractString( node, "fileas", tmpstr ) ||
       WebdavHandler::extractString( node, "cn", tmpstr ) )
    addressee.setFormattedName( tmpstr );
  if ( WebdavHandler::extractString( node, "givenName", tmpstr ) ) 
    addressee.setGivenName( tmpstr );
  if ( WebdavHandler::extractString( node, "middlename", tmpstr ) ) 
    addressee.setAdditionalName( tmpstr );
  if ( WebdavHandler::extractString( node, "sn", tmpstr ) ) 
    addressee.setFamilyName( tmpstr );
  //urn:schemas:contacts:initials not used      -
  if ( WebdavHandler::extractString( node, "namesuffix", tmpstr ) ) 
    addressee.setSuffix( tmpstr );
  if ( WebdavHandler::extractString( node, "personaltitle", tmpstr ) ) 
    addressee.setPrefix( tmpstr );

  // Role
  if ( WebdavHandler::extractString( node, "title", tmpstr ) )
    addressee.setRole( tmpstr );

  // Company-Related settings
  if ( WebdavHandler::extractString( node, "o", tmpstr ) ) 
    addressee.setOrganization( tmpstr );
  if ( WebdavHandler::extractString( node, "department", tmpstr ) ) 
    addressee.insertCustom( "KADDRESSBOOK", "X-Department", tmpstr );
  if ( WebdavHandler::extractString( node, "roomnumber", tmpstr ) ) 
    addressee.insertCustom( "KADDRESSBOOK", "X-Office", tmpstr );
  if ( WebdavHandler::extractString( node, "profession", tmpstr ) ) 
    addressee.insertCustom( "KADDRESSBOOK", "X-Profession", tmpstr );
  if ( WebdavHandler::extractString( node, "manager", tmpstr ) ) 
    addressee.insertCustom( "KADDRESSBOOK", "X-ManagersName", tmpstr );

  if ( WebdavHandler::extractString( node, "secretarycn", tmpstr ) ) 
    addressee.insertCustom( "KADDRESSBOOK", "X-AssistantsName", tmpstr );


  // Web-Related settings
  if ( WebdavHandler::extractString( node, "email1", tmpstr ) )
    addressee.insertEmail( tmpstr, true );
  if ( WebdavHandler::extractString( node, "email2", tmpstr ) ) 
    addressee.insertEmail( tmpstr );
  if ( WebdavHandler::extractString( node, "email3", tmpstr ) ) 
    addressee.insertEmail( tmpstr );
    
  // No kabc field for personalHomePage
  if ( WebdavHandler::extractString( node, "businesshomepage", tmpstr ) ) 
    addressee.setUrl( tmpstr );

  if ( WebdavHandler::extractString( node, "fburl", tmpstr ) ) {
    KCal::FreeBusyUrlStore::self()->writeUrl( addressee.preferredEmail(), tmpstr );
    KCal::FreeBusyUrlStore::self()->sync();
  }


  // General stuff:
  QStringList tmplst;
  if ( WebdavHandler::extractStringList( node, "Keywords", tmplst ) ) 
    addressee.setCategories( tmplst );
  // Exchange sentitivity values:
  // 0 None, 1 Personal, 2 Private, 3 Company Confidential
  if ( WebdavHandler::extractLong( node, "sensitivity", tmplng ) ) {
    switch( tmplng ) {
      case 0: addressee.setSecrecy( KABC::Secrecy::Public ); break;
      case 1: 
      case 2: addressee.setSecrecy( KABC::Secrecy::Private ); break;
      case 3: addressee.setSecrecy( KABC::Secrecy::Confidential ); break;
      default: kdWarning() << "Unknown sensitivity: " << tmplng << endl;
    }
  }

#define insertPhone( name, type ) \
  if ( WebdavHandler::extractString( node, name, tmpstr ) ) \
    addressee.insertPhoneNumber( KABC::PhoneNumber( tmpstr, type ) );
  // Phone numbers
  insertPhone( "telephoneNumber",         PhoneNumber::Work );
//insertPhone( "telephonenumber2",        PhoneNumber::Work );
  insertPhone( "officetelephonenumber",   PhoneNumber::Work );
//insertPhone( "office2telephonenumber",  PhoneNumber::Work );
//insertPhone( "secretaryphone",          PhoneNumber::Work );
//insertPhone( "organizationmainphone",   PhoneNumber::Work );
  insertPhone( "otherTelephone",          0 );
  insertPhone( "homePhone",               PhoneNumber::Home );
//insertPhone( "homephone2",              PhoneNumber::Home );
  insertPhone( "mobile",                  PhoneNumber::Cell );
//insertPhone( "othermobile",             PhoneNumber::Cell );
  insertPhone( "facsimiletelephonenumber",PhoneNumber::Fax );
  insertPhone( "homefax",                 PhoneNumber::Fax | PhoneNumber::Home );
  insertPhone( "otherfax",                PhoneNumber::Fax | PhoneNumber::Work );
  insertPhone( "pager",                   PhoneNumber::Pager | PhoneNumber::Work );
  insertPhone( "otherpager",              PhoneNumber::Pager );
  insertPhone( "internationalisdnnumber", PhoneNumber::Isdn );
  insertPhone( "callbackphone",           PhoneNumber::Msg );
  insertPhone( "telexnumber",             PhoneNumber::Bbs );
  insertPhone( "ttytddphone",             PhoneNumber::Pcs );
#undef insertPhone  

  // Addresses: Work, Home, Mailing and Other:
  extractAddress( node, addressee, Address::Work | Address::Pref,
    "street", "postofficebox", "l", "postalcode", "st", "co", "c" );
  extractAddress( node, addressee, Address::Home,
    "homeStreet", "homepostofficebox", "homeCity", "homePostalCode", 
    "homeState", "homeCountry", "homeCountrycode" );
  extractAddress( node, addressee, Address::Postal,
    "mailingstreet", "mailingpostofficebox", "mailingcity", "mailingpostalcode", 
    "mailingstate", "mailingcountry", "mailingcountrycode" );
  extractAddress( node, addressee, 0,
    "otherstreet", "otherpostofficebox", "othercity", "otherpostalcode", 
    "otherstate", "othercountry", "othercountrycode" );


  if ( WebdavHandler::extractString( node, "nickname", tmpstr ) ) 
    addressee.setNickName( tmpstr );
  if ( WebdavHandler::extractString( node, "spousecn", tmpstr ) ) 
    addressee.insertCustom( "KADDRESSBOOK", "X-SpousesName", tmpstr );

  QDateTime tmpdt;
  if ( WebdavHandler::extractDateTime( node, "bday", tmpdt ) ) 
    addressee.setBirthday( tmpdt.date() );
  if ( WebdavHandler::extractString( node, "weddinganniversary", tmpstr ) ) 
    addressee.insertCustom( "KADDRESSBOOK", "X-Anniversary", tmpstr );

// TODO? timeZone()

  float lt,lng;
  if ( WebdavHandler::extractFloat( node, "geolatitude", lt ) && 
       WebdavHandler::extractFloat( node, "geolongitude", lng ) )
    addressee.setGeo( Geo( lt, lng ) );
  // TODO: mapurl
  // TODO: location

  if ( WebdavHandler::extractString( node, "textdescription", tmpstr ) )
    addressee.setNote( tmpstr );

//   if ( WebdavHandler::extractString( node, "usercertificate", tmpstr ) ) 
//     addressee.setKeys()

  return true;
}

Addressee::List ExchangeConverterContact::parseWebDAV( const QDomDocument& davdata )
{
  // TODO: Handle multiple addressees per reply!
  Addressee::List list;

  QDomElement prop = davdata.documentElement().namedItem( "response" )
                     .namedItem( "propstat" ).namedItem( "prop" ).toElement();
  if ( prop.isNull() ) {
kdDebug()<<"ExchangeConverterContact::parseWebDAV, no response->propstat->prop element!"<<endl;
    return list;
  }
 
  QString contentclass;
  bool success = WebdavHandler::extractString( prop, "contentclass", contentclass );
  if ( !success ) {
kdDebug()<<"ExchangeConverterContact::parseWebDAV, No contentclass entry"<<endl;
    return list;
  }
  
  success = false;
  Addressee addressee;
  if ( contentclass == "urn:content-classes:person" ) {
    success = readAddressee( prop, addressee );
  } 
  
  if ( success ) {
    list.append( addressee );
  } else {
  
  }
  return list;
}

#define domDavProperty( name, value ) \
  WebdavHandler::addElement( doc, prop, "d:" name, value );
#define domProperty( NS, name, value ) \
  WebdavHandler::addElementNS( doc, prop, NS, name, value );
#define domContactProperty( name, value ) \
  WebdavHandler::addElement( doc, prop, "c:" name, value );
#define domCalendarProperty( name, value ) \
  WebdavHandler::addElement( doc, prop, "cal:" name, value );
#define domPhoneProperty( name, type ) \
  domContactProperty( name, addr.phoneNumber( type ).number() );


QDomDocument ExchangeConverterContact::createWebDAV( Addressee addr )
{
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement( doc, doc, "d:propertyupdate" );
  QDomElement set = WebdavHandler::addElement( doc, root, "d:set" );
  QDomElement prop = WebdavHandler::addElement( doc, set, "d:prop" );
  
  QDomAttr att_c = doc.createAttribute( "xmlns:c" );
  att_c.setValue( "urn:schemas:contacts:" );
  doc.documentElement().setAttributeNode( att_c );
  
  QDomAttr att_b = doc.createAttribute( "xmlns:b" );
  att_b.setValue( "urn:schemas-microsoft-com:datatypes" );
  root.setAttributeNode( att_b );

  domDavProperty( "contentclass", "urn:content-classes:person" );
  domProperty( "http://schemas.microsoft.com/exchange/",
               "outlookmessageclass", "IPM.Contact" );
//   domContactProperty( "uid", addr.uid() );
  
  domContactProperty( "fileas", addr.formattedName() );
  domContactProperty( "givenName", addr.givenName() );
  domContactProperty( "middlename", addr.additionalName() );
  domContactProperty( "sn", addr.familyName() );
  domContactProperty( "namesuffix", addr.suffix() );
  domContactProperty( "personaltitle", addr.prefix() );
  
  domContactProperty( "title", addr.role() );
  domContactProperty( "o", addr.organization() );
  domContactProperty( "department", addr.custom( "KADDRESSBOOK", "X-Department" ) );
  domContactProperty( "roomnumber", addr.custom( "KADDRESSBOOK", "X-Office" ) );
  domContactProperty( "profession", addr.custom( "KADDRESSBOOK", "X-Profession" ) );
  domContactProperty( "manager", addr.custom( "KADDRESSBOOK", "X-ManagersName" ) );
  domContactProperty( "secretarycn", addr.custom( "KADDRESSBOOK", "X-AssistantsName" ) );
  
  QStringList emails = addr.emails();
  QString prefemail = addr.preferredEmail();
  if ( emails.contains( prefemail ) ) 
    emails.remove( prefemail );
  emails.prepend( prefemail );
  if ( emails.count() > 0 ) {
    domContactProperty( "email1", emails[0] );
  }
  if ( emails.count() > 1 ) {
    domContactProperty( "email2", emails[1] );
  }
  if ( emails.count() > 2 ) {
    domContactProperty( "email3", emails[2] );
  }
  
  // No value for "personalHomePage"
  domContactProperty( "businesshomepage", addr.url().url() );
  
  QString fburl = KCal::FreeBusyUrlStore::self()->readUrl( addr.preferredEmail() );
  if ( !fburl.isEmpty() ) {
    domContactProperty( "fburl", fburl );
  }

/* FIXME: This doesn't work!
  QStringList cats = addr.categories();
  if ( cats.isEmpty() ) {
    QDomElement catsnode = WebdavHandler::addElementNS( doc, prop, 
                "urn:schemas-microsoft-com:office:office", "Keywords" );
    for ( QStringList::Iterator it = cats.begin(); it != cats.end(); ++it ) {
      WebdavHandler::addElementNS( doc, catsnode, "xml:", "v", *it );
    }
  } else {
//     QDomElement catsnode = addProperty( doc, prop, 
//                 "urn:schemas-microsoft-com:office:office", "Keywords", "" );
  }*/
  
  // Exchange sentitivity values:
  // 0 None, 1 Personal, 2 Private, 3 Company Confidential
  QString value;
  switch ( addr.secrecy().type() ) {
    case KABC::Secrecy::Private: value = "2"; break;
    case KABC::Secrecy::Confidential: value = "3"; break;
    default: value = "0";
  }
  domDavProperty( "sensitivity", value );

  // Phone numbers
  domPhoneProperty( "telephoneNumber", PhoneNumber::Work );
//   domPhoneProperty( "telephonenumber2", PhoneNumber::Work );
  domPhoneProperty( "officetelephonenumber", PhoneNumber::Work );
//   domPhoneProperty( "office2telephonenumber", PhoneNumber::Work );
//   domPhoneProperty( "secretaryphone", PhoneNumber::Work );
//   domPhoneProperty( "organizationmainphone", PhoneNumber::Work );
  domPhoneProperty( "otherTelephone", 0 );
  domPhoneProperty( "homePhone", PhoneNumber::Home );
//   domPhoneProperty( "homephone2", PhoneNumber::Home );
  domPhoneProperty( "mobile", PhoneNumber::Cell );
  domPhoneProperty( "othermobile", PhoneNumber::Cell | PhoneNumber::Home );
  domPhoneProperty( "facsimiletelephonenumber", PhoneNumber::Fax );
  domPhoneProperty( "homefax", PhoneNumber::Fax | PhoneNumber::Home );
  domPhoneProperty( "otherfax", PhoneNumber::Fax | PhoneNumber::Work );
  domPhoneProperty( "pager", PhoneNumber::Pager | PhoneNumber::Work );
  domPhoneProperty( "otherpager", PhoneNumber::Pager );
  domPhoneProperty( "internationalisdnnumber", PhoneNumber::Isdn );
  domPhoneProperty( "callbackphone", PhoneNumber::Msg );
  domPhoneProperty( "telexnumber", PhoneNumber::Bbs );
  domPhoneProperty( "ttytddphone", PhoneNumber::Pcs );

      
// work address:
  Address workaddr = addr.address( Address::Work | Address::Pref );
  if ( !workaddr.isEmpty() ) {
    domContactProperty( "street", workaddr.street() );
    domContactProperty( "postofficebox", workaddr.postOfficeBox() );
    domContactProperty( "l", workaddr.locality() );
    domContactProperty( "postalcode", workaddr.postalCode() );
    domContactProperty( "st", workaddr.region() );
    domContactProperty( "co", workaddr.country() );
    // domContactProperty( "c", workaddr.countryCode() );
  }

  // home address:
  Address homeaddr = addr.address( Address::Home );
  if ( !homeaddr.isEmpty() ) {
    domContactProperty( "homeStreet", homeaddr.street() );
    domContactProperty( "homepostofficebox", homeaddr.postOfficeBox() );
    domContactProperty( "homeCity", homeaddr.locality() );
    domContactProperty( "homePostalCode", homeaddr.postalCode() );
    domContactProperty( "homeState", homeaddr.region() );
    domContactProperty( "homeCountry", homeaddr.country() );
    // domContactProperty( "homeCountrycode", homeaddr.countryCode() );
  }
  
  // mailing address:
  Address mailingaddr = addr.address( Address::Postal );
  if ( !mailingaddr.isEmpty() ) {
    domContactProperty( "mailingstreet", mailingaddr.street() );
    domContactProperty( "mailingpostofficebox", mailingaddr.postOfficeBox() );
    domContactProperty( "mailingcity", mailingaddr.locality() );
    domContactProperty( "mailingpostalcode", mailingaddr.postalCode() );
    domContactProperty( "mailingstate", mailingaddr.region() );
    domContactProperty( "mailingcountry", mailingaddr.country() );
    // domContactProperty( "mailingcountrycode", mailingaddr.countryCode() );
  }
  
  // other address:
  Address otheraddr = addr.address( 0 );
  if ( !otheraddr.isEmpty() ) {
    domContactProperty( "otherstreet", otheraddr.street() );
    domContactProperty( "otherpostofficebox", otheraddr.postOfficeBox() );
    domContactProperty( "othercity", otheraddr.locality() );
    domContactProperty( "otherpostalcode", otheraddr.postalCode() );
    domContactProperty( "otherstate", otheraddr.region() );
    domContactProperty( "othercountry", otheraddr.country() );
    // domContactProperty( "othercountrycode", otheraddr.countryCode() );
  }
  
  
  domContactProperty( "nickname", addr.nickName() );
  domContactProperty( "spousecn", addr.custom( "KADDRESSBOOK", "X-SpousesName" ) );

 // TODO: Birthday and Anniversary:
 QDate dt = addr.birthday().date();
 QString str = (dt.isValid())?(dt.toString( Qt::ISODate )):(QString::null);
 QDomElement el = domContactProperty( "bday", str );
 el.setAttribute( "b:dt", "date" );
 
 dt = QDate::fromString( addr.custom( "KADDRESSBOOK", "X-Anniversary" ), Qt::ISODate );
 str = (dt.isValid())?(dt.toString( Qt::ISODate )):(QString::null);
 el = domContactProperty( "weddinganniversary", str );
 el.setAttribute( "b:dt", "date" );
/*  if ( WebdavHandler::extractDateTime( node, "bday", tmpdt ) ) 
    addressee.setBirthday( tmpdt.date() );
  if ( WebdavHandler::extractString( node, "weddinganniversary", tmpstr ) ) 
    addressee.insertCustom( "KADDRESSBOOK", "X-Anniversary", tmpstr );*/

// ? TODO: timeZone()

  KABC::Geo geo = addr.geo();
  if ( geo.isValid() ) {
    // TODO: Do we need to set any other attribute to make it a float?
    QDomAttr att_cal = doc.createAttribute( "xmlns:cal" );
    att_cal.setValue( "urn:schemas:calendar:" );
    doc.documentElement().setAttributeNode( att_cal );
    QDomElement el = domCalendarProperty( "geolatitude", QString::number( geo.latitude() ) );
    el.setAttribute( "b:dt", "float" );
    el = domCalendarProperty( "geolongitude", QString::number( geo.longitude() ) );
    el.setAttribute( "b:dt", "float" );
  }

  domContactProperty( "textdescription", addr.note() );

  // TODO:usercertificate
  // TODO: custom fields
kdDebug()<<"DOM document: "<<doc.toString() << endl;
  
  return doc;
}
#undef domDavProperty
#undef domProperty
#undef domContactProperty
#undef domPhoneProperty
