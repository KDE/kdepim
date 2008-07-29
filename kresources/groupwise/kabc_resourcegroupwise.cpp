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
  mDownloadJob = 0;
  mProgress = 0;
  mUpdateSystemAddressBook = false;
  mPrefs = new GroupwisePrefs;

  setType( "groupwise" );
}

void ResourceGroupwise::initGroupwise()
{
  mServer = new GroupwiseServer( mPrefs->url(), mPrefs->user(),
                                 mPrefs->password(), KDateTime::Spec::LocalZone(), this );

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
                          prefs()->password(), /*HACK*/ KDateTime::Spec::LocalZone(), this );

  if ( server.login() )
  {
    mAddressBooks = server.addressBookList();
    server.logout();

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

    if ( firstRetrieve ) {
      prefs()->setReadAddressBooks( reads );
      prefs()->setWriteAddressBook( write );
    }
  }
  else
    emit loadingError( this, server.error());
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

  if ( mDownloadJob ) {
    kDebug() <<"Download still in progress";
    return true;
  }

  loadFromCache();

  if ( addressBooks().isEmpty() ) {
    kDebug() <<"Retrieving default addressbook list.";
    retrieveAddressBooks();
    writeAddressBooks();
  }

  QStringList ids = mPrefs->readAddressBooks(); // start with all the address books

  // check if we are fetching the SAB and we previously fetched it
  if ( ids.contains( mPrefs->systemAddressBook() ) &&
     mPrefs->lastSequenceNumber() != 0 )
  {
    kDebug() <<"ResourceGroupwise::asyncLoad() - Found previous sequence number, updating SAB";
    ids.removeOne( mPrefs->systemAddressBook() ); // we don't need to read this one again
    mUpdateSystemAddressBook = true;
    if ( ids.isEmpty() )
    {
      kDebug() <<"ResourceGroupwise::asyncLoad() - No user addressbooks specified, just updating SAB";
      updateAddressBooks();
      mUpdateSystemAddressBook = false;
      return true;
    }
  }

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
  for( it = ids.begin(); it != ids.end(); ++it ) {
    if ( *it == mPrefs->systemAddressBook() )
      kDebug() <<"fetching SAB" << (ids.size() > 1 ?", and user addressbooks" :", and only SAB" );
    if ( it != ids.begin() ) query += '&';
    query += "addressbookid=" + *it;
  }
  url.setQuery( query );

  kDebug() <<"Download URL:" << url;

  mJobData.clear();

  mDownloadJob = KIO::get( url, KIO::NoReload, KIO::HideProgressInfo );
  connect( mDownloadJob, SIGNAL( result( KJob * ) ),
           SLOT( slotFetchJobResult( KJob * ) ) );
  connect( mDownloadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotReadJobData( KIO::Job *, const QByteArray & ) ) );
  connect( mDownloadJob, SIGNAL( percent( KJob *, unsigned long ) ),
           SLOT( slotJobPercent( KJob *, unsigned long ) ) );

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

  saveToCache();

  mServer->logout();

  return true;
}

void ResourceGroupwise::slotFetchJobResult( KJob *job )
{
  kDebug() <<"ResourceGroupwise::slotFetchJobResult()";

  if ( job->error() ) {
    kError() << job->errorString();
    emit loadingError( this, job->errorString() );
  }

  saveToCache();

  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;

  // now we fetched addressbooks, if we are using the SAB,  update it with deltas
  QStringList ids = mPrefs->readAddressBooks();
  if ( ids.contains( mPrefs->systemAddressBook() ) )
  {
    if ( mUpdateSystemAddressBook )
    {
      kDebug() <<"updating system addressbook";
      updateAddressBooks();
      mUpdateSystemAddressBook = false;
    }
    else // we just fetched the entire SAB, now get the delta info so we know the last sequence number we have.
    {
      kDebug() <<"fetched whole SAB, now fetching delta info";
      if ( mServer->login() )
      {
        GroupWise::DeltaInfo deltaInfo = mServer->getDeltaInfo( QStringList( mPrefs->systemAddressBook() ) );
        mServer->logout();

        kDebug() <<"storing delta info to prefs";
        mPrefs->setFirstSequenceNumber( deltaInfo.firstSequence );
        mPrefs->setLastSequenceNumber( deltaInfo.lastSequence );
        mPrefs->writeConfig();
        emit loadingFinished( this );
      }
    }
  }
}

void ResourceGroupwise::slotUpdateJobResult( KJob *job )
{
  kDebug() <<"ResourceGroupwise::slotUpdateJobResult()";

  if ( job->error() ) {
    kError() << job->errorString();
    emit loadingError( this, job->errorString() );
  }
  saveToCache();

  emit loadingFinished( this );

  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;

}

bool ResourceGroupwise::updateAddressBooks()
{
  kDebug() <<"ResourceGroupwise::updateAddressBooks() - Updating address books.";

  if ( mDownloadJob ) {
    kDebug() <<"Download still in progress";
    return true;
  }

  if ( addressBooks().isEmpty() ) {
    kDebug() <<"Retrieving default addressbook list.";
    retrieveAddressBooks();
    writeAddressBooks();
  }

  QStringList ids;
  ids.append( mPrefs->systemAddressBook() );

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
  for( it = ids.begin(); it != ids.end(); ++it ) {
    if ( it != ids.begin() ) query += '&';
    query += "addressbookid=" + *it;
  }
  query += "&update=true";
  query += QString::fromLatin1( "&lastSeqNo=%1" ).arg( mPrefs->lastSequenceNumber() );

  url.setQuery( query );

  kDebug() << "Update URL:" << url;

  mJobData.clear();

  mDownloadJob = KIO::get( url, KIO::NoReload, KIO::HideProgressInfo );
  connect( mDownloadJob, SIGNAL( result( KJob * ) ),
           SLOT( slotUpdateJobResult( KJob * ) ) );
  connect( mDownloadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotUpdateJobData( KIO::Job *, const QByteArray & ) ) );
  connect( mDownloadJob, SIGNAL( percent( KJob *, unsigned long ) ),
           SLOT( slotJobPercent( KJob *, unsigned long ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(), i18n("Updating System Address Book") );
  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );

  return true;
}

void ResourceGroupwise::slotReadJobData( KIO::Job *, const QByteArray &data )
{
  kDebug() <<"ResourceGroupwise::slotReadJobData()";

  mJobData.append( data );
  //mAddrMap.clear(); //ideally we would remove all the contacts from the personal addressbooks and keep the ones from the SAB
  // for the moment we will just have to deal with double removal.

  KABC::VCardConverter conv;
  QTime profile;
  profile.start();
  Addressee::List addressees = conv.parseVCards( data );
  kDebug() <<"ResourceGroupwise::slotReadJobData() - parsed" << addressees.count() <<" contacts in"  << profile.elapsed() <<"ms, now adding to resource...";

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
  mJobData.clear();
}

void ResourceGroupwise::slotUpdateJobData( KIO::Job *, const QByteArray &data )
{
  KABC::VCardConverter conv;
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
          kError() <<"Addressee to delete did not have a remote UID, unable to find the corresponding local contact";
      }
    }
  }
}

void ResourceGroupwise::slotJobPercent( KJob *, unsigned long percent )
{
  kDebug() <<"ResourceGroupwise::slotJobPercent()" << percent;
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
