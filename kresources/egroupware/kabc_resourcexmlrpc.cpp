/*
    This file is part of libkabc.
    Copyright (c) 2003 - 2004 Tobias Koenig <tokoe@kde.org>

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

#include <qapplication.h>

#include <kabc/addressee.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kstringhandler.h>

#include "kabc_resourcexmlrpc.h"
#include "kabc_resourcexmlrpcconfig.h"

#include "xmlrpciface.h"

using namespace KABC;

static const QString ReadEntriesObject = "addressbook.boaddressbook.read_list";
static const QString AddEntryObject = "addressbook.boaddressbook.add";
static const QString UpdateEntryObject = "addressbook.boaddressbook.save";
static const QString DeleteEntryObject = "addressbook.boaddressbook.delete";

ResourceXMLRPC::ResourceXMLRPC( const KConfig *config )
  : Resource( config ), mServer( 0 )
{
  if ( config ) {
    init( KURL( config->readEntry( "XmlRpcUrl" ) ),
          config->readEntry( "XmlRpcDomain", "default" ),
          config->readEntry( "XmlRpcUser" ),
          KStringHandler::obscure( config->readEntry( "XmlRpcPassword" ) ) );
  } else {
    init( KURL(), "default", "", "" );
  }
}

ResourceXMLRPC::ResourceXMLRPC( const KURL &url, const QString &domain,
                                const QString &user, const QString &password )
  : Resource( 0 ), mServer( 0 )
{
  init( url, domain, user, password );
}

void ResourceXMLRPC::init( const KURL &url, const QString &domain,
                           const QString &user, const QString &password )
{
  mSyncComm = false;

  mURL = url;
  mDomain = domain;
  mUser = user;
  mPassword = password;

  mAddrTypes.insert( "dom", Address::Dom );
  mAddrTypes.insert( "intl", Address::Intl );
  mAddrTypes.insert( "parcel", Address::Parcel );
  mAddrTypes.insert( "postal", Address::Postal );
}

ResourceXMLRPC::~ResourceXMLRPC()
{
  delete mServer;
  mServer = 0;
}

void ResourceXMLRPC::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  config->writeEntry( "XmlRpcUrl", mURL.url() );
  config->writeEntry( "XmlRpcDomain", mDomain );
  config->writeEntry( "XmlRpcUser", mUser );
  config->writeEntry( "XmlRpcPassword", KStringHandler::obscure( mPassword ) );
}

Ticket *ResourceXMLRPC::requestSaveTicket()
{
  if ( !addressBook() ) {
	  kdDebug(5700) << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceXMLRPC::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceXMLRPC::doOpen()
{
  if ( mServer )
    delete mServer;

  mServer = new KXMLRPC::Server( KURL(), this );
	mServer->setUrl( KURL( mURL ) );
  mServer->setUserAgent( "KDE-AddressBook" );

  QMap<QString, QVariant> args;
  args.insert( "domain", mDomain );
  args.insert( "username", mUser );
  args.insert( "password", mPassword );

  mServer->call( "system.login", QVariant( args ),
                 this, SLOT( loginFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  enter_loop();

  return true;
}

void ResourceXMLRPC::doClose()
{
  QMap<QString, QVariant> args;
  args.insert( "sessionid", mSessionID );
  args.insert( "kp3", mKp3 );

  mServer->call( "system.logout", QVariant( args ),
                 this, SLOT( logoutFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  enter_loop();
}

bool ResourceXMLRPC::load()
{
  return true;
}

bool ResourceXMLRPC::asyncLoad()
{
  if ( !mServer )
    return false;

  QMap<QString, QVariant> fieldsMap;
  fieldsMap.insert( "fn", "fn" );
  fieldsMap.insert( "n_given", "n_given" );
  fieldsMap.insert( "n_family", "n_family" );
  fieldsMap.insert( "n_middle", "n_middle" );
  fieldsMap.insert( "n_prefix", "n_prefix" );
  fieldsMap.insert( "n_suffix", "n_suffix" );
  fieldsMap.insert( "sound", "sound" );
  fieldsMap.insert( "bday", "bday" );
  fieldsMap.insert( "note", "note" );
  fieldsMap.insert( "tz", "tz" );
  fieldsMap.insert( "geo", "geo" );
  fieldsMap.insert( "url", "url" );
  fieldsMap.insert( "pubkey", "pubkey" );
  fieldsMap.insert( "org_name", "org_name" );
  fieldsMap.insert( "org_unit", "org_unit" );
  fieldsMap.insert( "title", "title" );
  fieldsMap.insert( "adr_one_street", "adr_one_street" );
  fieldsMap.insert( "adr_one_locality", "adr_one_locality" );
  fieldsMap.insert( "adr_one_region", "adr_one_region" );
  fieldsMap.insert( "adr_one_postalcode", "adr_one_postalcode" );
  fieldsMap.insert( "adr_one_countryname", "adr_one_countryname" );
  fieldsMap.insert( "adr_one_type", "adr_one_type" );
  fieldsMap.insert( "label", "label" );
  fieldsMap.insert( "adr_two_street", "adr_two_street" );
  fieldsMap.insert( "adr_two_locality", "adr_two_locality" );
  fieldsMap.insert( "adr_two_region", "adr_two_region" );
  fieldsMap.insert( "adr_two_postalcode", "adr_two_postalcode" );
  fieldsMap.insert( "adr_two_countryname", "adr_two_countryname" );
  fieldsMap.insert( "adr_two_type", "adr_two_type" );
  fieldsMap.insert( "tel_work", "tel_work" );
  fieldsMap.insert( "tel_home", "tel_home" );
  fieldsMap.insert( "tel_voice", "tel_voice" );
  fieldsMap.insert( "tel_fax", "tel_fax" );
  fieldsMap.insert( "tel_msg", "tel_msg" );
  fieldsMap.insert( "tel_cell", "tel_cell" );
  fieldsMap.insert( "tel_pager", "tel_pager" );
  fieldsMap.insert( "tel_bbs", "tel_bbs" );
  fieldsMap.insert( "tel_modem", "tel_modem" );
  fieldsMap.insert( "tel_car", "tel_car" );
  fieldsMap.insert( "tel_isdn", "tel_isdn" );
  fieldsMap.insert( "tel_video", "tel_video" );
  fieldsMap.insert( "tel_prefer", "tel_prefer" );
  fieldsMap.insert( "email", "email" );
  fieldsMap.insert( "email_type", "email_type" );
  fieldsMap.insert( "email_home", "email_home" );
  fieldsMap.insert( "email_home_type", "email_home_type" );

  QMap<QString, QVariant> args;
  args.insert( "start", "1" );
  args.insert( "limit", "1000" );
  args.insert( "fields", QVariant( fieldsMap ) );
  args.insert( "query", "" );
  args.insert( "filter", "" );
  args.insert( "sort", "" );
  args.insert( "order", "" );

  mServer->call( ReadEntriesObject, args,
                 this, SLOT( listEntriesFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  return true;
}


bool ResourceXMLRPC::save( Ticket* )
{
  return false; // readonly
}

bool ResourceXMLRPC::asyncSave( Ticket* )
{
  return false; // readonly
}

void ResourceXMLRPC::insertAddressee( const Addressee& addr )
{
  QMap<QString, QVariant> args;
  fillArgs( addr, args );

  if ( mAddrMap.find( addr.uid() ) == mAddrMap.end() ) { // new entry
    mAddrMap.insert( addr.uid(), addr );

    mServer->call( AddEntryObject, args,
                   this, SLOT( addEntryFinished( const QValueList<QVariant>&, const QVariant& ) ),
                   this, SLOT( fault( int, const QString&, const QVariant& ) ),
                   QVariant( addr.uid() ) );
  } else {
    mAddrMap[ addr.uid() ] = addr;
    args.insert( "id", mUidMap[ addr.uid() ] );
    mServer->call( UpdateEntryObject, args,
                   this, SLOT( updateEntryFinished( const QValueList<QVariant>&, const QVariant& ) ),
                   this, SLOT( fault( int, const QString&, const QVariant& ) ) );
  }

  enter_loop();
}

void ResourceXMLRPC::removeAddressee( const Addressee& addr )
{
  int id = mUidMap[ addr.uid() ].toInt();

  mServer->call( DeleteEntryObject, id,
                 this, SLOT( deleteEntryFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ),
                 QVariant( addr.uid() ) );


  enter_loop();
}

void ResourceXMLRPC::loginFinished( const QValueList<QVariant> &variant,
                                    const QVariant& )
{
  QMap<QString, QVariant> map = variant[0].toMap();

  KURL url = mURL;
  if ( map[ "GOAWAY" ].toString() == "XOXO" ) { // failed
    mSessionID = mKp3 = "";
    addressBook()->error( i18n( "Login failed, please check your username and password." ) );
  } else {
    mSessionID = map[ "sessionid" ].toString();
    mKp3 = map[ "kp3" ].toString();
  }

  url.setUser( mSessionID );
  url.setPass( mKp3 );
  mServer->setUrl( url );

  exit_loop();
}

void ResourceXMLRPC::logoutFinished( const QValueList<QVariant> &variant,
                                     const QVariant& )
{
  QMap<QString, QVariant> map = variant[0].toMap();

  if ( map[ "GOODBYE" ].toString() != "XOXO" )
    addressBook()->error( i18n( "Logout failed, please check your username and password." ) );

  KURL url = mURL;
  mSessionID = mKp3 = "";
  url.setUser( mSessionID );
  url.setPass( mKp3 );
  mServer->setUrl( url );

  exit_loop();
}

void ResourceXMLRPC::listEntriesFinished( const QValueList<QVariant> &mapList,
                                          const QVariant& )
{
  mUidMap.clear();

  QMap<QString, QVariant> listMap = mapList[ 0 ].toMap();
  QMap<QString, QVariant>::Iterator mapIt;

  for ( mapIt = listMap.begin(); mapIt != listMap.end(); ++mapIt ) {
    QMap<QString, QVariant> entryMap = (*mapIt).toMap();
    QMap<QString, QVariant>::Iterator entryIt;

    for ( entryIt = entryMap.begin(); entryIt != entryMap.end(); ++entryIt ) {
      QMap<QString, QVariant> map = (*entryIt).toMap();
      QMap<QString, QVariant>::Iterator it;

      Addressee addr;
      Address addrOne, addrTwo;
      QString uid;

      for ( it = map.begin(); it != map.end(); ++it ) {
        if ( it.key() == "id" ) {
          uid = it.data().toString();
        } else if ( it.key() == "access" ) {
          Secrecy secrecy;
          if ( it.data().toString() == "private" )
            secrecy.setType( Secrecy::Private );
          else
            secrecy.setType( Secrecy::Public );

          addr.setSecrecy( secrecy );
        } else if ( it.key() == "fn" ) {
          addr.setFormattedName( it.data().toString() );
        } else if ( it.key() == "n_given" ) {
          addr.setGivenName( it.data().toString() );
        } else if ( it.key() == "n_family" ) {
          addr.setFamilyName( it.data().toString() );
        } else if ( it.key() == "n_middle" ) {
          addr.setAdditionalName( it.data().toString() );
        } else if ( it.key() == "n_prefix" ) {
          addr.setPrefix( it.data().toString() );
        } else if ( it.key() == "n_suffix" ) {
          addr.setSuffix( it.data().toString() );
        } else if ( it.key() == "sound" ) {
        } else if ( it.key() == "bday" ) {
          addr.setBirthday( it.data().toDateTime() );
        } else if ( it.key() == "note" ) {
          addr.setNote( it.data().toString() );
        } else if ( it.key() == "tz" ) {
          int hour = it.data().toInt();
          TimeZone timeZone( hour * 60 );
          addr.setTimeZone( timeZone );
        } else if ( it.key() == "geo" ) {
        } else if ( it.key() == "url" ) {
          addr.setUrl( KURL( it.data().toString() ) );
        } else if ( it.key() == "pubkey" ) {
        } else if ( it.key() == "org_name" ) {
          addr.setOrganization( it.data().toString() );
        } else if ( it.key() == "org_unit" ) {
        } else if ( it.key() == "title" ) {
          addr.setTitle( it.data().toString() );
        } else if ( it.key() == "adr_one_street" ) {
          addrOne.setStreet( it.data().toString() );
        } else if ( it.key() == "adr_one_locality" ) {
          addrOne.setLocality( it.data().toString() );
        } else if ( it.key() == "adr_one_region" ) {
          addrOne.setRegion( it.data().toString() );
        } else if ( it.key() == "adr_one_postalcode" ) {
          addrOne.setPostalCode( it.data().toString() );
        } else if ( it.key() == "adr_one_countryname" ) {
          addrOne.setCountry( it.data().toString() );
        } else if ( it.key() == "adr_one_type" ) {
          QStringList types = QStringList::split( ';', it.data().toString() );

          int type = Address::Work;
          for ( uint i = 0; i < types.count(); ++i )
            type += mAddrTypes[ types[ i ] ];

          addrOne.setType( type );
        } else if ( it.key() == "label" ) {
          addrOne.setLabel( it.data().toString() );
        } else if ( it.key() == "adr_two_street" ) {
          addrTwo.setStreet( it.data().toString() );
        } else if ( it.key() == "adr_two_locality" ) {
          addrTwo.setLocality( it.data().toString() );
        } else if ( it.key() == "adr_two_region" ) {
          addrTwo.setRegion( it.data().toString() );
        } else if ( it.key() == "adr_two_postalcode" ) {
          addrTwo.setPostalCode( it.data().toString() );
        } else if ( it.key() == "adr_two_countryname" ) {
          addrTwo.setCountry( it.data().toString() );
        } else if ( it.key() == "adr_two_type" ) {
          QStringList types = QStringList::split( ';', it.data().toString() );

          int type = Address::Home;
          for ( uint i = 0; i < types.count(); ++i )
            type += mAddrTypes[ types[ i ] ];

          addrTwo.setType( type );
        } else if ( it.key() == "tel_work" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Work ) );
        } else if ( it.key() == "tel_home" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Home ) );
        } else if ( it.key() == "tel_voice" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Voice ) );
        } else if ( it.key() == "tel_fax" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Fax ) );
        } else if ( it.key() == "tel_msg" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Msg ) );
        } else if ( it.key() == "tel_cell" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Cell ) );
        } else if ( it.key() == "tel_pager" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Pager ) );
        } else if ( it.key() == "tel_bbs" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Bbs ) );
        } else if ( it.key() == "tel_modem" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Modem ) );
        } else if ( it.key() == "tel_car" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Car ) );
        } else if ( it.key() == "tel_isdn" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Isdn ) );
        } else if ( it.key() == "tel_video" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Video ) );
        } else if ( it.key() == "tel_prefer" ) {
          addr.insertPhoneNumber( PhoneNumber( it.data().toString(), PhoneNumber::Pref ) );
        } else if ( it.key() == "email" ) {
          addr.insertEmail( it.data().toString(), true );
        } else if ( it.key() == "email_type" ) {
        } else if ( it.key() == "email_home" ) {
          addr.insertEmail( it.data().toString(), false );
        } else if ( it.key() == "email_home_type" ) {
        }
      }

      if ( !addrOne.isEmpty() )
        addr.insertAddress( addrOne );
      if ( !addrTwo.isEmpty() )
        addr.insertAddress( addrTwo );

      if ( !addr.isEmpty() ) {
        addr.setResource( this );
        addr.setChanged( false );

        mAddrMap.insert( addr.uid(), addr );
        mUidMap.insert( addr.uid(), uid );
      }
    }
  }

  exit_loop();

  emit loadingFinished( this );
}

void ResourceXMLRPC::deleteEntryFinished( const QValueList<QVariant>&,
                                          const QVariant &id )
{
  mAddrMap.erase( id.toString() );
  mUidMap.erase( id.toString() );

  exit_loop();
}

void ResourceXMLRPC::updateEntryFinished( const QValueList<QVariant> &list,
                                          const QVariant& )
{
  QMap<QString, QVariant> map = list[ 0 ].toMap();
  bool ok = map[ "0" ].toBool();

  if ( !ok )
    addressBook()->error( "Unable to update contact." );

  exit_loop();
}

void ResourceXMLRPC::addEntryFinished( const QValueList<QVariant> &list,
                                       const QVariant &id )
{
  QMap<QString, QVariant> map = list[ 0 ].toMap();
  int uid = map[ "0" ].toInt();

  mUidMap.insert( id.toString(), QString::number( uid ) );

  exit_loop();
}

void ResourceXMLRPC::fault( int error, const QString &errorMsg,
                            const QVariant& )
{
  QString msg = i18n( "<qt>Server send error %1: <b>%2</b></qt>" ).arg( error ).arg( errorMsg );
  addressBook()->error( msg );

  exit_loop();
}


void ResourceXMLRPC::setURL( const KURL &url )
{
  mURL = url;
}

KURL ResourceXMLRPC::url() const
{
  return mURL;
}

void ResourceXMLRPC::setDomain( const QString &domain )
{
  mDomain = domain;
}

QString ResourceXMLRPC::domain() const
{
  return mDomain;
}

void ResourceXMLRPC::setUser( const QString &user )
{
  mUser = user;
}

QString ResourceXMLRPC::user() const
{
  return mUser;
}

void ResourceXMLRPC::setPassword( const QString &password )
{
  mPassword = password;
}

QString ResourceXMLRPC::password() const
{
  return mPassword;
}

QString ResourceXMLRPC::addrTypesToTypeStr( int typeMask )
{
  QStringList types;
  QMap<QString, int>::Iterator it;
  for ( it = mAddrTypes.begin(); it != mAddrTypes.end(); ++it )
    if ( it.data() & typeMask )
      types.append( it.key() );

  return types.join( ";" );
}

void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );

void ResourceXMLRPC::enter_loop()
{
  QWidget dummy( 0, 0, WType_Dialog | WShowModal );
  dummy.setFocusPolicy( QWidget::NoFocus );
  qt_enter_modal( &dummy );
  mSyncComm = true;
  qApp->enter_loop();
  qt_leave_modal( &dummy );
}

void ResourceXMLRPC::exit_loop()
{
  if ( mSyncComm ) {
    mSyncComm = false;
    qApp->exit_loop();
  }
}

void ResourceXMLRPC::fillArgs( const Addressee &addr, QMap<QString, QVariant> &args )
{
  args.insert( "access", ( addr.secrecy().type() == Secrecy::Private ? "private" : "public" ) );
  args.insert( "fn", addr.formattedName() );
  args.insert( "n_given", addr.givenName() );
  args.insert( "n_family", addr.familyName() );
  args.insert( "n_middle", addr.additionalName() );
  args.insert( "n_prefix", addr.prefix() );
  args.insert( "n_suffix", addr.suffix() );
//  args.insert( "sound", "sound" );
  args.insert( "bday", addr.birthday() );
  args.insert( "note", addr.note() );
  int hours = addr.timeZone().offset() / 60;
  args.insert( "tz", hours );
//  args.insert( "geo", "geo" );
  args.insert( "url", addr.url().url() );
//  args.insert( "pubkey", "pubkey" );
  args.insert( "org_name", addr.organization() );
//  args.insert( "org_unit", "org_unit" );
  args.insert( "title", addr.title() );

  Address workAddr = addr.address( Address::Work );
  if ( !workAddr.isEmpty() ) {
    args.insert( "adr_one_street", workAddr.street() );
    args.insert( "adr_one_locality", workAddr.locality() );
    args.insert( "adr_one_region", workAddr.region() );
    args.insert( "adr_one_postalcode", workAddr.postalCode() );
    args.insert( "adr_one_countryname", workAddr.country() );

    args.insert( "adr_one_type", addrTypesToTypeStr( workAddr.type() ) );
    args.insert( "label", workAddr.label() );
  }

  Address homeAddr = addr.address( Address::Home );
  if ( !homeAddr.isEmpty() ) {
    args.insert( "adr_two_street", homeAddr.street() );
    args.insert( "adr_two_locality", homeAddr.locality() );
    args.insert( "adr_two_region", homeAddr.region() );
    args.insert( "adr_two_postalcode", homeAddr.postalCode() );
    args.insert( "adr_two_countryname", homeAddr.country() );
    args.insert( "adr_two_type", addrTypesToTypeStr( homeAddr.type() ) );
  }

  PhoneNumber phone = addr.phoneNumber( PhoneNumber::Work );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_work", phone.number() );

  phone = addr.phoneNumber( PhoneNumber::Home );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_home", phone.number() );

  phone = addr.phoneNumber( PhoneNumber::Voice );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_voice", phone.number() );

  phone = addr.phoneNumber( PhoneNumber::Fax );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_fax", phone.number() );

  phone = addr.phoneNumber( PhoneNumber::Msg );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_msg", phone.number() );

  phone = addr.phoneNumber( PhoneNumber::Cell );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_cell", phone.number() );

  phone = addr.phoneNumber( PhoneNumber::Pager );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_pager", phone.number() );

  phone = addr.phoneNumber( PhoneNumber::Bbs );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_bbs", phone.number() );

  phone = addr.phoneNumber( PhoneNumber::Modem );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_modem", phone.number() );

  phone = addr.phoneNumber( PhoneNumber::Car );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_car", phone.number() );

  phone = addr.phoneNumber( PhoneNumber::Isdn );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_isdn", phone.number() );

  phone = addr.phoneNumber( PhoneNumber::Video );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_video", phone.number() );

  phone = addr.phoneNumber( PhoneNumber::Pref );
  if ( !phone.number().isEmpty() )
    args.insert( "tel_prefer", phone.number() );

  if ( !addr.preferredEmail().isEmpty() ) {
    args.insert( "email", addr.preferredEmail() );
    args.insert( "email_type", "internet" );
  }

  if ( addr.emails().count() > 1 ) {
    args.insert( "email_home", addr.emails()[ 1 ] );
    args.insert( "email_home_type", "internet" );
  }
}

#include "kabc_resourcexmlrpc.moc"
