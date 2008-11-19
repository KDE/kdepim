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

#include <QApplication>
#include <QFile>

#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include "kabc_groupwiseprefs.h"

#include "kabc_resourcegroupwise.h"

using namespace KABC;


ResourceGroupwise::ResourceGroupwise()
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  initGroupwise();
}
ResourceGroupwise::ResourceGroupwise( const KConfigGroup &group )
  : ResourceCached( group )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  readConfig( group );

  initGroupwise();
}

ResourceGroupwise::ResourceGroupwise( const KUrl &url,
                                      const QString &user,
                                      const QString &password,
                                      const QStringList &readAddressBooks,
                                      const QString &writeAddressBook )
  : ResourceCached()
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
                                 mPrefs->password(), KDateTime::Spec::LocalZone(), this );

}

ResourceGroupwise::~ResourceGroupwise()
{
  delete mServer;
  mServer = 0;

  delete mPrefs;
  mPrefs = 0;
}

void ResourceGroupwise::readConfig( const KConfigGroup & )
{
  mPrefs->readConfig();

  readAddressBooks();
}

void ResourceGroupwise::writeConfig( KConfigGroup &group )
{
  Resource::writeConfig( group );

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
    kError() <<"Corrupt addressbook configuration";
    return;
  }

  mAddressBooks.clear();

  for( int i = 0; i < ids.count(); ++i ) {
    GroupWise::AddressBook ab;
    ab.id = ids[ i ];
    ab.name = names[ i ];
    ab.isPersonal = personals[ i ] == "1";
    ab.isFrequentContacts = frequents[ i ] == "1";

    mAddressBooks.append( ab );
  }
  initGroupwise();
}

