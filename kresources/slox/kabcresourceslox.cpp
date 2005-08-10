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

#include <qapplication.h>

#include <kabc/addressee.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <libkdepim/kpimprefs.h>
#include <libkdepim/progressmanager.h>
#include <kio/davjob.h>

#include "webdavhandler.h"
#include "sloxaccounts.h"
#include "kabcsloxprefs.h"

#include "kabcresourceslox.h"

using namespace KABC;

ResourceSlox::ResourceSlox( const KConfig *config )
  : Resource( config ), SloxBase( this )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config ) {
    readConfig( config );
  }
}

ResourceSlox::ResourceSlox( const KURL &url,
                            const QString &user, const QString &password )
  : Resource( 0 ), SloxBase( this )
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
  mProgress = 0;

  setReadOnly( true );
}

ResourceSlox::~ResourceSlox()
{
  kdDebug() << "KABC::~ResourceSlox()" << endl;

  if ( mDownloadJob ) mDownloadJob->kill();

  delete mPrefs;

  kdDebug() << "KABC::~ResourceSlox() done" << endl;
}

void ResourceSlox::readConfig( const KConfig * )
{
  mPrefs->readConfig();
}

void ResourceSlox::writeConfig( KConfig *config )
{
  kdDebug() << "ResourceSlox::writeConfig() " << endl;
  kdDebug() << mPrefs->url() << endl;

  Resource::writeConfig( config );

  mPrefs->writeConfig();
}

Ticket *ResourceSlox::requestSaveTicket()
{
  if ( !addressBook() ) {
	  kdDebug(5700) << "no addressbook" << endl;
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
}

bool ResourceSlox::load()
{
  kdDebug() << "KABC::ResourceSlox::load()" << endl;

#if 0
  return asyncLoad();
#else
  kdDebug() << "KABC::ResourceSlox::load() is a nop." << endl;
  return true;
#endif
}

bool ResourceSlox::asyncLoad()
{
  kdDebug() << "KABC::ResourceSlox::asyncLoad()" << endl;

  if ( mDownloadJob ) {
    kdWarning() << "KABC::ResourceSlox::asyncLoad(): Loading still in progress."
                << endl;
    return false;
  }

  KURL url = mPrefs->url();
  url.setPath( "/servlet/webdav.contacts/" );
  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement( doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement( doc, root, "prop" );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( LastSync ), "0" );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( FolderId ), mPrefs->folderId() );
  if ( type() == "ox" ) {
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "NEW_AND_MODIFIED" );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "DELETED" );
  } else
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "all" );

  kdDebug() << "REQUEST CONTACTS: \n" << doc.toString( 2 ) << endl;

  mDownloadJob = KIO::davPropFind( url, doc, "0", false );
  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotResult( KIO::Job * ) ) );
  connect( mDownloadJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
           SLOT( slotProgress( KIO::Job *, unsigned long ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n("Downloading contacts") );
  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelDownload() ) );

  return true;
}

void ResourceSlox::slotResult( KIO::Job *job )
{
  kdDebug() << "ResourceSlox::slotResult()" << endl;

  if ( job->error() ) {
    job->showErrorDialog( 0 );
  } else {
    kdDebug() << "ResourceSlox::slotResult() success" << endl;

    QDomDocument doc = mDownloadJob->response();

    mWebdavHandler.log( doc.toString( 2 ) );

    QValueList<SloxItem> items = WebdavHandler::getSloxItems( this, doc );

    bool changed = false;

    QValueList<SloxItem>::ConstIterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
      SloxItem item = *it;
      QString uid = "kresources_slox_kabc_" + item.sloxId;
      if ( item.status == SloxItem::Delete ) {
        QMap<QString,Addressee>::Iterator it;
        it = mAddrMap.find( uid );
        if ( it != mAddrMap.end() ) {
          mAddrMap.remove( it );
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

        mAddrMap.replace( a.uid(), a );

        // TODO: Do we need to try to associate addressees with slox accounts?

        changed = true;
      }
    }
  }

  mDownloadJob = 0;
  mProgress->setComplete();
  mProgress = 0;

  emit loadingFinished( this );
}

