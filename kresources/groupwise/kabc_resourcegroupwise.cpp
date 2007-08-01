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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
  mJob = 0;
  mProgress = 0;
  mSABProgress = 0;
  mUABProgress = 0;
  mServerFirstSequence = 0;
  mServerLastSequence = 0;
  mServerLastPORebuildTime = 0;
  mPrefs = new GroupwisePrefs;
  mState = Start;
  setType( "groupwise" );
}

void ResourceGroupwise::initGroupwise()
{
  mServer = new GroupwiseServer( mPrefs->url(), mPrefs->user(),
                                 mPrefs->password(), this );
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

    QStringList reads;
    QString write; 

    GroupWise::AddressBook::List::ConstIterator it;
    for( it = mAddressBooks.begin(); it != mAddressBooks.end(); ++it ) {
      reads.append( (*it).id );
      if ( (*it).isPersonal ) {
        if ( write.isEmpty() ) write = (*it).id;
      }
      else
        prefs()->setSystemAddressBook( (*it).id );
    }

    if ( firstRetrieve ) {
      prefs()->setReadAddressBooks( reads );
      prefs()->setWriteAddressBook( write );
    }
  }
  else
    emit loadingError( this, server.errorText() );
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

  if ( appIsWhiteListedForSAB() )
  saveCache();

  mServer->logout();

  return true;
}

bool ResourceGroupwise::load()
{
  return asyncLoad();
}

bool ResourceGroupwise::asyncLoad()
{
  kdDebug() << "ResourceGroupwise::asyncLoad()" << endl;
  //mPrefs->readConfig();  TODO: remove if the system addressbook is not read when disabled in config

  if ( mState != Start )
  {
     kdDebug() << "  Download still in progress" << endl;
     return true;
  }

  if ( appIsWhiteListedForSAB() )
    loadCache();

  if ( !mProgress )
  {
    mProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n( "Loading GroupWise resource %1" ).arg( resourceName() ), QString::null, true /*CanBeCancelled*/, mPrefs->url().startsWith("https" ) );
    connect( mProgress, SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
               SLOT( cancelLoad() ) );
  }

  if ( addressBooks().isEmpty() ) {
    kdDebug() << "  Retrieving default addressbook list." << endl;
    retrieveAddressBooks();
    writeAddressBooks();
  }

  SABState sabState = systemAddressBookState();
  if ( shouldFetchSystemAddressBook() )
  {
    if ( sabState == RefreshNeeded )
    {
      kdDebug() << "  Fetching system addressbook" << endl;
      fetchAddressBooks( System );
      return true;
    }
    else if ( sabState == Stale )
    {
      kdDebug() << "  Updating system addressbook" << endl;
      updateSystemAddressBook(); // we then fetch the user address books after doing this
      return true;
    }
  }
  else if ( shouldFetchUserAddressBooks() )
  {
    kdDebug() << "  Fetching user addressbook" << endl;
    fetchAddressBooks( User );
    return true;
  }
  return true;
}