void ResourceGroupwise::writeAddressBooks()
{
  QStringList ids;
  QStringList names;
  QStringList personals;
  QStringList frequents;
  GroupWise::AddressBook::List::ConstIterator it;
  for( it = mAddressBooks.constBegin(); it != mAddressBooks.constEnd(); ++it ) {
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
                          prefs()->password(), /*HACK*/ KDateTime::Spec::LocalZone(), this );

  if ( server.login() )
  {
    mAddressBooks = server.addressBookList();
    server.logout();

    QStringList reads;
    QString write;

    GroupWise::AddressBook::List::ConstIterator it;
    for( it = mAddressBooks.constBegin(); it != mAddressBooks.constEnd(); ++it ) {
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
    kDebug(5700) <<"no addressbook";
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
  kDebug() <<"ResourceGroupwise::doClose()";

  cancelLoad();
}

bool ResourceGroupwise::load()
{
  return asyncLoad();
}

bool ResourceGroupwise::asyncLoad()
{
  kDebug() <<"ResourceGroupwise::asyncLoad()";
  //mPrefs->readConfig();  TODO: remove if the system addressbook is not read when disabled in config
  
  if ( mState != Start )
  {
     kDebug() << "  Download still in progress";
     return true;
  }

  if ( appIsWhiteListedForSAB() )
    loadFromCache();

  if ( !mProgress )
  {
    mProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18nc( "label for addressbook load progress", "Loading GroupWise resource %1", resourceName() ), QString::null, true /*CanBeCancelled*/, mPrefs->url().startsWith("https" ) );
    connect( mProgress, SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
               SLOT( cancelLoad() ) );
  }

  if ( addressBooks().isEmpty() ) {
    kDebug() << "  Retrieving default addressbook list.";
    retrieveAddressBooks();
    writeAddressBooks();
  }

  SABState sabState = systemAddressBookState();
  if ( shouldFetchSystemAddressBook() )
  {
    if ( sabState == RefreshNeeded )
    {
      kDebug() << "  Fetching system addressbook";
      fetchAddressBooks( System );
      return true;
    }
    else if ( sabState == Stale )
    {
      kDebug() << "  Updating system addressbook";
      updateSystemAddressBook(); // we then fetch the user address books after doing this
      return true;
    }
  }
  else if ( shouldFetchUserAddressBooks() )
  {
    kDebug() << "  Fetching user addressbook";
    fetchAddressBooks( User );
    return true;
  }
  return true;
}
 
void ResourceGroupwise::fetchAddressBooks( const BookType bookType )
{
  KUrl url = createAccessUrl( bookType, Fetch );
  if ( !url.isValid() )
    return;

  kDebug() << k_funcinfo << ( bookType == System ? " System" : " User" ) << " URL: " << url;
  // sanity check
  if ( bookType == User && !( mState == SABUptodate || mState == Start ) )
  {
    kDebug() << "  **ERROR** - fetchAddressBooks( User ) called when SAB not up to date";
    return;
  }

  mState = ( bookType == System ? FetchingSAB : FetchingUAB );
  mJobData.clear();

  if ( mJob )
  {
    kDebug() << "  **ERROR** - called when a job was already running!";
    return;
  }

  mJob = KIO::get( url, KIO::NoReload, KIO::HideProgressInfo );
  kDebug() << "  Job address: " << mJob;
  connect( mJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotReadJobData( KIO::Job *, const QByteArray & ) ) );
  connect( mJob, SIGNAL( percent( KJob *, unsigned long ) ),
           SLOT( slotJobPercent( KJob *, unsigned long ) ) );
  connect( mJob, SIGNAL( finished( KJob * ) ),
           SLOT( slotJobFinished( KJob * ) ) );


  if ( bookType == System )
  {
    connect( mJob, SIGNAL( result( KJob * ) ),
           SLOT( fetchSABResult( KJob * ) ) );
    mSABProgress = KPIM::ProgressManager::instance()->createProgressItem(
        mProgress, KPIM::ProgressManager::getUniqueID(),
        i18n( "Fetching System Address Book" ), QString::null,
        false /*CannotBeCancelled*/,
        mPrefs->url().startsWith("https" ) );
  }
  else
  {
    connect( mJob, SIGNAL( result( KJob * ) ),
           SLOT( fetchUABResult( KJob * ) ) );
    mUABProgress = KPIM::ProgressManager::instance()->createProgressItem(
        mProgress, KPIM::ProgressManager::getUniqueID(),
        i18n( "Fetching User Address Books" ), QString::null,
        false /*CannotBeCancelled*/,
        mPrefs->url().startsWith("https" ) );
  }

  return;
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
    saveToCache();

  mServer->logout();

  return true;
}

