/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <QApplication>

#include <kabc/picture.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <libkdepim/kpimprefs.h>
#include <libkdepim/progressmanager.h>
#include <kio/jobuidelegate.h>
#include <kio/davjob.h>

#include "webdavhandler.h"
#include "sloxaccounts.h"
#include "kabcsloxprefs.h"

#include "kabcresourceslox.h"

using namespace KABC;

ResourceSlox::ResourceSlox()
  : ResourceCached(), SloxBase( this )
{
  init();

  mPrefs->addGroupPrefix( identifier() );
}

ResourceSlox::ResourceSlox( const KConfigGroup &group )
  : ResourceCached( group ), SloxBase( this )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  readConfig( group );
}

ResourceSlox::ResourceSlox( const KUrl &url,
                            const QString &user, const QString &password )
  : ResourceCached(), SloxBase( this )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  mPrefs->setUrl( url.url() );
  mPrefs->setUser( user );
  mPrefs->setPassword( password );
}

void ResourceSlox::init()
{
  mPrefs = new SloxPrefs;
  mWebdavHandler.setResource( this );

  mDownloadJob = 0;
  mUploadJob = 0;
  mDownloadProgress = 0;
  mUploadProgress = 0;

  // phone number mapping for SLOX
  mPhoneNumberSloxMap[PhoneNumber::Work] << "phone" << "phone2";
  mPhoneNumberSloxMap[PhoneNumber::Home] << "privatephone" << "privatephone2";
  mPhoneNumberSloxMap[PhoneNumber::Cell | PhoneNumber::Work] << "mobile" << "mobile2";
  mPhoneNumberSloxMap[PhoneNumber::Cell | PhoneNumber::Home] << "privatemobile" << "privatemobile2";
  mPhoneNumberSloxMap[PhoneNumber::Fax | PhoneNumber::Work] << "fax" << "fax2";
  mPhoneNumberSloxMap[PhoneNumber::Fax | PhoneNumber::Home] << "privatefax" << "privatefax2";

  // phone number mapping for OX (mapping partly taken from Kolab)
  mPhoneNumberOxMap[PhoneNumber::Work] << "phone_business" << "phone_business2";
  mPhoneNumberOxMap[PhoneNumber::Home] << "phone_home" << "phone_home2";
  mPhoneNumberOxMap[PhoneNumber::Cell] << "mobile1"<< "mobile2";
  mPhoneNumberOxMap[PhoneNumber::Fax | PhoneNumber::Work] << "fax_business";
  mPhoneNumberOxMap[PhoneNumber::Fax | PhoneNumber::Home] << "fax_home";
  mPhoneNumberOxMap[PhoneNumber::Fax] << "fax_other";
  mPhoneNumberOxMap[PhoneNumber::Car] << "phone_car";
  mPhoneNumberOxMap[PhoneNumber::Isdn] << "isdn";
  mPhoneNumberOxMap[PhoneNumber::Pager] << "pager";
  mPhoneNumberOxMap[PhoneNumber::Pref] << "primary";
  mPhoneNumberOxMap[PhoneNumber::Voice] << "callback";
  mPhoneNumberOxMap[PhoneNumber::Video] << "radio";
  mPhoneNumberOxMap[PhoneNumber::Bbs] << "tty_tdd";
  mPhoneNumberOxMap[PhoneNumber::Modem] << "telex";
  mPhoneNumberOxMap[PhoneNumber::Pcs] << "phone_assistant";
  mPhoneNumberOxMap[PhoneNumber::Msg] << "phone_company";
}

ResourceSlox::~ResourceSlox()
{
  kDebug() <<"KABC::~ResourceSlox()";

  if ( mDownloadJob ) mDownloadJob->kill();

  delete mPrefs;

  kDebug() <<"KABC::~ResourceSlox() done";
}

void ResourceSlox::readConfig( const KConfigGroup & )
{
  mPrefs->readConfig();
}

