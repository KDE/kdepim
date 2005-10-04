/*
    This file is part of kdepim.
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qapplication.h>

#include <kabc/addressee.h>
#include <kabprefs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <libkcal/freebusyurlstore.h>
#include <libkdepim/kpimprefs.h>

#include "kabc_egroupwareprefs.h"
#include "kabc_resourcexmlrpc.h"
#include "kabc_resourcexmlrpcconfig.h"

#include "access.h"
#include "synchronizer.h"
#include "xmlrpciface.h"

using namespace KABC;

static const QString SearchContactsCommand = "addressbook.boaddressbook.search";
static const QString AddContactCommand = "addressbook.boaddressbook.write";
static const QString DeleteContactCommand = "addressbook.boaddressbook.delete";
static const QString LoadCategoriesCommand = "addressbook.boaddressbook.categories";
static const QString LoadCustomFieldsCommand = "addressbook.boaddressbook.customfields";

static void setRights( KABC::Addressee &addr, int rights )
{
  addr.insertCustom( "EGWRESOURCE", "RIGHTS", QString::number( rights ) );
}

static int rights( const KABC::Addressee &addr )
{
  return addr.custom( "EGWRESOURCE", "RIGHTS" ).toInt();
}

ResourceXMLRPC::ResourceXMLRPC( const KConfig *config )
  : ResourceCached( config ), mServer( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config )
    mPrefs->readConfig();

  initEGroupware();
}

ResourceXMLRPC::ResourceXMLRPC( const QString &url, const QString &domain,
                                const QString &user, const QString &password )
  : ResourceCached( 0 ), mServer( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  mPrefs->setUrl( url );
  mPrefs->setDomain( domain );
  mPrefs->setUser( user );
  mPrefs->setPassword( password );

  initEGroupware();
}

void ResourceXMLRPC::init()
{
  setType( "xmlrpc" );

  mSynchronizer = new Synchronizer;

  mPrefs = new EGroupwarePrefs;
}

void ResourceXMLRPC::initEGroupware()
{
  KURL url( mPrefs->url() );

  mAddrTypes.insert( "dom", Address::Dom );
  mAddrTypes.insert( "intl", Address::Intl );
  mAddrTypes.insert( "parcel", Address::Parcel );
  mAddrTypes.insert( "postal", Address::Postal );
}

ResourceXMLRPC::~ResourceXMLRPC()
{
  saveCache();

  delete mServer;
  mServer = 0;

  delete mPrefs;
  mPrefs = 0;

  delete mSynchronizer;
  mSynchronizer = 0;
}

void ResourceXMLRPC::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  mPrefs->writeConfig();
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
  mServer->setUrl( KURL( mPrefs->url() ) );
  mServer->setUserAgent( "KDE-AddressBook" );

  QMap<QString, QVariant> args;
  args.insert( "domain", mPrefs->domain() );
  args.insert( "username", mPrefs->user() );
  args.insert( "password", mPrefs->password() );

  mServer->call( "system.login", QVariant( args ),
                 this, SLOT( loginFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  mSynchronizer->start();

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

  mSynchronizer->start();
}

bool ResourceXMLRPC::load()
{
  mAddrMap.clear();

  return true;
}

bool ResourceXMLRPC::asyncLoad()
{
  if ( !mServer )
    return false;

  mAddrMap.clear();

  loadCache();

  QMap<QString, QVariant> args;
  args.insert( "start", "0" );
  args.insert( "query", "" );
  args.insert( "filter", "" );
  args.insert( "sort", "" );
  args.insert( "order", "" );
  args.insert( "include_users", "calendar" );

  mServer->call( SearchContactsCommand, args,
                 this, SLOT( listContactsFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  mServer->call( LoadCategoriesCommand, QVariant( false, 0 ),
                 this, SLOT( loadCategoriesFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  mServer->call( LoadCustomFieldsCommand, QVariant( QValueList<QVariant>() ),
                 this, SLOT( loadCustomFieldsFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  return true;
}


bool ResourceXMLRPC::save( Ticket *ticket )
{
  return asyncSave( ticket );
}

bool ResourceXMLRPC::asyncSave( Ticket* )
{
  KABC::Addressee::List::ConstIterator it;

  const KABC::Addressee::List addedList = addedAddressees();
  for ( it = addedList.begin(); it != addedList.end(); ++it ) {
    addContact( *it );
  }

  const KABC::Addressee::List changedList = changedAddressees();
  for ( it = changedList.begin(); it != changedList.end(); ++it ) {
    updateContact( *it );
  }

  const KABC::Addressee::List deletedList = deletedAddressees();
  for ( it = deletedList.begin(); it != deletedList.end(); ++it ) {
    deleteContact( *it );
  }

  return true;
}

void ResourceXMLRPC::addContact( const Addressee& addr )
{
  QMap<QString, QVariant> args;
  writeContact( addr, args );

  mServer->call( AddContactCommand, args,
                 this, SLOT( addContactFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( addContactFault( int, const QString&, const QVariant& ) ),
                 QVariant( addr.uid() ) );
}

void ResourceXMLRPC::updateContact( const Addressee& addr )
{
  if ( !(rights( addr ) & EGW_ACCESS_DELETE) && (rights( addr ) != -1) ) {
    clearChange( addr.uid() );
    return;
  }

  QMap<QString, QVariant> args;
  writeContact( addr, args );

  args.insert( "id", idMapper().remoteId( addr.uid() ) );
  mServer->call( AddContactCommand, args,
                 this, SLOT( updateContactFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( updateContactFault( int, const QString&, const QVariant& ) ),
                 QVariant( addr.uid() ) );
}

void ResourceXMLRPC::deleteContact( const Addressee& addr )
{
  if ( !(rights( addr ) & EGW_ACCESS_DELETE) && rights( addr ) != -1 ) {
    clearChange( addr.uid() );
    idMapper().removeRemoteId( idMapper().remoteId( addr.uid() ) );
    return;
  }

  mServer->call( DeleteContactCommand, idMapper().remoteId( addr.uid() ),
                 this, SLOT( deleteContactFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( deleteContactFault( int, const QString&, const QVariant& ) ),
                 QVariant( addr.uid() ) );
}

void ResourceXMLRPC::loginFinished( const QValueList<QVariant> &variant,
                                    const QVariant& )
{
  QMap<QString, QVariant> map = variant[0].toMap();

  KURL url( mPrefs->url() );
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

  mSynchronizer->stop();
}

void ResourceXMLRPC::logoutFinished( const QValueList<QVariant> &variant,
                                     const QVariant& )
{
  QMap<QString, QVariant> map = variant[0].toMap();

  if ( map[ "GOODBYE" ].toString() != "XOXO" )
    addressBook()->error( i18n( "Logout failed, please check your username and password." ) );

  KURL url( mPrefs->url() );
  mSessionID = mKp3 = "";
  url.setUser( mSessionID );
  url.setPass( mKp3 );
  mServer->setUrl( url );

  mSynchronizer->stop();
}

void ResourceXMLRPC::listContactsFinished( const QValueList<QVariant> &mapList,
                                           const QVariant& )
{
  const QValueList<QVariant> contactList = mapList[ 0 ].toList();
  QValueList<QVariant>::ConstIterator contactIt;

  KABC::Addressee::List serverContacts;
  for ( contactIt = contactList.begin(); contactIt != contactList.end(); ++contactIt ) {
    const QMap<QString, QVariant> map = (*contactIt).toMap();

    Addressee addr;
    QString uid;

    readContact( map, addr, uid );

    if ( !addr.isEmpty() ) {
      addr.setResource( this );
      addr.setChanged( false );

      QString local = idMapper().localId( uid );
      if ( local.isEmpty() ) { // new entry
        idMapper().setRemoteId( addr.uid(), uid );
      } else {
        addr.setUid( local );
      }

      mAddrMap.insert( addr.uid(), addr );
      serverContacts.append( addr );
    }
  }

  cleanUpCache( serverContacts );
  saveCache();

  emit loadingFinished( this );
}

void ResourceXMLRPC::addContactFinished( const QValueList<QVariant> &list,
                                         const QVariant &id )
{
  clearChange( id.toString() );
  idMapper().setRemoteId( id.toString(), list[ 0 ].toString() );

  saveCache();
}

void ResourceXMLRPC::updateContactFinished( const QValueList<QVariant>&,
                                            const QVariant &id )
{
  clearChange( id.toString() );

  saveCache();
}

void ResourceXMLRPC::deleteContactFinished( const QValueList<QVariant>&,
                                            const QVariant &id )
{
  clearChange( id.toString() );
  idMapper().removeRemoteId( idMapper().remoteId( id.toString() ) );

  saveCache();
}

void ResourceXMLRPC::fault( int error, const QString &errorMsg,
                            const QVariant& )
{
  QString msg = i18n( "<qt>Server sent error %1: <b>%2</b></qt>" ).arg( error ).arg( errorMsg );
  if ( addressBook() )
    addressBook()->error( msg );

  mSynchronizer->stop();
}

void ResourceXMLRPC::addContactFault( int, const QString &errorMsg,
                                      const QVariant &id )
{
  KABC::Addressee addr = mAddrMap[ id.toString() ];

  mAddrMap.remove( addr.uid() );

  QString msg = i18n( "Unable to add contact %1 to server. (%2)" );
  addressBook()->error( msg.arg( addr.formattedName(), errorMsg ) );
}

void ResourceXMLRPC::updateContactFault( int, const QString &errorMsg,
                                         const QVariant &id )
{
  KABC::Addressee addr = mAddrMap[ id.toString() ];

  QString msg = i18n( "Unable to update contact %1 on server. (%2)" );
  addressBook()->error( msg.arg( addr.formattedName(), errorMsg ) );
}

void ResourceXMLRPC::deleteContactFault( int, const QString &errorMsg,
                                         const QVariant &id )
{
  KABC::Addressee addr;

  const KABC::Addressee::List deletedList = deletedAddressees();
  KABC::Addressee::List::ConstIterator it;
  for ( it = deletedList.begin(); it != deletedList.end(); ++it ) {
    if ( (*it).uid() == id.toString() ) {
      addr = *it;
      break;
    }
  }

  mAddrMap.insert( addr.uid(), addr );

  QString msg = i18n( "Unable to delete contact %1 from server. (%2)" );
  addressBook()->error( msg.arg( addr.formattedName(), errorMsg ) );
}

QString ResourceXMLRPC::addrTypesToTypeStr( int typeMask )
{
  QStringList types;
  QMap<QString, int>::ConstIterator it;
  for ( it = mAddrTypes.begin(); it != mAddrTypes.end(); ++it )
    if ( it.data() & typeMask )
      types.append( it.key() );

  return types.join( ";" );
}

void ResourceXMLRPC::writeContact( const Addressee &addr, QMap<QString, QVariant> &args )
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

  // CATEGORIES
  QStringList::ConstIterator catIt;
  const QStringList categories = addr.categories();

  QMap<QString, QVariant> catMap;
  int counter = 0;
  for ( catIt = categories.begin(); catIt != categories.end(); ++catIt ) {
    QMap<QString, int>::ConstIterator it = mCategoryMap.find( *catIt );
    if ( it == mCategoryMap.end() ) // new category
      catMap.insert( QString::number( counter-- ), *catIt );
    else
      catMap.insert( QString::number( it.data() ), *catIt );
  }
  args.insert( "cat_id", catMap );

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
    args.insert( "email_type", "INTERNET" );
  }

  if ( addr.emails().count() > 1 ) {
    args.insert( "email_home", addr.emails()[ 1 ] );
    args.insert( "email_home_type", "INTERNET" );
  }


  const QStringList customFields = addr.customs();
  QStringList::ConstIterator it;
  for ( it = customFields.begin(); it != customFields.end(); ++it ) {
    int colon = (*it).find( ":" );
    QString identifier = (*it).left( colon );
    int dash = identifier.find( "-" );
    QString app = identifier.left( dash );
    QString name = identifier.mid( dash + 1 );
    QString value = (*it).mid( colon + 1 );
    if ( value.isEmpty() )
      continue;

    if ( app == "XMLRPCResource" )
      args.insert( name, value );
  }

  QString url = KCal::FreeBusyUrlStore::self()->readUrl( addr.preferredEmail() );
  if ( !url.isEmpty() )
    args.insert( "freebusy_url", url );
}

void ResourceXMLRPC::readContact( const QMap<QString, QVariant> &args, Addressee &addr, QString &uid )
{
  Address addrOne, addrTwo;

  QMap<QString, QVariant>::ConstIterator it;
  for ( it = args.begin(); it != args.end(); ++it ) {
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
    } else if ( it.key() == "cat_id" ) {
      const QMap<QString, QVariant> categories = it.data().toMap();
      QMap<QString, QVariant>::ConstIterator it;

      for ( it = categories.begin(); it != categories.end(); ++it )
        addr.insertCategory( it.data().toString() );
    } else if ( it.key() == "rights" ) {
      setRights( addr, it.data().toInt() );
    }
  }

  QMap<QString, QString>::ConstIterator cfIt;
  for ( cfIt = mCustomFieldsMap.begin(); cfIt != mCustomFieldsMap.end(); ++cfIt ) {
    if ( args[ cfIt.key() ].toString().isEmpty() )
      continue;

    if ( cfIt.key() == "freebusy_url" ) {
      KCal::FreeBusyUrlStore::self()->writeUrl( addr.preferredEmail(),
                                                args[ cfIt.key() ].toString() );
      KCal::FreeBusyUrlStore::self()->sync();
    } else
      addr.insertCustom( "XMLRPCResource", cfIt.key(), cfIt.data() );
  }

  if ( !addrOne.isEmpty() )
    addr.insertAddress( addrOne );
  if ( !addrTwo.isEmpty() )
    addr.insertAddress( addrTwo );
}

void ResourceXMLRPC::loadCategoriesFinished( const QValueList<QVariant> &mapList,
                                             const QVariant& )
{
  mCategoryMap.clear();

  const QMap<QString, QVariant> map = mapList[ 0 ].toMap();
  QMap<QString, QVariant>::ConstIterator it;

  KABPrefs *prefs = KABPrefs::instance();
  for ( it = map.begin(); it != map.end(); ++it ) {
    mCategoryMap.insert( it.data().toString(), it.key().toInt() );

    QStringList categories = prefs->customCategories();
    if ( categories.find( it.data().toString() ) == categories.end() )
      categories.append( it.data().toString() );

    prefs->mCustomCategories = categories;
  }
}

void ResourceXMLRPC::loadCustomFieldsFinished( const QValueList<QVariant> &mapList,
                                               const QVariant& )
{
  mCustomFieldsMap.clear();

  const QMap<QString, QVariant> map = mapList[ 0 ].toMap();
  QMap<QString, QVariant>::ConstIterator it;

  for ( it = map.begin(); it != map.end(); ++it )
    mCustomFieldsMap.insert( it.key(), it.data().toString() );
}

#include "kabc_resourcexmlrpc.moc"