void ResourceGroupwise::fetchSABResult( KJob *job )
{
  kDebug();

  if ( job->error() ) {
    kError() << job->errorString();
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

void ResourceGroupwise::fetchUABResult( KJob *job )
{
  kDebug() << "ResourceGroupwise::fetchUABResult() ";

  if ( job->error() ) {
    kError() << job->errorString();
    emit loadingError( this, job->errorString() );
    // TODO kill the rest of the load sequence as well
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
  kDebug();

  if ( mState != Start ) {
    kWarning() << "  Action already in progress";
    return;
  }

  if ( addressBooks().isEmpty() ) {
    kDebug() << "  Retrieving default addressbook list.";
    retrieveAddressBooks();
    writeAddressBooks();
  }

  KUrl url = createAccessUrl( System, Update, mPrefs->lastSequenceNumber(), mPrefs->lastTimePORebuild() );
  kDebug() << "  Update URL: " << url;

  mJobData.clear();
  mSABProgress = KPIM::ProgressManager::instance()->createProgressItem(
      mProgress, KPIM::ProgressManager::getUniqueID(),
      i18n( "Updating System Address Book" ), QString::null,
      false /*CannotBeCancelled*/,
      mPrefs->url().startsWith("https" ) );
 
  mJob = KIO::get( url, KIO::NoReload, KIO::HideProgressInfo );
  connect( mJob, SIGNAL( result( KJob * ) ),
           SLOT( updateSABResult( KJob * ) ) );
  connect( mJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotUpdateJobData( KIO::Job *, const QByteArray & ) ) );
  connect( mJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
           SLOT( slotJobPercent( KIO::Job *, unsigned long ) ) );
  connect( mJob, SIGNAL( finished( KJob * ) ),
           SLOT( slotJobFinished( KJob * ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(), i18n("Updating System Address Book") );
  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );

  return;
}

void ResourceGroupwise::updateSABResult( KJob *job )
{
  kDebug() << "ResourceGroupwise::updateSABResult() ";

  mSABProgress->setComplete();
  mSABProgress = 0;
  mJob->disconnect( this );
  mJob = 0;

  int errorCode = job->error();
  if ( errorCode != 0 ) {
    if ( errorCode == KIO::ERR_NO_CONTENT ) // we need to refresh the SAB
    {
      kDebug() << "  update SAB failed, fetching all of it again";
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
  kDebug();
  Q_UNUSED( job );

  mJobData.append( data );

  KABC::VCardConverter conv;
  QTime profile;
  profile.start();
  Addressee::List addressees = conv.parseVCards( data );
  kDebug() << "  parsed " << addressees.count() << " contacts in "  << profile.elapsed() << "ms, now adding to resource...";

  Addressee::List::ConstIterator it;
  for( it = addressees.constBegin(); it != addressees.constEnd(); ++it ) {
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
  mJobData.clear();
}

void ResourceGroupwise::slotUpdateJobData( KIO::Job *job, const QByteArray &data )
{
  kDebug() << "  Job address: " << job;
  KABC::VCardConverter conv;
  mJobData.append( data.data() );

  Addressee::List addressees = conv.parseVCards( mJobData );
  Addressee::List::ConstIterator it;

  for( it = addressees.constBegin(); it != addressees.constEnd(); ++it ) {
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
          kError() <<"Addressee to delete did not have a remote UID, unable to find the corresponding local contact";
      }
    }
  }
  mJobData.clear();
}

void ResourceGroupwise::loadCompleted()
{
  kDebug() << "ResourceGroupwise::loadCompleted()";
  if ( mProgress )
    mProgress->setComplete();
  mProgress = 0;
  mSABProgress = 0;
  mUABProgress = 0;
  mState = Start;
  if ( appIsWhiteListedForSAB() ) {
    saveToCache();
  }
  emit loadingFinished( this );
  addressBook()->emitAddressBookChanged();
}

void ResourceGroupwise::slotJobPercent( KJob *, unsigned long percent )
{
  // TODO: make this act on the correct progress item
  kDebug() <<"ResourceGroupwise::slotJobPercent()" << percent;
  if ( mProgress ) mProgress->setProgress( percent );
}

void ResourceGroupwise::slotJobFinished( KJob * )
{
  kDebug();
  if ( mJob ) {
    mJob = 0;
  }
  if ( mProgress ) {
    mProgress->setComplete();
    mProgress = 0;
    mState = Start;
  }
}

void ResourceGroupwise::cancelLoad()
{
  if ( mJob ) {
    mJob->disconnect( this );
    mJob->kill();
    mJob = 0;
  }
  if ( mProgress ) {
    mProgress->setComplete();
    mProgress = 0;
    mState = Start;
  }
}

ResourceGroupwise::SABState ResourceGroupwise::systemAddressBookState()
{
  unsigned long storedFirstSequence = mPrefs->firstSequenceNumber();
  unsigned long storedLastSequence = mPrefs->lastSequenceNumber();
  unsigned long storedLastPORebuildTime = mPrefs->lastTimePORebuild();

  kDebug() << "  Stored first seq no: " << storedFirstSequence ;
  kDebug() << "  Stored last seq no: " << storedLastSequence;
  kDebug() << "  Stored last PO Rebuild time: " << storedLastPORebuildTime;

  kDebug() << "  Fetching delta info to check if update possible";
  if ( mServer->login() )
  {
    GroupWise::DeltaInfo deltaInfo = mServer->getDeltaInfo( QStringList( mPrefs->systemAddressBook() ) );
    mServer->logout();

    mServerFirstSequence = deltaInfo.firstSequence;
    mServerLastSequence = deltaInfo.lastSequence;
    mServerLastPORebuildTime = deltaInfo.lastTimePORebuild;

    kDebug() << "  Server first seq no: " << mServerFirstSequence ;
    kDebug() << "  Server last seq no: " << mServerLastSequence;
    kDebug() << "  Server last PO Rebuild time: " << mServerLastPORebuildTime;

    if ( storedFirstSequence == 0 || storedLastSequence == 0 )
    {
      kDebug() << "  no fetched SAB exists yet, can't update";
      return RefreshNeeded;
    }

    if ( mServerFirstSequence > storedLastSequence || storedLastPORebuildTime != mServerLastPORebuildTime)
    {
      kDebug() << "  New entries since the last fetch are no longer available as deltas, or the PO was rebuilt, refresh needed";
      return RefreshNeeded;
    }

    if ( mServerLastSequence == storedLastSequence )
    {
      kDebug() << "  The local data is up to date";
      return InSync;
    }
  }
  else
    emit loadingError( this, mServer->errorText() );


  if ( storedFirstSequence == 0 || storedLastSequence == 0 )
  {
    kDebug() << "  Fallthrough - no fetched SAB exists yet, refresh";
    return RefreshNeeded;
  }
  else
    kDebug() << "  Fallthrough  - returning Stale";
  return Stale;
}

bool ResourceGroupwise::shouldFetchSystemAddressBook()
{
  QStringList ids = mPrefs->readAddressBooks();
  return appIsWhiteListedForSAB() && ids.contains( mPrefs->systemAddressBook() );
}

bool ResourceGroupwise::shouldFetchUserAddressBooks()
{
  QStringList ids = mPrefs->readAddressBooks();
  return ids.count() > 1 || ids.contains( mPrefs->systemAddressBook() );
}

KUrl ResourceGroupwise::createAccessUrl( BookType bookType, AccessMode mode, unsigned long lastSequenceNumber, unsigned long lastPORebuildTime )
{
  // set up list of address book IDs to fetch
  QStringList ids;
  if ( bookType == System )
    ids.append( mPrefs->systemAddressBook() );
  else
  {
    ids = mPrefs->readAddressBooks();
    ids.removeAll( mPrefs->systemAddressBook() );
  }

  if ( ids.isEmpty() )
    return KUrl();

  KUrl url( prefs()->url() );
  if ( url.protocol() == "http" ) 
    url.setProtocol( "groupwise" );
  else
    url.setProtocol( "groupwises" );

  url.setPath( url.path() + "/addressbook/" );
  url.setUser( prefs()->user() );
  url.setPass( prefs()->password() );

  QString query = "?";
  QStringList::ConstIterator it;
  for( it = ids.constBegin(); it != ids.constEnd(); ++it ) {
    if ( it != ids.constBegin() ) query += "&";
    query += "addressbookid=" + *it;
  }

  if ( mode == Update && lastSequenceNumber > 0 && lastPORebuildTime > 0 )
  {
    query += QString::fromLatin1( "&update=true&lastSeqNo=%1&PORebuildTime=%2" ).arg( lastSequenceNumber ).arg( lastPORebuildTime );
  }
  url.setQuery( query );
  return url;
}

void ResourceGroupwise::storeDeltaInfo()
{
  // update SAB delta info
  kDebug() << "  Server first seq no: " << mServerFirstSequence;
  kDebug() << "  Server last seq no: " << mServerLastSequence;
  kDebug() << "  Server last PO Rebuild time: " << mServerLastPORebuildTime;

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
    kDebug() << "Application " << qApp->argv()[ 0 ] << " is _blacklisted_ to load the SAB";
    return false;
  }
  return true;
}

#include "kabc_resourcegroupwise.moc"