void ResourceSlox::writeConfig( KConfigGroup &group )
{
  kDebug() <<"ResourceSlox::writeConfig()";
  kDebug() << mPrefs->url();

  Resource::writeConfig( group );

  mPrefs->writeConfig();
}

Ticket *ResourceSlox::requestSaveTicket()
{
  if ( !addressBook() ) {
	  kDebug(5700) <<"no addressbook";
    return 0;
  }

  return createTicket( this );
}

void ResourceSlox::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceSlox::doOpen()
{
  return true;
}

void ResourceSlox::doClose()
{
  cancelDownload();
  cancelUpload();
}

bool ResourceSlox::load()
{
  kDebug() <<"KABC::ResourceSlox::load()";

#if 0
  return asyncLoad();
#else
  kDebug() <<"KABC::ResourceSlox::load() is a nop.";
  return true;
#endif
}

bool ResourceSlox::asyncLoad()
{
  kDebug() <<"KABC::ResourceSlox::asyncLoad()";

  if ( mDownloadJob ) {
    kDebug() <<"KABC::ResourceSlox::asyncLoad(): Loading still in progress.";
    return true;
  }

  loadFromCache();
  clearChanges();

  KUrl url = mPrefs->url();
  url.setPath( "/servlet/webdav.contacts/" );
  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  QString lastsync = "0";
  if ( mPrefs->useLastSync() ) {
    QDateTime dt = mPrefs->lastSync();
    if ( dt.isValid() )
      lastsync = WebdavHandler::qDateTimeToSlox( dt.addDays( -1 ) );
  }

  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement( doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement( doc, root, "prop" );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( LastSync ), lastsync );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( FolderId ), mPrefs->folderId() );
  if ( type() == "ox" ) {
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "NEW_AND_MODIFIED" );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "DELETED" );
  } else
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "all" );

  kDebug() <<"REQUEST CONTACTS:" << doc.toString( 2 );

  mDownloadJob = KIO::davPropFind( url, doc, "0", KIO::HideProgressInfo );
  connect( mDownloadJob, SIGNAL( result( KJob * ) ),
           SLOT( slotResult( KJob * ) ) );
  connect( mDownloadJob, SIGNAL( percent( KJob *, unsigned long ) ),
           SLOT( slotProgress( KJob *, unsigned long ) ) );

  mDownloadProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n("Downloading contacts") );
  connect( mDownloadProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelDownload() ) );

  mPrefs->setLastSync( QDateTime::currentDateTime() );

  return true;
}

void ResourceSlox::slotResult( KJob *job )
{
  kDebug() <<"ResourceSlox::slotResult()";

  if ( job->error() ) {
    static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
  } else {
    kDebug() <<"ResourceSlox::slotResult() success";

    QDomDocument doc = mDownloadJob->response();

    mWebdavHandler.log( doc.toString( 2 ) );

    QList<SloxItem> items = WebdavHandler::getSloxItems( this, doc );

    bool changed = false;

    QList<SloxItem>::ConstIterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
      SloxItem item = *it;
      QString uid = "kresources_slox_kabc_" + item.sloxId;
      if ( item.status == SloxItem::Delete ) {
        QMap<QString,Addressee>::Iterator it;
        it = mAddrMap.find( uid );
        if ( it != mAddrMap.end() ) {
          mAddrMap.erase( it );
          changed = true;
        }
      } else if ( item.status == SloxItem::Create ) {
        Addressee a;
        a.setUid( uid );

        mWebdavHandler.clearSloxAttributeStatus();

        QDomNode n;
        for( n = item.domNode.firstChild(); !n.isNull(); n = n.nextSibling() ) {
          QDomElement e = n.toElement();
          mWebdavHandler.parseSloxAttribute( e );
          parseContactAttribute( e, a );
        }

        mWebdavHandler.setSloxAttributes( a );

        a.setResource( this );
        a.setChanged( false );

        mAddrMap.insert( a.uid(), a );

        // TODO: Do we need to try to associate addressees with slox accounts?

        changed = true;
      }
    }

    clearChanges();
    saveToCache();
  }

  mDownloadJob = 0;
  mDownloadProgress->setComplete();
  mDownloadProgress = 0;

  emit loadingFinished( this );
}

