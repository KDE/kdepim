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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
  } else return QString();
}


KUrl::List FolderLister::activeFolderIds() const
{
  KUrl::List ids;

  FolderLister::Entry::List::ConstIterator it;
  for( it = mFolders.begin(); it != mFolders.end(); ++it ) {
    if ( (*it).active ) {
      ids.append( KUrl((*it).id) );
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
kDebug() << "FolderLister::contentTypeToStrings( type=" << type << ")" << endl;
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

QList<FolderLister::ContentType> FolderLister::supportedTypes()
{
  if ( adaptor() ) {
    return adaptor()->supportedTypes();
  } else {
    return QList<ContentType>();
  }
}



void FolderLister::readConfig( KPIM::GroupwarePrefsBase *newprefs )
{
  kDebug(7000) << "FolderLister::readConfig()" << endl;
  mFolders.clear();

  QStringList active = newprefs->activeFolders();
  int nr = newprefs->folderNumber();

  for ( int i=0; i<nr; ++i ) {
    QStringList l( newprefs->folder( i ) );
//     QStringList l( cfgg.readEntry( QString("Folder%1", QStringList() ).arg( i ) ) );
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
    else defaultFolders << QString();
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
  connect( mAdaptor, SIGNAL( folderInfoRetrieved( const KUrl &,
                            const QString &, KPIM::FolderLister::ContentType ) ),
           this, SLOT( processFolderResult( const KUrl &, const QString &,
                                           KPIM::FolderLister::ContentType ) ) );
  connect( mAdaptor, SIGNAL( folderSubitemRetrieved( const KUrl &, bool ) ),
           this, SLOT( folderSubitemRetrieved( const KUrl &, bool ) ) );
}

void FolderLister::folderSubitemRetrieved( const KUrl &url, bool isFolder )
{
  if ( isFolder )
    doRetrieveFolder( url );
  else {
    KUrl u( url ) ;
    u.setUser( QString() );
    u.setPass( QString() );
    mProcessedPathes.append( url.path(-1) );
  }
}

void FolderLister::retrieveFolders( const KUrl &u )
{
kDebug()<<"FolderLister::retrieveFolders( "<<u.url()<<" )"<<endl;
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

void FolderLister::doRetrieveFolder( const KUrl &u )
{
  kDebug(7000) << "FolderLister::doRetrieveFolder: " << u.prettyURL() << endl;

  KUrl url( u );
  if ( adaptor() ) adaptor()->adaptDownloadUrl( url );
  if ( mUrls.contains( url ) || mProcessedPathes.contains( url.path(-1) ) ) {
    kDebug()<<"Item "<<u.path(-1)<<" is already being downloaded "<<endl;
  } else {

    KIO::Job *listjob = createListFoldersJob( url );
    if ( listjob ) {
      mUrls.append( url );

      kDebug(7000) << "FolderLister::retrieveFolders: adjustedURL=" 
                    << url.prettyURL() << endl;
      connect( listjob, SIGNAL( result( KIO::Job * ) ),
               SLOT( slotListJobResult( KIO::Job * ) ) );
    } else {
      // TODO: Indicate a problem to the user!
      kWarning() << "Unable to create the folder list job for the url " 
                  << url.prettyURL() << endl;
    }
  }
  if ( mUrls.isEmpty() ) {
    kDebug()<<"No more URLS to download, emitting foldersRead()"<<endl;
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

void FolderLister::processFolderResult( const KUrl &href, 
                                        const QString &displayName, 
                                        ContentType type )
{
kDebug() << "FolderLister::processFolderResult( href=" << href.url() << ", displayName=" << displayName << ", type=" << int(type) << endl;
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
    kDebug(7000) << "FOLDER: " << displayName << endl;
kDebug()<<"mFolders.size="<<mFolders.size()<<endl;
  } else {
kDebug() << "Folder "<< href << " is not of correct type ("<<type<<")"<<endl;
  }
}

void FolderLister::slotListJobResult( KIO::Job *job )
{
  kDebug(7000) << "OpenGroupware::slotListJobResult(): " << endl;
  kDebug() << "URLS (" << mUrls.count() << "): " << mUrls.toStringList().join(" | ") << endl;
  kDebug() << "Processed URLS (" << mProcessedPathes.count() << "): "
            << mProcessedPathes.join(" | ") << endl;
  KIO::SimpleJob *j = dynamic_cast<KIO::SimpleJob*>(job);
  if ( j ) {
    mUrls.remove( j->url() );
    mProcessedPathes.append( j->url().path(-1) );
  }

  if ( job->error() ) {
    kError() << "Unable to retrieve folders." << endl;
  } else {
    interpretListFoldersJob( job );
  }
  kDebug() << "After URLS (" << mUrls.count() << "): " 
            << mUrls.toStringList().join(" | ") << endl;
  kDebug() << "After Processed URLS (" << mProcessedPathes.count() << "): "
            << mProcessedPathes.join(" | ") << endl;
  if ( mUrls.isEmpty() ) {
    kDebug()<<"No more URLS to download, emitting foldersRead()"<<endl;
    emit foldersRead();
  }
}

void FolderLister::interpretListFoldersJob( KIO::Job *job )
{
  if ( adaptor() ) {
    adaptor()->interpretListFoldersJob( job, this );
  } else {
    // TODO: Error handling should show a message to the user!
    kError() << "The FolderLister does not have a GroupwareDataAdaptor, so "
                 "it cannot interpret the response!" << endl;
  }
}

KIO::Job *FolderLister::createListFoldersJob( const KUrl &url )
{
  if ( adaptor() ) {
    return adaptor()->createListFoldersJob( url );
  } else {
    // TODO: Error handling should show a message to the user!
    kError() << "The FolderLister does not have a GroupwareDataAdaptor, so "
                 "it cannot create the job for the folder list!" << endl;
    return 0;
  }
}

#include "folderlister.moc"
