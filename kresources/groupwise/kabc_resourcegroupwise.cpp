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
#include <qfile.h>

#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include "kabc_groupwiseprefs.h"

#include "kabc_resourcegroupwise.h"

using namespace KABC;

ResourceGroupwise::ResourceGroupwise( const KConfig *config )
  : ResourceCached( config )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config ) {
    readConfig( config );
  }

  initGroupwise();
}

ResourceGroupwise::ResourceGroupwise( const KURL &url,
                                      const QString &user,
                                      const QString &password,
                                      const QStringList &readAddressBooks,
                                      const QString &writeAddressBook )
  : ResourceCached( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  mPrefs->setUrl( url.url() );
  mPrefs->setUser( user );
  mPrefs->setPassword( password );
  mPrefs->setReadAddressBooks( readAddressBooks );
  mPrefs->setWriteAddressBook( writeAddressBook );

  initGroupwise();
}

void ResourceGroupwise::init()
{
  mDownloadJob = 0;
  mProgress = 0;
  mUpdateSystemAddressBook = false;
  mPrefs = new GroupwisePrefs;

  setType( "groupwise" );
}

void ResourceGroupwise::initGroupwise()
{
  mServer = new GroupwiseServer( mPrefs->url(), mPrefs->user(),
                                 mPrefs->password(), this );

  // TODO: find out what this was meant to do.  the ReadAddressBooksJob could cause the server to emit this job when its run() ends
  // connect( mServer, SIGNAL( readAddressBooksFinished() ),
  //         SLOT( loadFinished() ) );
}

ResourceGroupwise::~ResourceGroupwise()
{
  delete mServer;
  mServer = 0;

  delete mPrefs;
  mPrefs = 0;
}

void ResourceGroupwise::readConfig( const KConfig * )
{
  mPrefs->readConfig();

  readAddressBooks();
}

void ResourceGroupwise::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  writeAddressBooks();

  mPrefs->writeConfig();
}

void ResourceGroupwise::clearCache()
{
  idMapper().clear();
  mAddrMap.clear();
  QFile file( cacheFile() );
  file.remove();
}

void ResourceGroupwise::readAddressBooks()
{
  QStringList ids = prefs()->ids();
  QStringList names = prefs()->names();
  QStringList personals = prefs()->personals();
  QStringList frequents = prefs()->frequents();

  if ( ids.count() != names.count() || ids.count() != personals.count() ||
    ids.count() != frequents.count() ) {
    kdError() << "Corrupt addressbook configuration" << endl;
    return;
  }

  mAddressBooks.clear();

  for( uint i = 0; i < ids.count(); ++i ) {
    GroupWise::AddressBook ab;
    ab.id = ids[ i ];
    ab.name = names[ i ];
    ab.isPersonal = personals[ i ] == "1";
    ab.isFrequentContacts = frequents[ i ] == "1";
    
    mAddressBooks.append( ab );
  }
}

void ResourceGroupwise::writeAddressBooks()
{
  QStringList ids;
  QStringList names;
  QStringList personals;
  QStringList frequents;
  GroupWise::AddressBook::List::ConstIterator it;
  for( it = mAddressBooks.begin(); it != mAddressBooks.end(); ++it ) {
    ids.append( (*it).id );
    names.append( (*it).name );
    personals.append( (*it).isPersonal ? "1" : "0" );
    frequents.append( (*it).isFrequentContacts ? "1" : "0" );
  }
  prefs()->setIds( ids );
  prefs()->setNames( names );
  prefs()->setPersonals( personals );
  prefs()->setFrequents( frequents );
}

void ResourceGroupwise::retrieveAddressBooks()
{
  bool firstRetrieve = mAddressBooks.isEmpty();

  GroupwiseServer server( prefs()->url(),
                          prefs()->user(),
                          prefs()->password(), this );

  if ( server.login() )
  {
    mAddressBooks = server.addressBookList();
    server.logout();
  
    if ( firstRetrieve ) {
      QStringList reads;
      QString write; 
      
      GroupWise::AddressBook::List::ConstIterator it;
      for( it = mAddressBooks.begin(); it != mAddressBooks.end(); ++it ) {
        if ( (*it).isPersonal ) {
          reads.append( (*it).id );
          if ( write.isEmpty() ) write = (*it).id;
        }
        else
          prefs()->setSystemAddressBook( (*it).id );
      }
      
      prefs()->setReadAddressBooks( reads );
      prefs()->setWriteAddressBook( write );
    }
  }
  else
    emit loadingError( this, server.error() );
}