void ResourceSlox::slotUploadResult( KJob *job )
{
  kDebug() <<"ResourceSlox::slotUploadResult()";

  if ( job->error() ) {
    static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
  } else {
    kDebug() <<"ResourceSlox::slotUploadResult() success";

    QDomDocument doc = mUploadJob->response();
    kDebug() <<"Upload result:";
    kDebug() << doc.toString();

    QList<SloxItem> items = WebdavHandler::getSloxItems( this, doc );

    QList<SloxItem>::ConstIterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
      SloxItem item = *it;
      if ( !item.response.contains( "200" ) ) {
        savingError( this, item.response + '\n' + item.responseDescription );
        continue;
      }
      if ( item.status == SloxItem::New ) {
        QMap<QString,Addressee>::Iterator search_res;
        search_res = mAddrMap.find( item.clientId );
        if ( search_res != mAddrMap.end() ) {
          // use the id provided by the server
          Addressee a = *search_res;
          mAddrMap.erase( search_res );
          a.setUid( "kresources_slox_kabc_" + item.sloxId );
          a.setResource( this );
          a.setChanged( false );
          mAddrMap.insert( a.uid(), a );
          saveToCache();
        }
      }
    }
  }

  clearChange( mUploadAddressee );

  mUploadJob = 0;
  mUploadProgress->setComplete();
  mUploadProgress = 0;

  uploadContacts();
}

void ResourceSlox::parseContactAttribute( const QDomElement &e, Addressee &a )
{
  QString text = decodeText( e.text() );
  if ( text.isEmpty() ) return;
  QString tag = e.tagName();
  KABC::PhoneNumber::Type pnType;

  if ( tag == fieldName( Birthday ) ) {
    QDateTime dt = WebdavHandler::sloxToQDateTime( text );
    a.setBirthday( dt );
  } else if ( tag == fieldName( Role ) ) {
    a.setRole( text );
  } else if ( tag == "salutation" ) { // what's this in OX?
    a.setPrefix( text );
  } else if ( tag == fieldName( Title ) ) {
    a.setTitle( text );
  } else if ( tag == fieldName( Organization ) ) {
    a.setOrganization( text );
  } else if ( tag == fieldName( Department ) ) {
#if KDE_IS_VERSION(3,5,8)
    a.setDepartment( text );
#else
    a.insertCustom( "KADDRESSBOOK", "X-Department", text );
#endif
  } else if ( tag == fieldName( FamilyName ) ) {
    a.setFamilyName( text );
  } else if ( tag == fieldName( GivenName) ) {
    a.setGivenName( text );
  } else if ( tag == fieldName( SecondName ) ) {
    a.setAdditionalName( text );
  } else if ( tag == fieldName( DisplayName ) ) {
    a.setFormattedName( text );
  } else if ( tag == fieldName( Suffix ) ) {
    a.setSuffix( text );
  } else if ( tag == fieldName( PrimaryEmail ) ) {
    a.insertEmail( text, true );
  } else if ( (pnType = phoneNumberType( tag )) ) {
    a.insertPhoneNumber( PhoneNumber( text, pnType ) );
  } else if ( tag == fieldName( Comment ) ) {
    a.setNote( text );
  } else if ( tag == fieldName( SecondaryEmail1 ) || tag == fieldName( SecondaryEmail2 ) ||
              tag == fieldName( SecondaryEmail3 ) ) {
    a.insertEmail( text );
  } else if ( tag == fieldName( Url ) ) {
    a.setUrl( text );
  } else if ( tag == fieldName( Image ) ) {
    QByteArray decodedPicture;
    KCodecs::base64Decode( text.toUtf8(), decodedPicture );
    a.setPhoto( Picture( QImage( decodedPicture.constData() ) ) );
  } else if ( tag == fieldName( NickName ) ) {
    a.setNickName( text );
  } else if ( tag == fieldName( InstantMsg ) ) {
    a.insertCustom( "KADDRESSBOOK", "X-IMAddress", text );
  } else if ( tag == fieldName( Office ) ) {
    a.insertCustom( "KADDRESSBOOK", "X-Office", text );
  } else if ( tag == fieldName( Profession ) ) {
    a.insertCustom( "KADDRESSBOOK", "X-Profession", text );
  } else if ( tag == fieldName( ManagersName ) ) {
    a.insertCustom( "KADDRESSBOOK", "X-ManagersName", text );
  } else if ( tag == fieldName( AssistantsName ) ) {
    a.insertCustom( "KADDRESSBOOK", "X-AssistantsName", text );
  } else if ( tag == fieldName( SpousesName ) ) {
    a.insertCustom( "KADDRESSBOOK", "X-SpousesName", text );
  } else if ( tag == fieldName( Anniversary ) ) {
    QDateTime dt = WebdavHandler::sloxToQDateTime( text );
    a.insertCustom( "KADDRESSBOOK", "X-Anniversary", dt.toString( Qt::ISODate ) );
  } else if ( tag == fieldName( Categories ) ) {
    a.setCategories( text.split( QRegExp(",\\s*") ) );
  } else if ( type() == "ox" ) { // FIXME: Address reading is missing for SLOX
    // read addresses
    Address addr;
    if ( tag.startsWith( fieldName( BusinessPrefix ) ) ) {
      addr = a.address( KABC::Address::Work );
    } else if ( tag.startsWith( fieldName( OtherPrefix ) ) ) {
      addr = a.address( 0 );
    } else {
      addr = a.address( KABC::Address::Home );
    }
    if ( tag.endsWith( fieldName( Street ) ) ) {
      addr.setStreet( text );
    } else if ( tag.endsWith( fieldName( PostalCode ) ) ) {
      addr.setPostalCode( text );
    } else if ( tag.endsWith( fieldName( City ) ) ) {
      addr.setLocality( text );
    } else if ( tag.endsWith( fieldName( State ) ) ) {
      addr.setRegion( text );
    } else if ( tag.endsWith( fieldName( Country ) ) ) {
      addr.setCountry( text );
    }
    a.insertAddress( addr );
  }
}

