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
#include "kresources_groupwareprefs.h"

#include <kio/job.h>

#include <kdebug.h>
#include <kconfig.h>
#include <qstringlist.h>

using namespace KPIM;

FolderLister::FolderLister( Type type )
  : mType( type ), mAdaptor( 0 )
{
}

void FolderLister::setFolders( const FolderLister::Entry::List &folders )
{
  mFolders = folders;
}

void FolderLister::setWriteDestinationId( KPIM::FolderLister::ContentType type, const QString &id )
{
  mWriteDestinationId[ type ] = id;
}

QString FolderLister::writeDestinationId( KPIM::FolderLister::ContentType type ) const
{
  if ( mWriteDestinationId.contains( type ) ) {
    return mWriteDestinationId[type];
  } else if ( mWriteDestinationId.contains( KPIM::FolderLister::All ) ) {
    return mWriteDestinationId[ KPIM::FolderLister::All ];
  } else if ( mWriteDestinationId.contains( KPIM::FolderLister::Unknown ) ) {
    return mWriteDestinationId[ KPIM::FolderLister::Unknown ];
  } else return QString::null;
}


KURL::List FolderLister::activeFolderIds() const
{
  KURL::List ids;

  FolderLister::Entry::List::ConstIterator it;
  for( it = mFolders.begin(); it != mFolders.end(); ++it ) {
    if ( (*it).active ) {
      ids.append( KURL((*it).id) );
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

QStringList FolderLister::contentTypeToStrings( ContentType type )
{
kdDebug() << "FolderLister::contentTypeToStrings( type=" << type << ")" << endl;
  QStringList cont;
  if ( type == All ) {
    cont << "All";
  } else if ( type == Unknown ) {
    cont << "Unknown";
  } else {
    if ( type & Contact ) cont << "Contact";
    if ( type & Event )   cont << "Event";
    if ( type & Todo )    cont << "Todo";
    if ( type & Journal ) cont << "Journal";
    if ( type & Message ) cont << "Message";
    if ( type & Memo )    cont << "Memo";
    if ( type & Folder )  cont << "Folder";
  }
  return cont;
}

FolderLister::ContentType FolderLister::contentTypeFromString( const QString &type )
{
  if ( type == "All" )     return All;
  if ( type == "Contact" ) return Contact;
  if ( type == "Event" )   return Event;
  if ( type == "Todo" )    return Todo;
  if ( type == "Journal" ) return Journal;
  if ( type == "Message" ) return Message;
  if ( type == "Memo" )    return Memo;
  if ( type == "Folder" )  return Folder;
  return Unknown;
}

QValueList<FolderLister::ContentType> FolderLister::supportedTypes()
{
  if ( adaptor() ) {
    return adaptor()->supportedTypes();
  } else {
    return QValueList<ContentType>();
  }
}



void FolderLister::readConfig( KPIM::GroupwarePrefsBase *newprefs )
{
  kdDebug(7000) << "FolderLister::readConfig()" << endl;
  mFolders.clear();

  QStringList active = newprefs->activeFolders();
  int nr = newprefs->folderNumber();

  for ( int i=0; i<nr; ++i ) {
    QStringList l( newprefs->folder( i ) );
//     QStringList l( cfgg.readListEntry( QString("Folder%1").arg( i ) ) );
    Entry e;
    if ( l.count()>0 ) {
      e.id = l.first();
      l.pop_front();
    }
    if ( l.count()>1 ) {
      e.name = l.first();
      l.pop_front();
    }
    e.type = Unknown;
    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
      e.type = (FolderLister::ContentType)( e.type | contentTypeFromString( *it ) );
    }
    if ( active.find( e.id ) != active.end() ) e.active = true;
    mFolders.append( e );
  }

  QStringList destinations( newprefs->defaultDestinations() );
  
  #define readDestination(type) \
    if ( destinations.count()>0 ) { \
      mWriteDestinationId[ type ] = destinations.first(); \
      destinations.pop_front(); \
    }
  readDestination( FolderLister::Event );
  readDestination( FolderLister::Todo );
  readDestination( FolderLister::Journal );
  readDestination( FolderLister::Contact );
  readDestination( FolderLister::All );
  readDestination( FolderLister::Unknown );
  #undef readDestination
}

void FolderLister::writeConfig( GroupwarePrefsBase *newprefs )
{
  QStringList ids;
  QStringList names;
  QStringList active;

//   KConfigGroup cfgg( newprefs->config(), "Folders" );
  int nr = 0;
  Entry::List::ConstIterator it;
  for( it = mFolders.begin(); it != mFolders.end(); ++it ) {
    QStringList lst;
    lst << (*it).id;
    lst << (*it).name;
    lst += contentTypeToStrings( (*it).type );
    newprefs->setFolder( nr, lst );
//     cfgg.writeEntry( QString("Folder%1").arg( nr ), lst );
    if ( (*it).active ) active.append( (*it).id );
    nr++;
  }
  newprefs->setFolderNumber( nr );

  newprefs->setActiveFolders( active );

  QStringList defaultFolders;
  #define writeDestination(type) \
    if ( mWriteDestinationId.contains( type ) ) \
      defaultFolders << mWriteDestinationId[type]; \
    else defaultFolders << QString::null;
  writeDestination( KPIM::FolderLister::Event );
  writeDestination( KPIM::FolderLister::Todo );
  writeDestination( KPIM::FolderLister::Journal );
  writeDestination( KPIM::FolderLister::Contact );
  writeDestination( KPIM::FolderLister::All );
  writeDestination( KPIM::FolderLister::Unknown );
  #undef writeDestination

  newprefs->setDefaultDestinations( defaultFolders );
}

void FolderLister::setAdaptor( KPIM::GroupwareDataAdaptor *adaptor )
{
  if ( mAdaptor ) {
    disconnect( mAdaptor, 0, this, 0 );
  }
  mAdaptor = adaptor;
  connect( mAdaptor, SIGNAL( folderInfoRetrieved( const KURL &,
                            const QString &, KPIM::FolderLister::ContentType ) ),
           this, SLOT( processFolderResult( const KURL &, const QString &,
                                           KPIM::FolderLister::ContentType ) ) );
  connect( mAdaptor, SIGNAL( folderSubitemRetrieved( const KURL &, bool ) ),
           this, SLOT( folderSubitemRetrieved( const KURL &, bool ) ) );
}

void FolderLister::folderSubitemRetrieved( const KURL &url, bool isFolder )
{
  if ( isFolder )
    doRetrieveFolder( url );
  else {
    KURL u( url ) ;
    u.setUser( QString::null );
    u.setPass( QString::null );
    mProcessedPathes.append( url.path(-1) );
  }
}

void FolderLister::retrieveFolders( const KURL &u )
{
kdDebug()<<"FolderLister::retrieveFolders( "<<u.url()<<" )"<<endl;
  mUrls.clear();
  mProcessedPathes.clear();
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
  kdDebug(7000) << "FolderLister::doRetrieveFolder: " << u.prettyURL() << endl;

  KURL url( u );
  if ( adaptor() ) adaptor()->adaptDownloadUrl( url );
  if ( mUrls.contains( url ) || mProcessedPathes.contains( url.path(-1) ) ) {
    kdDebug()<<"Item "<<u.path(-1)<<" is already being downloaded "<<endl;
  } else {

    KIO::Job *listjob = createListFoldersJob( url );
    if ( listjob ) {
      mUrls.append( url );

      kdDebug(7000) << "FolderLister::retrieveFolders: adjustedURL=" 
                    << url.prettyURL() << endl;
      connect( listjob, SIGNAL( result( KIO::Job * ) ),
               SLOT( slotListJobResult( KIO::Job * ) ) );
    } else {
      // TODO: Indicate a problem to the user!
      kdWarning() << "Unable to create the folder list job for the url " 
                  << url.prettyURL() << endl;
    }
  }
  if ( mUrls.isEmpty() ) {
    kdDebug()<<"No more URLS to download, emitting foldersRead()"<<endl;
    emit foldersRead();
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

void FolderLister::processFolderResult( const KURL &href, 
                                        const QString &displayName, 
                                        ContentType type )
{
kdDebug() << "FolderLister::processFolderResult( href=" << href.url() << ", displayName=" << displayName << ", type=" << int(type) << endl;
  if ( ( mType == Calendar && ( type & ( Event | Todo |Journal) ) ) ||
       ( mType == AddressBook && (type & Contact ) ) ) {

    if ( !href.isEmpty() && !displayName.isEmpty() ) {
      Entry entry;
      entry.id = href.url();
      entry.name = displayName;
      entry.type = type;
      entry.active = isActive( entry.id );

      mFolders.append( entry );
    }
    kdDebug(7000) << "FOLDER: " << displayName << endl;
kdDebug()<<"mFolders.size="<<mFolders.size()<<endl;
  } else {
kdDebug() << "Folder "<< href << " is not of correct type ("<<type<<")"<<endl;
  }
}

void FolderLister::slotListJobResult( KIO::Job *job )
{
  kdDebug(7000) << "OpenGroupware::slotListJobResult(): " << endl;
  kdDebug() << "URLS (" << mUrls.count() << "): " << mUrls.toStringList().join(" | ") << endl;
  kdDebug() << "Processed URLS (" << mProcessedPathes.count() << "): "
            << mProcessedPathes.join(" | ") << endl;
  KIO::SimpleJob *j = dynamic_cast<KIO::SimpleJob*>(job);
  if ( j ) {
    mUrls.remove( j->url() );
    mProcessedPathes.append( j->url().path(-1) );
  }

  if ( job->error() ) {
    kdError() << "Unable to retrieve folders." << endl;
  } else {
    interpretListFoldersJob( job );
  }
  kdDebug() << "After URLS (" << mUrls.count() << "): " 
            << mUrls.toStringList().join(" | ") << endl;
  kdDebug() << "After Processed URLS (" << mProcessedPathes.count() << "): "
            << mProcessedPathes.join(" | ") << endl;
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
