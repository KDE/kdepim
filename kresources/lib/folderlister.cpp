/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "folderlister.h"
#include "groupwaredataadaptor.h"

#include <kio/job.h>

#include <kdebug.h>
#include <kconfig.h>

using namespace KPIM;

FolderLister::FolderLister( Type type )
  : mType( type ), mAdaptor( 0 )
{
}

void FolderLister::setFolders( const FolderLister::Entry::List &folders )
{
  mFolders = folders;
}

void FolderLister::setWriteDestinationId( const QString &id )
{
  mWriteDestinationId = id;
}

QStringList FolderLister::activeFolderIds() const
{
  QStringList ids;

  FolderLister::Entry::List::ConstIterator it;
  for( it = mFolders.begin(); it != mFolders.end(); ++it ) {
    if ( (*it).active ) {
      ids.append( (*it).id );
    }
  }

  return ids;
}

bool FolderLister::isActive( const QString &id ) const
{
  FolderLister::Entry::List::ConstIterator it;
  for( it = mFolders.begin(); it != mFolders.end(); ++it ) {
    if ( (*it).id == id ) return (*it).active;
  }
  return false;
}

void FolderLister::readConfig( const KConfig *config )
{
  kdDebug(7000) << "FolderLister::readConfig()" << endl;

  QStringList ids = config->readListEntry( "FolderIds" );
  QStringList names = config->readListEntry( "FolderNames" );
  QStringList active = config->readListEntry( "ActiveFolders" );

  QStringList::ConstIterator it;
  QStringList::ConstIterator it2 = names.begin();
  for( it = ids.begin(); it != ids.end(); ++it ) {
    Entry e;
    e.id = *it;
    if ( it2 != names.end() ) e.name = *it2;
    else e.name = *it;
    if ( active.find( e.id ) != active.end() ) e.active = true;

    mFolders.append( e );

    ++it2;
  }

  mWriteDestinationId = config->readEntry( "WriteDestinationId" );
}

void FolderLister::writeConfig( KConfig *config )
{
  QStringList ids;
  QStringList names;
  QStringList active;

  Entry::List::ConstIterator it;
  for( it = mFolders.begin(); it != mFolders.end(); ++it ) {
    ids.append( (*it).id );
    names.append( (*it).name );
    if ( (*it).active ) active.append( (*it).id );
  }

  config->writeEntry( "FolderIds", ids );
  config->writeEntry( "FolderNames", names );
  config->writeEntry( "ActiveFolders", active );

  config->writeEntry( "WriteDestinationId", mWriteDestinationId );
}

void FolderLister::setAdaptor( KPIM::GroupwareDataAdaptor *adaptor )
{
  if ( mAdaptor ) {
    disconnect( mAdaptor, 0, this, 0 );
  }
  mAdaptor = adaptor;
  connect( mAdaptor, SIGNAL( folderInfoRetrieved( const QString &,
                            const QString &, KPIM::FolderLister::FolderType ) ),
           this, SLOT( processFolderResult( const QString &, const QString &, 
                                           KPIM::FolderLister::FolderType ) ) );
  connect( mAdaptor, SIGNAL( folderSubitemRetrieved( const KURL &, bool ) ),
           this, SLOT( folderSubitemRetrieved( const KURL &, bool ) ) );
}

void FolderLister::folderSubitemRetrieved( const KURL &url, bool isFolder )
{
  if ( isFolder )
    doRetrieveFolder( url );
  else
    mProcessedUrls.append( url.path(-1) );
}

void FolderLister::retrieveFolders( const KURL &u )
{
kdDebug()<<"FolderLister::retrieveFolders( "<<u.url()<<" )"<<endl;
  mUrls.clear();
  mProcessedUrls.clear();
  bool firstRetrieve = mFolders.isEmpty();
  mFolders = defaultFolders();
  Entry::List::Iterator it = mFolders.begin(); 

  for ( ; it != mFolders.end(); ++it ) {
    if ( firstRetrieve ) {
      (*it).active = true;
    } else {
      (*it).active = isActive( (*it).id );
    }
  }

  doRetrieveFolder( u );
}