KABC::PhoneNumber::Type ResourceSlox::phoneNumberType( const QString &fieldName ) const
{
  QMap<KABC::PhoneNumber::Type, QStringList> pnmap;
  if ( type() == "ox" )
    pnmap = mPhoneNumberOxMap;
  else
    pnmap = mPhoneNumberSloxMap;
  QMap<KABC::PhoneNumber::Type, QStringList>::ConstIterator it;
  for ( it = pnmap.begin(); it != pnmap.end(); ++it ) {
    QStringList l = it.value();
    QStringList::ConstIterator it2;
    for ( it2 = l.begin(); it2 != l.end(); ++it2 )
      if ( (*it2) == fieldName )
        return it.key();
  }
  return 0;
}

bool ResourceSlox::save( Ticket* )
{
  kDebug() ;

  if ( readOnly() || !hasChanges() || type() != "ox" ) {
    emit savingFinished( this );
    return true;
  }

  if ( mDownloadJob ) {
    kWarning() <<"download still in progress";
    return false;
  }
  if ( mUploadJob ) {
    kWarning() <<"upload still in progress";
    return false;
  }

  saveToCache();
  uploadContacts();
  return true;
}

bool ResourceSlox::asyncSave( Ticket* )
{
  return false; // readonly
}

void ResourceSlox::uploadContacts()
{
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement( doc, doc, "propertyupdate" );
  QDomElement set = WebdavHandler::addDavElement( doc, root, "set" );
  QDomElement prop = WebdavHandler::addDavElement( doc, set, "prop" );

  bool isDelete = false;

  KABC::Addressee::List addedAddr = addedAddressees();
  KABC::Addressee::List changedAddr = changedAddressees();
  KABC::Addressee::List deletedAddr = deletedAddressees();

  if ( !addedAddr.isEmpty() ) {
    mUploadAddressee = addedAddr.first();
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ClientId ), mUploadAddressee.uid() );
  } else if ( !changedAddr.isEmpty() ) {
    mUploadAddressee = changedAddr.first();
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectId ),
                                   mUploadAddressee.uid().remove( 0, sizeof("kresources_slox_kabc_") - 1) );
  } else if ( !deletedAddr.isEmpty() ) {
    mUploadAddressee = deletedAddr.first();
    isDelete = true;
  } else {
    kDebug() <<"Upload finished.";
    emit savingFinished( this );
    return;
  }

  if ( !isDelete ) {
    createAddresseeFields( doc, prop, mUploadAddressee );
  } else {
    QString tmp_uid = mUploadAddressee.uid().remove( 0, sizeof("kresources_slox_kabc_") - 1); // remove prefix from uid
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectId ), tmp_uid );
    WebdavHandler::addSloxElement( this, doc, prop, "method", "DELETE" );
  }

  kDebug() << doc.toString();

  KUrl url = mPrefs->url();
  url.setPath( "/servlet/webdav.contacts/" );
  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  mUploadJob = KIO::davPropPatch( url, doc, KIO::HideProgressInfo );
  connect( mUploadJob, SIGNAL( result( KJob * ) ),
           SLOT( slotUploadResult( KJob * ) ) );
  connect( mUploadJob, SIGNAL( percent( KJob *, unsigned long ) ),
           SLOT( slotProgress( KJob *, unsigned long ) ) );

  mUploadProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n("Uploading contacts") );
  connect( mUploadProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelUpload() ) );
}