void ResourceGroupwise::fetchAddressBooks( const BookType bookType )
{
  KURL url = createAccessUrl( bookType, Fetch );
  if ( !url.isValid() )
    return;

  kdDebug() << k_funcinfo << ( bookType == System ? " System" : " User" ) << " URL: " << url << endl;
  // sanity check
  if ( bookType == User && !( mState == SABUptodate || mState == Start ) )
  {
    kdDebug() << "  **ERROR** - fetchAddressBooks( User ) called when SAB not up to date" << endl;
    return;
  }

  mState = ( bookType == System ? FetchingSAB : FetchingUAB );
  mJobData = QString::null;

  if ( mJob )
  {
    kdDebug() << "  **ERROR** - fetchAddressBooks() called when a job was already running!" << endl;
    return;
  }

  mJob = KIO::get( url, false, false );  // TODO: make the GW jobs call finished if the URL 
                                         // contains no address book IDs
  kdDebug() << "  Job address: " << mJob << endl;
  connect( mJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotReadJobData( KIO::Job *, const QByteArray & ) ) );
  connect( mJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
           SLOT( slotJobPercent( KIO::Job *, unsigned long ) ) );

  if ( bookType == System )
  {
    connect( mJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( fetchSABResult( KIO::Job * ) ) );
    mSABProgress = KPIM::ProgressManager::instance()->createProgressItem(
        mProgress, KPIM::ProgressManager::getUniqueID(),
        i18n( "Fetching System Address Book" ), QString::null,
        false /*CannotBeCancelled*/,
        mPrefs->url().startsWith("https" ) );
  }
  else
  {
    connect( mJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( fetchUABResult( KIO::Job * ) ) );
    mUABProgress = KPIM::ProgressManager::instance()->createProgressItem(
        mProgress, KPIM::ProgressManager::getUniqueID(),
        i18n( "Fetching User Address Books" ), QString::null,
        false /*CannotBeCancelled*/,
        mPrefs->url().startsWith("https" ) );
  }

  return;
}

void ResourceGroupwise::fetchSABResult( KIO::Job *job )
{
  kdDebug() << "ResourceGroupwise::fetchSABResult() " << endl;

  if ( job->error() ) {
    kdError() << job->errorString() << endl;
    emit loadingError( this, job->errorString() );
    // TODO kill the rest of the load sequence as well
  }

  mJob->disconnect( this );
  mJob = 0;
  mState = SABUptodate;
  if ( mSABProgress )
    mSABProgress->setComplete();

  storeDeltaInfo();

  if ( shouldFetchUserAddressBooks() )
    fetchAddressBooks( User );
  else
    loadCompleted();
}

void ResourceGroupwise::fetchUABResult( KIO::Job *job )
{
  kdDebug() << "ResourceGroupwise::fetchUABResult() " << endl;

  if ( job->error() ) {
    kdError() << job->errorString() << endl;
    emit loadingError( this, job->errorString() );
  }

  mJob->disconnect( this );
  mJob = 0;
  mState = Uptodate;
  if ( mUABProgress )
    mUABProgress->setComplete();
  loadCompleted();
}

void ResourceGroupwise::updateSystemAddressBook()
{
  kdDebug() << "ResourceGroupwise::updateSystemAddressBook()" << endl;

  if ( mState != Start ) {
    kdWarning() << "  Action already in progress" << endl;
    return;
  }

  if ( addressBooks().isEmpty() ) {
    kdDebug() << "  Retrieving default addressbook list." << endl;
    retrieveAddressBooks();
    writeAddressBooks();
  }

  KURL url = createAccessUrl( System, Update, mPrefs->lastSequenceNumber(), mPrefs->lastTimePORebuild() );
  kdDebug() << "  Update URL: " << url << endl;

  mJobData = QString::null;
  mSABProgress = KPIM::ProgressManager::instance()->createProgressItem(
      mProgress, KPIM::ProgressManager::getUniqueID(),
      i18n( "Updating System Address Book" ), QString::null,
      false /*CannotBeCancelled*/,
      mPrefs->url().startsWith("https" ) );

  mJob = KIO::get( url, false, false );
  mJob->setInteractive( false );
  connect( mJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( updateSABResult( KIO::Job * ) ) );
  connect( mJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotUpdateJobData( KIO::Job *, const QByteArray & ) ) );
  connect( mJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
           SLOT( slotJobPercent( KIO::Job *, unsigned long ) ) );

  return;
}