void FolderLister::doRetrieveFolder( const KURL &u )
{
  kdDebug(7000) << "FolderLister::retrieveFolders: " << u.prettyURL() << endl;

  if ( mUrls.contains( u.path(-1) ) || mProcessedUrls.contains( u.path(-1) ) ) {
    kdDebug()<<"Item "<<u.path(-1)<<" is already being downloaded "<<endl;
  } else {
    KURL url( u );
    if ( adaptor() ) adaptor()->adaptDownloadUrl( url );

    KIO::Job *listjob = createListFoldersJob( url );
    if ( listjob ) {
      mUrls.append( url.path(-1) );

      kdDebug(7000) << "FolderLister::retrieveFolders: adjustedURL=" 
                    << url.prettyURL() << endl;
      connect( listjob, SIGNAL( result( KIO::Job * ) ),
               SLOT( slotListJobResult( KIO::Job * ) ) );
    } else {
      // TODO: Indicate a problem to the user!
      kdWarning() << "Unable to create the folder list job for the url " 
                  << url.prettyURL() << endl;
      if ( mUrls.isEmpty() ) {
        kdDebug()<<"No more URLS to download, emitting foldersRead()"<<endl;
        emit foldersRead();
      }
    }
  }
}

FolderLister::Entry::List FolderLister::defaultFolders()
{
  if ( adaptor() ) {
    return adaptor()->defaultFolders();
  } else {
    return Entry::List();
  }
}

void FolderLister::processFolderResult( const QString &href, 
                                        const QString &displayName, 
                                        FolderType type )
{
kdDebug() << "FolderLister::processFolderResult( href=" << href << ", displayName=" << displayName << ", type=" << int(type) << endl;
  if ( ( mType == Calendar &&
          ( type == CalendarFolder || type == TasksFolder ||
            type == JournalsFolder ) ) ||
       ( type == ContactsFolder && mType == AddressBook ) ) {

    if ( !href.isEmpty() && !displayName.isEmpty() ) {
      Entry entry;
      entry.id = href;
      entry.name = displayName;
      entry.active = isActive( entry.id );

      mFolders.append( entry );
    }
    kdDebug(7000) << "FOLDER: " << displayName << endl;
  } else {
kdDebug() << "Folder "<< href << " is not of correct type ("<<type<<")"<<endl;
  }
}

void FolderLister::slotListJobResult( KIO::Job *job )
{
  kdDebug(7000) << "OpenGroupware::slotListJobResult(): " << endl;
  kdDebug() << "URLS (" << mUrls.count() << "): " << mUrls.join(" | ") << endl;
  kdDebug() << "Processed URLS (" << mProcessedUrls.count() << "): "
            << mProcessedUrls.join(" | ") << endl;
  KIO::SimpleJob *j = dynamic_cast<KIO::SimpleJob*>(job);
  if ( j ) {
    mUrls.remove( j->url().path(-1) );
    mProcessedUrls.append( j->url().path(-1) );
  }

  if ( job->error() ) {
    kdError() << "Unable to retrieve folders." << endl;
  } else {
    interpretListFoldersJob( job );
  }
  kdDebug() << "After URLS (" << mUrls.count() << "): " 
            << mUrls.join(" | ") << endl;
  kdDebug() << "After Processed URLS (" << mProcessedUrls.count() << "): " 
            << mProcessedUrls.join(" | ") << endl;
  if ( mUrls.isEmpty() ) {
    kdDebug()<<"No more URLS to download, emitting foldersRead()"<<endl;
    emit foldersRead();
  }
}

void FolderLister::interpretListFoldersJob( KIO::Job *job )
{
  if ( adaptor() ) {
    adaptor()->interpretListFoldersJob( job, this );
  } else {
    // TODO: Error handling should show a message to the user!
    kdError() << "The FolderLister does not have a GroupwareDataAdaptor, so "
                 "it cannot interpret the response!" << endl;
  }
}

KIO::Job *FolderLister::createListFoldersJob( const KURL &url )
{
  if ( adaptor() ) {
    return adaptor()->createListFoldersJob( url );
  } else {
    // TODO: Error handling should show a message to the user!
    kdError() << "The FolderLister does not have a GroupwareDataAdaptor, so "
                 "it cannot create the job for the folder list!" << endl;
    return 0;
  }
}

#include "folderlister.moc"