void ResourceSlox::createAddresseeFields( QDomDocument &doc, QDomElement &prop,
                                          const Addressee &a )
{
  // choose addressbook
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( FolderId ), mPrefs->folderId() );

  // person
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( GivenName ), a.givenName() );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( FamilyName ), a.familyName() );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( Title ), a.title() );
  if ( !a.birthday().isNull() )
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( Birthday ),
                                   WebdavHandler::kDateTimeToSlox( KDateTime( a.birthday() ) ) );
  else
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( Birthday ) );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( Role ), a.role() );
#if KDE_IS_VERSION(3,5,8)
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( Department ),
                                 a.department( ) );
#else
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( Department ),
                                 a.custom( "KADDRESSBOOK", "X-Department" ) );
#endif
  if ( type() == "ox" ) { // OX only fields
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( DisplayName ), a.formattedName() );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( SecondName ), a.additionalName() );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( Suffix ), a.suffix() );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( Organization ), a.organization() );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( NickName ), a.nickName() );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( InstantMsg ),
                                   a.custom( "KADDRESSBOOK", "X-IMAddress" ) );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( Office ),
                                   a.custom( "KADDRESSBOOK", "X-Office" ) );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( Profession ),
                                   a.custom( "KADDRESSBOOK", "X-Profession" ) );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ManagersName ),
                                   a.custom( "KADDRESSBOOK", "X-ManagersName" ) );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( AssistantsName ),
                                   a.custom( "KADDRESSBOOK", "X-AssistantsName" ) );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( SpousesName ),
                                   a.custom( "KADDRESSBOOK", "X-SpousesName" ) );
    QString anniversary = a.custom( "KADDRESSBOOK", "X-Anniversary" );
    if ( !anniversary.isEmpty() )
      WebdavHandler::addSloxElement( this, doc, prop, fieldName( Anniversary ),
        WebdavHandler::kDateTimeToSlox( KDateTime::fromString( anniversary ) ) );
    else
      WebdavHandler::addSloxElement( this, doc, prop, fieldName( Anniversary ) );
  }

  WebdavHandler::addSloxElement( this, doc, prop, fieldName( Url ), a.url().url() );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( Comment ), a.note() );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( Categories ), a.categories().join( ", " ) );

  // emails
  QStringList email_list = a.emails();
  QStringList::const_iterator emails_it = email_list.begin();
  if ( emails_it != email_list.end() )
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( PrimaryEmail ), *(emails_it++) );
  if ( emails_it != email_list.end() )
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( SecondaryEmail1 ), *(emails_it++) );
  if ( emails_it != email_list.end() )
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( SecondaryEmail2 ), *(emails_it++) );

  // phone numbers
  PhoneNumber::List pnlist = a.phoneNumbers();
  QMap<KABC::PhoneNumber::Type, QStringList> pnSaveMap;
  if ( type() == "ox" )
    pnSaveMap = mPhoneNumberOxMap;
  else
    pnSaveMap = mPhoneNumberSloxMap;
  for ( PhoneNumber::List::ConstIterator it = pnlist.begin() ; it != pnlist.end(); ++it ) {
    if ( pnSaveMap.contains( (*it).type() ) ) {
      QStringList l = pnSaveMap[(*it).type()];
      QString fn = l.first();
      l.erase( l.begin() );
      if ( !l.isEmpty() )
        pnSaveMap[(*it).type()] = l;
      else
        pnSaveMap.remove( (*it).type() );
      WebdavHandler::addSloxElement( this, doc, prop, fn, (*it).number() );
    } else
      kDebug() <<"Can't save phone number" << (*it).number() <<" of type" << (*it).type();
  }
  // send empty fields for the remaining ohone number fields
  // it's not possible to delete phone numbers otherwise
  for ( QMap<KABC::PhoneNumber::Type, QStringList>::ConstIterator it = pnSaveMap.begin(); it != pnSaveMap.end(); ++it ) {
    QStringList l = it.value();
    for ( QStringList::ConstIterator it2 = l.begin(); it2 != l.end(); ++it2 )
      WebdavHandler::addSloxElement( this, doc, prop, (*it2) );
  }

  // write addresses
  createAddressFields( doc, prop, fieldName( HomePrefix ), a.address( KABC::Address::Home ) );
  if ( type() == "ox" ) {
    createAddressFields( doc, prop, fieldName( BusinessPrefix ), a.address( KABC::Address::Work ) );
    createAddressFields( doc, prop, fieldName( OtherPrefix ), a.address( 0 ) );
  }
}

