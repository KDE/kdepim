/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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

#include "kabc_resourceopengroupware.h"

#include "folderlister.h"
#include "webdavhandler.h"
#include "kabc_opengroupwareprefs.h"

#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kio/davjob.h>

#include <qapplication.h>
#include <qdom.h>

using namespace KABC;

ResourceOpenGroupware::ResourceOpenGroupware( const KConfig *config )
  : ResourceCached( config )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config ) {
    readConfig( config );
  }
}

ResourceOpenGroupware::ResourceOpenGroupware( const KURL &url,
                                      const QString &user,
                                      const QString &password )
  : ResourceCached( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  mPrefs->setUrl( url.url() );
  mPrefs->setUser( user );
  mPrefs->setPassword( password );
}

void ResourceOpenGroupware::init()
{
  mDownloadJob = 0;
  mProgress = 0;

  mPrefs = new OpenGroupwarePrefs;
  mFolderLister = new KCal::FolderLister( KCal::FolderLister::AddressBook );

  setType( "OpenGroupware" );
}

ResourceOpenGroupware::~ResourceOpenGroupware()
{
  delete mPrefs;
  mPrefs = 0;
}

void ResourceOpenGroupware::readConfig( const KConfig *config )
{
  mPrefs->readConfig();

  mFolderLister->readConfig( config );
}

void ResourceOpenGroupware::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  mPrefs->writeConfig();

  mFolderLister->writeConfig( config );
}

Ticket *ResourceOpenGroupware::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdDebug(5700) << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceOpenGroupware::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceOpenGroupware::doOpen()
{
  return true;
}

void ResourceOpenGroupware::doClose()
{
  kdDebug() << "ResourceOpenGroupware::doClose()" << endl;

  cancelLoad();
}

bool ResourceOpenGroupware::load()
{
  return asyncLoad();
}

bool ResourceOpenGroupware::asyncLoad()
{
  if ( mDownloadJob ) {
    kdWarning() << "Download still in progress" << endl;
    return false;
  }

  mAddrMap.clear();
  loadCache();

  mFoldersForDownload = mFolderLister->activeFolderIds();
  
  mItemsForDownload.clear();

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(), i18n("Downloading addressbook") );
  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );

  listItems();

  return true;
}

void ResourceOpenGroupware::listItems()
{
  if ( mFoldersForDownload.isEmpty() ) {
    if ( mProgress ) {
      mProgress->setTotalItems( mItemsForDownload.count() );
      mProgress->setCompletedItems( 1 );
      mProgress->updateProgress();
    }
    downloadItem();
  } else {
    QDomDocument props = WebdavHandler::createItemsAndVersionsPropsRequest();

    KURL url = mFoldersForDownload.front();
    mFoldersForDownload.pop_front();

    url.setUser( prefs()->user() );
    url.setPass( prefs()->password() );

    kdDebug() << "OpenGroupware::listItems(): " << url << endl;

    mListEventsJob = KIO::davPropFind( url, props, "1", false );

    connect( mListEventsJob, SIGNAL( result( KIO::Job * ) ),
             SLOT( slotListJobResult( KIO::Job * ) ) );
  }
}

void ResourceOpenGroupware::slotListJobResult( KIO::Job *job )
{
  kdDebug() << "ResourceOpenGroupware::slotListJobResult(): " << endl;

  if ( job->error() ) {
    kdError() << "Unable to list folders: " << job->errorString() << endl;
    if ( mProgress ) {
      mProgress->setComplete();
      mProgress = 0;
    }
  } else {
    QDomDocument doc = mListEventsJob->response();

    //kdDebug(7000) << " Doc: " << doc.toString() << endl;

    //kdDebug(7000) << idMapper().asString() << endl;

    QDomNodeList entries = doc.elementsByTagNameNS( "DAV:", "href" );

    QDomNodeList fingerprints = doc.elementsByTagNameNS( "DAV:", "getetag" );
    int c = entries.count();
    int i = 0;
    while ( i < c ) {
      QDomNode node = entries.item( i );
      QDomElement e = node.toElement();
      const QString &entry = e.text();
      mItemsForDownload << entry;
      
      kdDebug() << "ITEM: " << entry << endl;
      
      i++;
    }
  }
  mListEventsJob = 0;

  listItems();
}


void ResourceOpenGroupware::downloadItem()
{
  if ( !mItemsForDownload.isEmpty() ) {
    const QString entry = mItemsForDownload.front();
    mItemsForDownload.pop_front();

    KURL url( entry );
    url.setProtocol( "webdav" );
    url.setUser( mPrefs->user() );
    url.setPass( mPrefs->password() );

    mJobData = QString::null;

    mDownloadJob = KIO::get( url, false, false );
    connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
        SLOT( slotJobResult( KIO::Job * ) ) );
    connect( mDownloadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
        SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );
  } else {
    if ( mProgress ) mProgress->setComplete();
    mProgress = 0;
    emit loadingFinished( this );
  }
}

void ResourceOpenGroupware::slotJobResult( KIO::Job *job )
{
  kdDebug() << "ResourceOpenGroupware::slotJobResult(): " << endl;

  if ( job->error() ) {
    kdError() << "job failed: " << job->errorString() << endl;
  } else {
    KABC::VCardConverter conv;
    Addressee::List addressees = conv.parseVCards( mJobData );
    Addressee::List::ConstIterator it;
    for( it = addressees.begin(); it != addressees.end(); ++it ) {
      KABC::Addressee addr = *it;
      if ( !addr.isEmpty() ) {
        addr.setResource( this );
        insertAddressee( addr );
      }
    }
  }

  if ( mProgress ) {
    mProgress->incCompletedItems();
    mProgress->updateProgress();
  }
  mJobData = QString::null;
  mDownloadJob = 0;

  downloadItem();
}

bool ResourceOpenGroupware::save( Ticket *ticket )
{
  return asyncSave( ticket );
}

bool ResourceOpenGroupware::asyncSave( Ticket* )
{
#if 0
  if ( !mServer->login() ) return false;

  KABC::Addressee::List::Iterator it;

  KABC::Addressee::List addedList = addedAddressees();
  for ( it = addedList.begin(); it != addedList.end(); ++it ) {
    if ( mServer->insertAddressee( mPrefs->writeAddressBook(), *it ) ) {
      clearChange( *it );
      idMapper().setRemoteId( (*it).uid(), (*it).custom( "GWRESOURCE", "UID" ) );
    }
  }

  KABC::Addressee::List changedList = changedAddressees();
  for ( it = changedList.begin(); it != changedList.end(); ++it ) {
    if ( mServer->changeAddressee( *it ) )
      clearChange( *it );
  }

  KABC::Addressee::List deletedList = deletedAddressees();
  for ( it = deletedList.begin(); it != deletedList.end(); ++it ) {
    if ( mServer->removeAddressee( *it ) )
      clearChange( *it );
  }

  saveCache();

  mServer->logout();
#endif

  return true;
}

void ResourceOpenGroupware::slotJobData( KIO::Job *, const QByteArray &data )
{
//  kdDebug() << "ResourceOpenGroupware::slotJobData()" << endl;

  mJobData.append( data.data() );
}

void ResourceOpenGroupware::cancelLoad()
{
  if ( mDownloadJob ) mDownloadJob->kill();
  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
}

#include "kabc_resourceopengroupware.moc"
