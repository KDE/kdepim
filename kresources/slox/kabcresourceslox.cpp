/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <qapplication.h>

#include <kabc/addressee.h>
#include <kaddressbook/kabprefs.h>
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
  : Resource( config )
{
  init();

  if ( config ) {
    readConfig( config );
  }

  initSlox();
}

ResourceSlox::ResourceSlox( const KURL &url,
                            const QString &user, const QString &password )
  : Resource( 0 )
{
  init();

  mPrefs->setUrl( url.url() );
  mPrefs->setUser( user );
  mPrefs->setPassword( password );

  initSlox();
}

void ResourceSlox::init()
{
  mPrefs = new SloxPrefs;

  setType( "slox" );

  mDownloadJob = 0;
  mProgress = 0;
  
  setReadOnly( true );
}

void ResourceSlox::initSlox()
{
  SloxAccounts::setServer( KURL( mPrefs->url() ).host() );

  SloxAccounts::self();
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
  WebdavHandler::addSloxElement( doc, prop, "lastsync", "0" );
  WebdavHandler::addSloxElement( doc, prop, "folderid" );
  WebdavHandler::addSloxElement( doc, prop, "objecttype", "all" );

  kdDebug() << "REQUEST CONTACTS: \n" << doc.toString( 2 ) << endl;

  mDownloadJob = KIO::davPropFind( url, doc, "0", false );
  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotResult( KIO::Job * ) ) );
  connect( mDownloadJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
           SLOT( slotProgress( KIO::Job *, unsigned long ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n("Downloading contacts") );
  connect( mProgress,
           SIGNAL( progressItemCanceled( ProgressItem * ) ),
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

    QValueList<SloxItem> items = WebdavHandler::getSloxItems( doc );

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
  // FIXME: Why is the text still UTF8 encoded?
  QString text = QString::fromUtf8( e.text().latin1() );
  if ( text.isEmpty() ) return;
  QString tag = e.tagName();

  if ( tag == "birthday" ) {
    QDateTime dt = WebdavHandler::sloxToQDateTime( text );
    a.setBirthday( dt.date() );
  } else if ( tag == "position" ) {
    a.setRole( text );
  } else if ( tag == "salutation" ) {
    a.setPrefix( text );
  } else if ( tag == "title" ) {
    a.setTitle( text );
  } else if ( tag == "department" ) {
    a.setOrganization( text );
  } else if ( tag == "lastname" ) {
    a.setFamilyName( text );
  } else if ( tag == "firstname" ) {
    a.setGivenName( text );
  } else if ( tag == "email" ) {
    a.insertEmail( text, true );
  } else if ( tag == "phone" || tag == "phone2" ) {
    a.insertPhoneNumber( PhoneNumber( text, PhoneNumber::Work ) );
  } else if ( tag == "mobile" || tag == "mobile2" ) {
    a.insertPhoneNumber( PhoneNumber( text, PhoneNumber::Cell |
                                      PhoneNumber:: Work ) );
  } else if ( tag == "fax" || tag == "fax2" ) {
    a.insertPhoneNumber( PhoneNumber( text, PhoneNumber::Fax |
                                      PhoneNumber::Work ) );
  } else if ( tag == "privatephone" || tag == "privatephone2" ) {
    a.insertPhoneNumber( PhoneNumber( text, PhoneNumber::Home ) );
  } else if ( tag == "privatemobile" || tag == "privatemobile2" ) {
    a.insertPhoneNumber( PhoneNumber( text, PhoneNumber::Home |
                                      PhoneNumber::Cell ) );
  } else if ( tag == "privatefax" || tag == "privatefax2" ) {
    a.insertPhoneNumber( PhoneNumber( text, PhoneNumber::Fax |
                                      PhoneNumber::Home ) );
  } else if ( tag == "comment" ) {
    a.setNote( text );
  } else if ( tag == "email2" || tag == "privateemail" ||
              tag == "privateemail2" ) {
    a.insertEmail( text );
  } else if ( tag == "privateurl" ) {
    a.setUrl( text );
  }
  // FIXME: Read addresses
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