Ticket *ResourceGroupwise::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdDebug(5700) << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceGroupwise::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceGroupwise::doOpen()
{
  return true;
}

void ResourceGroupwise::doClose()
{
  kdDebug() << "ResourceGroupwise::doClose()" << endl;

  cancelLoad();
}

bool ResourceGroupwise::load()
{
  return asyncLoad();
}

bool ResourceGroupwise::asyncLoad()
{
  kdDebug() << "ResourceGroupwise::asyncLoad()" << endl;

  if ( mDownloadJob ) {
    kdWarning() << "Download still in progress" << endl;
    return false;
  }

  loadCache();

  if ( addressBooks().isEmpty() ) {
    kdDebug() << "Retrieving default addressbook list." << endl;
    retrieveAddressBooks();
    writeAddressBooks();
  }

  QStringList ids = mPrefs->readAddressBooks(); // start with all the address books

  // check if we are fetching the SAB and we previously fetched it
  if ( ( ids.find( mPrefs->systemAddressBook() ) != ids.end() ) &&
     mPrefs->lastSequenceNumber() != 0 ) 
  {
    kdDebug() << "ResourceGroupwise::asyncLoad() - Found previous sequence number, updating SAB" << endl;
    ids.remove( mPrefs->systemAddressBook() ); // we don't need to read this one again
    mUpdateSystemAddressBook = true;
    if ( ids.isEmpty() )
    {
      kdDebug() << "ResourceGroupwise::asyncLoad() - No user addressbooks specified, just updating SAB" << endl;
      updateAddressBooks();
      mUpdateSystemAddressBook = false;
      return true; 
    }
  }

  KURL url( prefs()->url() );
  if ( url.protocol() == "http" ) 
    url.setProtocol( "groupwise" );
  else 
    url.setProtocol( "groupwises" );

  url.setPath( url.path() + "/addressbook/" );
  url.setUser( prefs()->user() );
  url.setPass( prefs()->password() );

  QString query = "?";
  QStringList::ConstIterator it;
  for( it = ids.begin(); it != ids.end(); ++it ) {
    if ( *it == mPrefs->systemAddressBook() )
      kdDebug() << "fetching SAB" << (ids.size() > 1 ? ", and user addressbooks" : ", and only SAB" ) << endl;
    if ( it != ids.begin() ) query += "&";
    query += "addressbookid=" + *it;
  }
  url.setQuery( query );

  kdDebug() << "Download URL: " << url << endl;

  mJobData = QString::null;

  mDownloadJob = KIO::get( url, false, false );
  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotFetchJobResult( KIO::Job * ) ) );
  connect( mDownloadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );
  connect( mDownloadJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
           SLOT( slotJobPercent( KIO::Job *, unsigned long ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(), i18n("Downloading addressbook") );
  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );

  return true;
}

bool ResourceGroupwise::save( Ticket *ticket )
{
  return asyncSave( ticket );
}

bool ResourceGroupwise::asyncSave( Ticket* )
{
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

  return true;
}

