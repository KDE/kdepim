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
#include <kabc/vcardconverter.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include <libkdepim/groupwarejob.h>

#include "kabc_groupwareprefs.h"

#include "kabc_resourcegroupware.h"

using namespace KABC;

ResourceGroupware::ResourceGroupware( const KConfig *config )
  : ResourceCached( config )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config ) {
    readConfig( config );
  }
}

ResourceGroupware::ResourceGroupware( const KURL &url,
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

void ResourceGroupware::init()
{
  mDownloadJob = 0;
  mProgress = 0;

  mPrefs = new GroupwarePrefs;

  setType( "groupware" );
}

ResourceGroupware::~ResourceGroupware()
{
  delete mPrefs;
  mPrefs = 0;
}

void ResourceGroupware::readConfig( const KConfig * )
{
  mPrefs->readConfig();

  readAddressBooks();
}

void ResourceGroupware::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  writeAddressBooks();

  mPrefs->writeConfig();
}

void ResourceGroupware::readAddressBooks()
{
#if 0
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
    Groupware::AddressBook ab;
    ab.id = ids[ i ];
    ab.name = names[ i ];
    ab.isPersonal = personals[ i ] == "1";
    ab.isFrequentContacts = frequents[ i ] == "1";
    
    mAddressBooks.append( ab );
  }
#endif
}

void ResourceGroupware::writeAddressBooks()
{
#if 0
  QStringList ids;
  QStringList names;
  QStringList personals;
  QStringList frequents;
  Groupware::AddressBook::List::ConstIterator it;
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
#endif
}

void ResourceGroupware::retrieveAddressBooks()
{
#if 0
  bool firstRetrieve = mAddressBooks.isEmpty();

  GroupwareServer server( prefs()->url(),
                          prefs()->user(),
                          prefs()->password(), this );

  server.login();
  mAddressBooks = server.addressBookList();
  server.logout();

  if ( firstRetrieve ) {
    QStringList reads;
    QString write; 
    
    Groupware::AddressBook::List::ConstIterator it;
    for( it = mAddressBooks.begin(); it != mAddressBooks.end(); ++it ) {
      if ( (*it).isPersonal ) {
        reads.append( (*it).id );
        if ( write.isEmpty() ) write = (*it).id;
      }
    }
    
    prefs()->setReadAddressBooks( reads );
    prefs()->setWriteAddressBook( write );
  }
#endif
}

Ticket *ResourceGroupware::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdDebug(5700) << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceGroupware::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceGroupware::doOpen()
{
  return true;
}

void ResourceGroupware::doClose()
{
  kdDebug() << "ResourceGroupware::doClose()" << endl;

  cancelLoad();
}

bool ResourceGroupware::load()
{
  return asyncLoad();
}

bool ResourceGroupware::asyncLoad()
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

  mJobData = QString::null;

  mDownloadJob = KPIM::GroupwareJob::getAddressBook( url );
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

  return true;
}

bool ResourceGroupware::save( Ticket *ticket )
{
  return asyncSave( ticket );
}

bool ResourceGroupware::asyncSave( Ticket* )
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

void ResourceGroupware::slotJobResult( KIO::Job *job )
{
  kdDebug() << "ResourceGroupware::slotJobResult(): " << endl;

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

void ResourceGroupware::slotJobData( KIO::Job *, const QByteArray &data )
{
//  kdDebug() << "ResourceGroupware::slotJobData()" << endl;

  mJobData.append( data.data() );
}

void ResourceGroupware::slotJobPercent( KIO::Job *, unsigned long percent )
{
  kdDebug() << "ResourceGroupware::slotJobPercent() " << percent << endl;
  if ( mProgress ) mProgress->setProgress( percent );
}

void ResourceGroupware::cancelLoad()
{
  if ( mDownloadJob ) mDownloadJob->kill();
  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
}

#include "kabc_resourcegroupware.moc"