void KABC::ResourceSlox::createAddressFields( QDomDocument &doc, QDomElement &parent,
                                               const QString &prefix, const KABC::Address &addr )
{
  WebdavHandler::addSloxElement( this, doc, parent, prefix + fieldName( Street ), addr.street() );
  WebdavHandler::addSloxElement( this, doc, parent, prefix + fieldName( PostalCode ), addr.postalCode() );
  WebdavHandler::addSloxElement( this, doc, parent, prefix + fieldName( City ), addr.locality() );
  WebdavHandler::addSloxElement( this, doc, parent, prefix + fieldName( State ), addr.region() );
  WebdavHandler::addSloxElement( this, doc, parent, prefix + fieldName( Country ), addr.country() );
}

void ResourceSlox::slotProgress( KJob *job, unsigned long percent )
{
  if ( mDownloadProgress && job == mDownloadJob )
    mDownloadProgress->setProgress( percent );
  else if ( mUploadProgress && job == mUploadJob )
    mUploadProgress->setProgress( percent );
}

void ResourceSlox::cancelDownload()
{
  if ( mDownloadJob ) mDownloadJob->kill();
  mDownloadJob = 0;
  if ( mDownloadProgress ) mDownloadProgress->setComplete();
  mDownloadProgress = 0;
}

void ResourceSlox::cancelUpload()
{
  if ( mUploadJob ) mUploadJob->kill();
  mUploadJob = 0;
  if ( mUploadProgress ) mUploadProgress->setComplete();
  mUploadProgress = 0;
}

void ResourceSlox::setReadOnly( bool b )
{
  if ( type() == "ox" )
    KABC::Resource::setReadOnly( b );
  else
    KABC::Resource::setReadOnly( true );
}

bool ResourceSlox::readOnly() const
{
  if ( type() == "ox" )
    return KABC::Resource::readOnly();
  else
    return true;
}

#include "kabcresourceslox.moc"