void ResourceGroupwise::slotFetchJobResult( KIO::Job *job )
{
  kdDebug() << "ResourceGroupwise::slotFetchJobResult() " << endl;

  if ( job->error() ) {
    kdError() << job->errorString() << endl;
    emit loadingError( this, job->errorString() );
  } else {
    //mAddrMap.clear(); //ideally we would remove all the contacts from the personal addressbooks and keep the ones from the SAB
    // for the moment we will just have to deal with double removal.
  
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

  // now we fetched addressbooks, if we are using the SAB,  update it with deltas
  QStringList ids = mPrefs->readAddressBooks();
  if ( ids.find( mPrefs->systemAddressBook() ) != ids.end() )
  {
    if ( mUpdateSystemAddressBook )
    {
      kdDebug() << "updating system addressbook" << endl;
      updateAddressBooks();
      mUpdateSystemAddressBook = false;
    }
    else // we just fetched the entire SAB, now get the delta info so we know the last sequence number we have.
    {
      kdDebug() << "fetched whole SAB, now fetching delta info" << endl;
      if ( mServer->login() )
      {
        GroupWise::DeltaInfo deltaInfo = mServer->getDeltaInfo( mPrefs->systemAddressBook() );
        mServer->logout();
    
        kdDebug() << "storing delta info to prefs" << endl;
        mPrefs->setFirstSequenceNumber( deltaInfo.firstSequence );
        mPrefs->setLastSequenceNumber( deltaInfo.lastSequence );
        mPrefs->writeConfig();
      } 
    }
  }
}

void ResourceGroupwise::slotUpdateJobResult( KIO::Job *job )
{
  kdDebug() << "ResourceGroupwise::slotUpdateJobResult() " << endl;

  if ( job->error() ) {
    kdError() << job->errorString() << endl;
    emit loadingError( this, job->errorString() );
  } else {
    KABC::VCardConverter conv;
    Addressee::List addressees = conv.parseVCards( mJobData );
    Addressee::List::ConstIterator it;

    for( it = addressees.begin(); it != addressees.end(); ++it ) {
      KABC::Addressee addr = *it;
      if ( !addr.isEmpty() ) {
#if 1 // until we figure out how to get complete changes...
        // switch between added, deleted and modified
        // if added or changed
        QString syncType = addr.custom( "GWRESOURCE", "SYNC" );
        QString remote = addr.custom( "GWRESOURCE", "UID" );
        QString local = idMapper().localId( remote );

        if ( syncType == "ADD" || syncType == "UPD" )
        {
          addr.setResource( this );
            if ( local.isEmpty() ) {
            idMapper().setRemoteId( addr.uid(), remote );
          } else {
            addr.setUid( local );
          }
  
          insertAddressee( addr );
          clearChange( addr );
        }
        else if ( syncType == "DEL" )
        {
          // if deleted
          if ( !remote.isEmpty() )
          {
            if ( !local.isEmpty() )
            {
              idMapper().removeRemoteId( remote );
              KABC::Addressee addrToDelete = findByUid( local );
              removeAddressee( addrToDelete );
            }
          }
          else 
            kdError() << "Addressee to delete did not have a remote UID, unable to find the corresponding local contact" << endl;
        }
#else
        addr.dump();
#endif
      }
    }
  }
  saveCache();

  emit loadingFinished( this );

  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;

}

bool ResourceGroupwise::updateAddressBooks()
{
  kdDebug() << "ResourceGroupwise::updateAddressBooks() - Updating address books." << endl;
    
  if ( mDownloadJob ) {
    kdWarning() << "Download still in progress" << endl;
    return false;
  }

  if ( addressBooks().isEmpty() ) {
    kdDebug() << "Retrieving default addressbook list." << endl;
    retrieveAddressBooks();
    writeAddressBooks();
  }

  QStringList ids;
  ids.append( mPrefs->systemAddressBook() );

  KURL url( prefs()->url() );
  if ( url.protocol() == "http" ) 
    url.setProtocol( "groupwise" );
  else 
    url.setProtocol( "groupwises" );

  url.setPath( url.path() + "/addressbook/" );
  url.setUser( prefs()->user() );
  url.setPass( prefs()->password() );

  QString query = "?";
  QStringList::ConstIterator it;
  for( it = ids.begin(); it != ids.end(); ++it ) {
    if ( it != ids.begin() ) query += "&";
    query += "addressbookid=" + *it;
  }
  query += "&update=true";
  query += QString::fromLatin1( "&lastSeqNo=%1" ).arg( mPrefs->lastSequenceNumber() );

  url.setQuery( query );

  kdDebug() << "Update URL: " << url << endl;

  mJobData = QString::null;

  mDownloadJob = KIO::get( url, false, false );
  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotUpdateJobResult( KIO::Job * ) ) );
  connect( mDownloadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );
  connect( mDownloadJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
           SLOT( slotJobPercent( KIO::Job *, unsigned long ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(), i18n("Updating System Address Book") );
  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );

  return true;
}

void ResourceGroupwise::slotJobData( KIO::Job *, const QByteArray &data )
{
//  kdDebug() << "ResourceGroupwise::slotJobData()" << endl;

  mJobData.append( data.data() );
}

void ResourceGroupwise::slotJobPercent( KIO::Job *, unsigned long percent )
{
  kdDebug() << "ResourceGroupwise::slotJobPercent() " << percent << endl;
  if ( mProgress ) mProgress->setProgress( percent );
}

void ResourceGroupwise::cancelLoad()
{
  if ( mDownloadJob ) mDownloadJob->kill();
  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
}


#include "kabc_resourcegroupwise.moc"
