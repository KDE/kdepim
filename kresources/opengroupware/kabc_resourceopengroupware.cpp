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
#include "addressbookadaptor.h"
#include "groupwaredownloadjob.h"
#include "groupwareuploadjob.h"

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

  mPrefs = new OpenGroupwarePrefs;
  mFolderLister = new KCal::FolderLister( KCal::FolderLister::AddressBook );

  setType( "opengroupware" );

  mAdaptor = new KPIM::AddressBookAdaptor();
  mAdaptor->setFolderLister( mFolderLister );
  mAdaptor->setDownloadProgressMessage( i18n("Downloading addressbook") );
  mAdaptor->setUser( prefs()->user() );
  mAdaptor->setPassword( prefs()->password() );
  mAdaptor->setIdMapper( &idMapper() );
  mAdaptor->setResource( this );
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

  mBaseUrl = KURL( prefs()->url() );
  mBaseUrl.setUser( prefs()->user() );
  mBaseUrl.setPass( prefs()->password() );
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

  if ( mDownloadJob ) mDownloadJob->kill();
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

  mDownloadJob = new KPIM::GroupwareDownloadJob( mAdaptor );
  connect( mDownloadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
    SLOT( slotDownloadJobResult( KPIM::GroupwareJob * ) ) );

  return true;
}

void ResourceOpenGroupware::slotDownloadJobResult( KPIM::GroupwareJob *job )
{
  kdDebug() << "ResourceOpenGroupware::slotJobResult(): " << endl;

  if ( job->error() ) {
    kdError() << "job failed: " << job->errorString() << endl;
  } else {
    emit loadingFinished( this );
  }

  mDownloadJob = 0;
}

bool ResourceOpenGroupware::save( Ticket *ticket )
{
  return asyncSave( ticket );
}

bool ResourceOpenGroupware::asyncSave( Ticket* )
{
  if ( mUploadJob ) {
    kdWarning() << "Upload still in progress." << endl;
    return false;
  }

  mUploadJob = new KPIM::GroupwareUploadJob( mAdaptor );
  connect( mUploadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
    SLOT( slotUploadJobResult( KPIM::GroupwareJob * ) ) );

  mUploadJob->setBaseUrl( mBaseUrl );

  KABC::Addressee::List::Iterator it;


  QStringList urlsForDeletion;

  KABC::Addressee::List deletedList = deletedAddressees();
  for ( it = deletedList.begin(); it != deletedList.end(); ++it ) {
    KURL url( mBaseUrl );
    url.setPath( (*it).custom( "KCalResourceOpengroupware", "storagelocation" ) );
    urlsForDeletion << url.url();
  }

  mUploadJob->setUrlsForDeletion( urlsForDeletion );


  QStringList dataForUpload;
  KABC::VCardConverter vcard;

  KABC::Addressee::List addedList = addedAddressees();
  for ( it = addedList.begin(); it != addedList.end(); ++it ) {
    dataForUpload << vcard.createVCard( *it );
  }

  KABC::Addressee::List changedList = changedAddressees();
  for ( it = changedList.begin(); it != changedList.end(); ++it ) {
    dataForUpload << vcard.createVCard( *it );
  }

  mUploadJob->setDataForUpload( dataForUpload );


  return true;
}

void ResourceOpenGroupware::slotUploadJobResult( KPIM::GroupwareJob *job )
{
  kdDebug() << "ResourceOpenGroupware::slotJobResult(): " << endl;

  if ( job->error() ) {
    kdError() << "job failed: " << job->errorString() << endl;
  } else {
  }

  mUploadJob = 0;
}


#include "kabc_resourceopengroupware.moc"