void ResourceGroupwise::updateSABResult( KIO::Job *job )
{
  kdDebug() << "ResourceGroupwise::updateSABResult() " << endl;

  mSABProgress->setComplete();
  mSABProgress = 0;
  mJob->disconnect( this );
  mJob = 0;

  int errorCode = job->error();
  if ( errorCode != 0 ) {
    if ( errorCode == KIO::ERR_NO_CONTENT ) // we need to refresh the SAB
    {
      kdDebug() << "  update SAB failed, fetching all of it again" << endl;
      mPrefs->setLastSequenceNumber( 0 );
      mPrefs->setFirstSequenceNumber( 0 );
      fetchAddressBooks( System );
      return;
    }
  }

  mState = SABUptodate;
  storeDeltaInfo();

  if ( shouldFetchUserAddressBooks() )
    fetchAddressBooks( User );
  else
    loadCompleted();
}

void ResourceGroupwise::slotReadJobData( KIO::Job *job , const QByteArray &data )
{
  kdDebug() << "ResourceGroupwise::slotReadJobData()" << endl;
  Q_UNUSED( job );

  mJobData.append( data.data() );

  KABC::VCardConverter conv;
  QTime profile;
  profile.start();
  Addressee::List addressees = conv.parseVCards( mJobData );
 // kdDebug() << "  parsed " << addressees.count() << " contacts in "  << profile.elapsed() << "ms, now adding to resource..." << endl;

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
  mJobData = QString::null;
}

void ResourceGroupwise::slotUpdateJobData( KIO::Job *job, const QByteArray &data )
{
  kdDebug() << "ResourceGroupwise::slotUpdateJobData()" << endl;
  kdDebug() << "  Job address: " << job << endl;
  KABC::VCardConverter conv;
  mJobData.append( data.data() );

  Addressee::List addressees = conv.parseVCards( mJobData );
  Addressee::List::ConstIterator it;

  for( it = addressees.begin(); it != addressees.end(); ++it ) {
    KABC::Addressee addr = *it;
    if ( !addr.isEmpty() ) {
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
    }
  }
  mJobData = QString::null;
}

void ResourceGroupwise::loadCompleted()
{
  kdDebug() << "ResourceGroupwise::loadCompleted()" << endl;
  if ( mProgress )
    mProgress->setComplete();
  mProgress = 0;
  mSABProgress = 0;
  mUABProgress = 0;
  mState = Start;
  if ( appIsWhiteListedForSAB() )
    saveCache();
  emit loadingFinished( this );
  addressBook()->emitAddressBookChanged();
}

void ResourceGroupwise::slotJobPercent( KIO::Job *, unsigned long percent )
{
  // TODO: make this act on the correct progress item
  kdDebug() << "ResourceGroupwise::slotJobPercent() " << percent << endl;
  if ( mProgress ) mProgress->setProgress( percent );
}

void ResourceGroupwise::cancelLoad()
{
  if ( mJob )
  {
    mJob->disconnect( this );
    mJob->kill();
  }
  mJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
  mState = Start;
}

