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

#include "kabc_resourcegroupwarebase.h"
#include "kabc_groupwareprefs.h"

#include "folderlister.h"
#include "addressbookadaptor.h"
#include "groupwaredownloadjob.h"
#include "groupwareuploadjob.h"

#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#include <klocale.h>

using namespace KABC;

ResourceGroupwareBase::ResourceGroupwareBase( const KConfig *config )
  : ResourceCached( config ),
    mPrefs(0), mFolderLister(0), mAdaptor(0), mDownloadJob(0), mUploadJob(0)
{
}

ResourceGroupwareBase::ResourceGroupwareBase( const KURL &url,
                                      const QString &user,
                                      const QString &password )
  : ResourceCached( 0 ),
    mPrefs(0), mFolderLister(0), mAdaptor(0), mDownloadJob(0), mUploadJob(0)
{
  mBaseUrl = url;
  mBaseUrl.setUser( user );
  mBaseUrl.setPass( password );
}

ResourceGroupwareBase::~ResourceGroupwareBase()
{
  delete mPrefs;
  mPrefs = 0;
}

KPIM::GroupwareDownloadJob *ResourceGroupwareBase::createDownloadJob( AddressBookAdaptor *adaptor )
{
  return new KPIM::GroupwareDownloadJob( adaptor );
}

KPIM::GroupwareUploadJob *ResourceGroupwareBase::createUploadJob( AddressBookAdaptor *adaptor )
{
  return new KPIM::GroupwareUploadJob( adaptor );
}

void ResourceGroupwareBase::setPrefs( GroupwarePrefsBase *prefs ) 
{
  if ( !prefs ) return;
  if ( mPrefs ) delete mPrefs;
  mPrefs = prefs;
  mPrefs->addGroupPrefix( identifier() );
}

void ResourceGroupwareBase::setFolderLister( KPIM::FolderLister *folderLister )
{
  if ( !folderLister ) return;
  if ( mFolderLister ) delete mFolderLister;
  mFolderLister = folderLister;
  if ( mAdaptor ) mAdaptor->setFolderLister( mFolderLister );
}

void ResourceGroupwareBase::setAdaptor( AddressBookAdaptor *adaptor )
{
  if ( !adaptor ) return;
  if ( mAdaptor ) delete mAdaptor;
  mAdaptor = adaptor;
  mAdaptor->setFolderLister( mFolderLister );
  mAdaptor->setDownloadProgressMessage( i18n("Downloading addressbook") );
  mAdaptor->setUploadProgressMessage( i18n("Uploading addressbook") );
  if ( prefs() ) {
    mAdaptor->setUser( prefs()->user() );
    mAdaptor->setPassword( prefs()->password() );
  }
  mAdaptor->setIdMapper( &idMapper() );
  mAdaptor->setResource( this );
}

void ResourceGroupwareBase::init()
{
  mDownloadJob = 0;
}

GroupwarePrefsBase *ResourceGroupwareBase::createPrefs()
{
  return new GroupwarePrefsBase();
}



void ResourceGroupwareBase::readConfig( const KConfig *config )
{
  mPrefs->readConfig();

  mFolderLister->readConfig( config );

  mBaseUrl = KURL( prefs()->url() );
  mBaseUrl.setUser( prefs()->user() );
  mBaseUrl.setPass( prefs()->password() );
}

void ResourceGroupwareBase::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  mPrefs->writeConfig();

  mFolderLister->writeConfig( config );
}

Ticket *ResourceGroupwareBase::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdDebug(5700) << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceGroupwareBase::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceGroupwareBase::doOpen()
{
  return true;
}

void ResourceGroupwareBase::doClose()
{
  kdDebug(5800) << "ResourceGroupwareBase::doClose()" << endl;

  if ( mDownloadJob ) mDownloadJob->kill();
}

bool ResourceGroupwareBase::load()
{
  return asyncLoad();
}

bool ResourceGroupwareBase::asyncLoad()
{
  if ( mDownloadJob ) {
    kdWarning() << "Download still in progress" << endl;
    return false;
  }

  mAddrMap.clear();
  loadCache();

  mDownloadJob = createDownloadJob( mAdaptor );
  connect( mDownloadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
    SLOT( slotDownloadJobResult( KPIM::GroupwareJob * ) ) );

  return true;
}

void ResourceGroupwareBase::slotDownloadJobResult( KPIM::GroupwareJob *job )
{
  kdDebug(5800) << "ResourceGroupwareBase::slotJobResult(): " << endl;

  if ( job->error() ) {
    kdError() << "job failed: " << job->errorString() << endl;
  } else {
    emit loadingFinished( this );
  }

  mDownloadJob = 0;
}

bool ResourceGroupwareBase::save( Ticket *ticket )
{
  return asyncSave( ticket );
}

bool ResourceGroupwareBase::asyncSave( Ticket* )
{
  if ( mUploadJob ) {
    kdWarning() << "Upload still in progress." << endl;
    return false;
  }

  mUploadJob = createUploadJob( mAdaptor );
  connect( mUploadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
    SLOT( slotUploadJobResult( KPIM::GroupwareJob * ) ) );
  mUploadJob->setBaseUrl( mBaseUrl );

  KABC::Addressee::List addr;
  KABC::Addressee::List::Iterator it;
  KPIM::GroupwareUploadItem::List addedItems, changedItems, deletedItems;

  addr = addedAddressees();
  for( it = addr.begin(); it != addr.end(); ++it ) {
    addedItems.append( adaptor()->newUploadItem( *it, KPIM::GroupwareUploadItem::Added ) );
  }
  // TODO: Check if the item has changed on the server...
  // In particular, check if the version we based our change on is still current 
  // on the server
  addr = changedAddressees();
  for( it = addr.begin(); it != addr.end(); ++it ) {
    changedItems.append( adaptor()->newUploadItem( *it, KPIM::GroupwareUploadItem::Changed ) );
  }
  addr = deletedAddressees();
  for( it = addr.begin(); it != addr.end(); ++it ) {
    deletedItems.append( adaptor()->newUploadItem( *it, KPIM::GroupwareUploadItem::Deleted ) );
  }

  mUploadJob->setAddedItems( addedItems );
  mUploadJob->setChangedItems( changedItems );
  mUploadJob->setDeletedItems( deletedItems );

  return true;
}

void ResourceGroupwareBase::slotUploadJobResult( KPIM::GroupwareJob *job )
{
  kdDebug(5800) << "ResourceGroupwareBase::slotJobResult(): " << endl;

  if ( job->error() ) {
    kdError() << "job failed: " << job->errorString() << endl;
  } else {
  }

  mUploadJob = 0;
}


#include "kabc_resourcegroupwarebase.moc"