void ResourceSlox::parseContactAttribute( const QDomElement &e, Addressee &a )
{
  QString text = decodeText( e.text() );
  if ( text.isEmpty() ) return;
  QString tag = e.tagName();

  if ( tag == fieldName( Birthday ) ) {
    QDateTime dt = WebdavHandler::sloxToQDateTime( text );
    a.setBirthday( dt.date() );
  } else if ( tag == fieldName( Role ) ) {
    a.setRole( text );
  } else if ( tag == "salutation" ) { // what's this in OX?
    a.setPrefix( text );
  } else if ( tag == fieldName( Title ) ) {
    a.setTitle( text );
  } else if ( tag == fieldName( Organization ) ) {
    a.setOrganization( text );
  } else if ( tag == fieldName( FamilyName ) ) {
    a.setFamilyName( text );
  } else if ( tag == fieldName( GivenName) ) {
    a.setGivenName( text );
  } else if ( tag == fieldName( DisplayName ) ) {
    a.setFormattedName( text );
  } else if ( tag == fieldName( PrimaryEmail ) ) {
    a.insertEmail( text, true );
  } else if ( tag == fieldName( WorkPhone1 ) || tag == fieldName( WorkPhone2 ) ) {
    a.insertPhoneNumber( PhoneNumber( text, PhoneNumber::Work ) );
  } else if ( tag == fieldName( WorkMobile1 ) || tag == fieldName( WorkMobile2 ) ) {
    a.insertPhoneNumber( PhoneNumber( text, PhoneNumber::Cell |
                                      PhoneNumber:: Work ) );
  } else if ( tag == fieldName( WorkFax1 ) || tag == fieldName( WorkFax2 ) ) {
    a.insertPhoneNumber( PhoneNumber( text, PhoneNumber::Fax |
                                      PhoneNumber::Work ) );
  } else if ( tag == fieldName( PrivatePhone1 ) || tag == fieldName( PrivatePhone2 ) ) {
    a.insertPhoneNumber( PhoneNumber( text, PhoneNumber::Home ) );
  } else if ( tag == fieldName( PrivateMobile1 ) || tag == fieldName( PrivateMobile2 ) ) {
    a.insertPhoneNumber( PhoneNumber( text, PhoneNumber::Home |
                                      PhoneNumber::Cell ) );
  } else if ( tag == fieldName( PrivateFax1 ) || tag == fieldName( PrivateFax2 ) ) {
    a.insertPhoneNumber( PhoneNumber( text, PhoneNumber::Fax |
                                      PhoneNumber::Home ) );
  } else if ( tag == fieldName( Comment ) ) {
    a.setNote( text );
  } else if ( tag == fieldName( SecondaryEmail1 ) || tag == fieldName( SecondaryEmail2 ) ||
              tag == fieldName( SecondaryEmail3 ) ) {
    a.insertEmail( text );
  } else if ( tag == fieldName( Url ) ) {
    a.setUrl( text );
  } else if ( type() == "ox" ) { // FIXME: Address reading is missing for SLOX
    // read addresses
    Address addr;
    if ( tag.startsWith( "business" ) ) {
      addr = a.address( KABC::Address::Work );
    } else if ( tag.startsWith( "second" ) ) {
      addr = a.address( 0 ); // FIXME: other ??
    } else {
      addr = a.address( KABC::Address::Home );
    }
    if ( tag.endsWith( "street" ) ) {
      addr.setStreet( text );
    } else if ( tag.endsWith( "postal_code" ) ) {
      addr.setPostalCode( text );
    } else if ( tag.endsWith( "city" ) ) {
      addr.setLocality( text );
    } else if ( tag.endsWith( "state" ) ) {
      addr.setRegion( text );
    } else if ( tag.endsWith( "country" ) ) {
      addr.setCountry( text );
    }
    a.insertAddress( addr );
  }
}

bool ResourceSlox::save( Ticket* )
{
  return false; // readonly
}

bool ResourceSlox::asyncSave( Ticket* )
{
  return false; // readonly
}

void ResourceSlox::insertAddressee( const Addressee& addr )
{
  Q_UNUSED( addr );
}

void ResourceSlox::removeAddressee( const Addressee& addr )
{
  Q_UNUSED( addr );
}

void ResourceSlox::slotProgress( KIO::Job *job, unsigned long percent )
{
#if 0
  kdDebug() << "PROGRESS: " << int( job ) << ": " << percent << endl;
#else
  Q_UNUSED( job );
  Q_UNUSED( percent );
#endif
  if ( mProgress ) mProgress->setProgress( percent );
}

void ResourceSlox::cancelDownload()
{
  if ( mDownloadJob ) mDownloadJob->kill();
  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
}

void ResourceSlox::setReadOnly( bool )
{
  KRES::Resource::setReadOnly( true );
}

bool ResourceSlox::readOnly() const
{
  return true;
}

#include "kabcresourceslox.moc"