ResourceGroupwise::SABState ResourceGroupwise::systemAddressBookState()
{
  unsigned long storedFirstSequence = mPrefs->firstSequenceNumber();
  unsigned long storedLastSequence = mPrefs->lastSequenceNumber();
  unsigned long storedLastPORebuildTime = mPrefs->lastTimePORebuild();

  kdDebug() << "ResourceGroupwise::systemAddressBookState()" << endl;
  kdDebug() << "  Stored first seq no: " << storedFirstSequence  << endl;
  kdDebug() << "  Stored last seq no: " << storedLastSequence << endl;
  kdDebug() << "  Stored last PO Rebuild time: " << storedLastPORebuildTime << endl;

  kdDebug() << "  Fetching delta info to check if update possible" << endl;
  if ( mServer->login() )
  {
    GroupWise::DeltaInfo deltaInfo = mServer->getDeltaInfo( mPrefs->systemAddressBook() );
    mServer->logout();

    mServerFirstSequence = deltaInfo.firstSequence;
    mServerLastSequence = deltaInfo.lastSequence;
    mServerLastPORebuildTime = deltaInfo.lastTimePORebuild;

    kdDebug() << "  Server first seq no: " << mServerFirstSequence  << endl;
    kdDebug() << "  Server last seq no: " << mServerLastSequence << endl;
    kdDebug() << "  Server last PO Rebuild time: " << mServerLastPORebuildTime << endl;

    if ( storedFirstSequence == 0 || storedLastSequence == 0 )
    {
      kdDebug() << "  no fetched SAB exists yet, can't update" << endl;
      return RefreshNeeded;
    }

    if ( mServerFirstSequence > storedLastSequence || storedLastPORebuildTime != mServerLastPORebuildTime)
    {
      kdDebug() << "  New entries since the last fetch are no longer available as deltas, or the PO was rebuilt, refresh needed" << endl;
      return RefreshNeeded;
    }
    
    if ( mServerLastSequence == storedLastSequence )
    {
      kdDebug() << "  The local data is up to date" << endl;
      return InSync;
    }
  }
  else
    emit loadingError( this, mServer->errorText() );

  if ( storedFirstSequence == 0 || storedLastSequence == 0 )
  {
    kdDebug() << "  Fallthrough - no fetched SAB exists yet, refresh" << endl;
    return RefreshNeeded;
  }
  else
    kdDebug() << "  Fallthrough  - returning Stale" << endl;
  return Stale;
}

bool ResourceGroupwise::shouldFetchSystemAddressBook()
{
  
  QStringList ids = mPrefs->readAddressBooks();
  return ( appIsWhiteListedForSAB() && ids.find( mPrefs->systemAddressBook() ) != ids.end() );
}

bool ResourceGroupwise::shouldFetchUserAddressBooks()
{
  QStringList ids = mPrefs->readAddressBooks();
  return ( ids.count() > 1 || ids.find( mPrefs->systemAddressBook() ) == ids.end() );
}

KURL ResourceGroupwise::createAccessUrl( BookType bookType, AccessMode mode, unsigned long lastSequenceNumber, unsigned long lastPORebuildTime )
{
  // set up list of address book IDs to fetch
  QStringList ids;
  if ( bookType == System )
    ids.append( mPrefs->systemAddressBook() );
  else
  {
    ids = mPrefs->readAddressBooks();
    ids.remove( mPrefs->systemAddressBook() );
  }

  if ( ids.isEmpty() )
    return KURL();

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

  if ( mode == Update && lastSequenceNumber > 0 && lastPORebuildTime > 0 )
  {
    query += QString::fromLatin1( "&update=true&lastSeqNo=%1&PORebuildTime=%2" ).arg( lastSequenceNumber ).arg( lastPORebuildTime );;
  }
  url.setQuery( query );
  return url;
}

void ResourceGroupwise::storeDeltaInfo()
{
  // update SAB delta info
  kdDebug() << "ResourceGroupwise::storeDeltaInfo()" << endl;
  kdDebug() << "  Server first seq no: " << mServerFirstSequence  << endl;
  kdDebug() << "  Server last seq no: " << mServerLastSequence << endl;
  kdDebug() << "  Server last PO Rebuild time: " << mServerLastPORebuildTime << endl;

  if ( mServerFirstSequence == 0 || mServerLastSequence == 0 || mServerLastPORebuildTime == 0 )
    return;
  mPrefs->setFirstSequenceNumber( mServerFirstSequence );
  mPrefs->setLastSequenceNumber( mServerLastSequence );
  mPrefs->setLastTimePORebuild( mServerLastPORebuildTime );
  mPrefs->writeConfig();
}

bool ResourceGroupwise::appIsWhiteListedForSAB()
{
  if ( !mPrefs->systemAddressBookWhiteList().contains( qApp->argv()[ 0 ] ) )
  {
    kdDebug() << "Application " << qApp->argv()[ 0 ] << " is _blacklisted_ to load the SAB" << endl;
    return false;
  }
  return true;
}

#include "kabc_resourcegroupwise.moc"
