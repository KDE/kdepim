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

#include "folderlister.h"

#include <qapplication.h>

#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include "kabc_opengroupwareprefs.h"

#include "kabc_resourceopengroupware.h"

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

#if 0
  if ( addressBooks().isEmpty() ) {
    kdDebug() << "Retrieving default addressbook list." << endl;
    retrieveAddressBooks();
    writeAddressBooks();
  }
#endif

  KURL url( prefs()->url() );
  url.setUser( prefs()->user() );
  url.setPass( prefs()->password() );

#if 0
  QString query = "?";
  QStringList ids = mPrefs->readAddressBooks();
  QStringList::ConstIterator it;
  for( it = ids.begin(); it != ids.end(); ++it ) {
    if ( it != ids.begin() ) query += "&";
    query += "addressbookid=" + *it;
  }
  url.setQuery( query );
#endif

  kdDebug() << "Download URL: " << url << endl;

#if 0
  mJobData = QString::null;

  mDownloadJob = KPIM::OpenGroupwareJob::getAddressBook( url );
  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotJobResult( KIO::Job * ) ) );
  connect( mDownloadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );
  connect( mDownloadJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
           SLOT( slotJobPercent( KIO::Job *, unsigned long ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(), i18n("Downloading addressbook") );
  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );
#endif

  return true;
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

void ResourceOpenGroupware::slotJobResult( KIO::Job *job )
{
  kdDebug() << "ResourceOpenGroupware::slotJobResult(): " << endl;

  if ( job->error() ) {
    kdError() << job->errorString() << endl;
    emit loadingError( this, job->errorString() );
  } else {
    mAddrMap.clear();
  
    KABC::VCardConverter conv;
    Addressee::List addressees = conv.parseVCards( mJobData );
    Addressee::List::ConstIterator it;
    for( it = addressees.begin(); it != addressees.end(); ++it ) {
      KABC::Addressee addr = *it;
      if ( !addr.isEmpty() ) {
        addr.setResource( this );

        QString remote = addr.custom( "GWRESOURCE", "UID" );
        QString local = idMapper().localId( remote );
        if ( local.isEmpty() ) {
          idMapper().setRemoteId( addr.uid(), remote );
        } else {
          addr.setUid( local );
        }

        insertAddressee( addr );
        clearChange( addr );
      }
    }
  }

  saveCache();

  emit loadingFinished( this );

  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
}

void ResourceOpenGroupware::slotJobData( KIO::Job *, const QByteArray &data )
{
//  kdDebug() << "ResourceOpenGroupware::slotJobData()" << endl;

  mJobData.append( data.data() );
}

void ResourceOpenGroupware::slotJobPercent( KIO::Job *, unsigned long percent )
{
  kdDebug() << "ResourceOpenGroupware::slotJobPercent() " << percent << endl;
  if ( mProgress ) mProgress->setProgress( percent );
}

void ResourceOpenGroupware::cancelLoad()
{
  if ( mDownloadJob ) mDownloadJob->kill();
  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
}

#include "kabc_resourceopengroupware.moc"
