/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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
#include "webdavhandler.h"

#include <kio/davjob.h>
#include <kio/job.h>

#include <kdebug.h>
#include <kconfig.h>

#include <qdom.h>

using namespace KPIM;

FolderLister::FolderLister( Type type )
  : mType( type )
{
}

KURL FolderLister::adjustUrl( const KURL &u )
{
kdDebug()<<"FolderLister::adjustUrl( url="<<u.url()<<")"<<endl;
  KURL url( u );
  url.setPass( mPassword );
  url.setUser( mUser );
  return WebdavHandler::toDAV( url );
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

KIO::DavJob *FolderLister::createJob( const KURL &url )
{
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement(  doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement(  doc, root, "prop" );
  WebdavHandler::addDavElement( doc, prop, "displayname" );
  WebdavHandler::addDavElement( doc, prop, "resourcetype" );
  WebdavHandler::addDavElement( doc, prop, "hassubs" );
  
  kdDebug(7000) << "props: " << doc.toString() << endl;
  return KIO::davPropFind( url, doc, "1", false );
}


void FolderLister::retrieveFolders( const KURL &u )
{
kdDebug()<<"FolderLister::retrieveFolders( "<<u.url()<<" )"<<endl;
  mUrls.clear();
  mProcessedUrls.clear();
  bool firstRetrieve = mFolders.isEmpty();
  mFolders = defaultFolders();

  for ( Entry::List::Iterator it = mFolders.begin(); it != mFolders.end(); ++it ) {
    if ( firstRetrieve ) {
      (*it).active = true;
    } else {
      (*it).active = isActive( (*it).id );
    }
  }
  
  mUser = u.user();
  mPassword = u.pass();
  
  doRetrieveFolder( u );
}

void FolderLister::doRetrieveFolder( const KURL &u )
{
  kdDebug(7000) << "FolderLister::retrieveFolders: " << u.prettyURL() << endl;

  if ( mUrls.contains( u.path(-1) ) || mProcessedUrls.contains( u.path(-1) ) ) {
    kdDebug()<<"Item "<<u.path(-1)<<" is already being downloaded "<<endl;
  } else {
    KURL url( adjustUrl( u ) );
    mUrls.append( url.path(-1) );

    KIO::Job *listjob = createJob( url );

    kdDebug(7000) << "FolderLister::retrieveFolders: adjustedURL=" << url.prettyURL() << endl;
    connect( listjob, SIGNAL( result( KIO::Job * ) ),
             SLOT( slotListJobResult( KIO::Job * ) ) );
  }
}

FolderLister::Entry::List FolderLister::defaultFolders()
{
  Entry::List newFolders;

  return newFolders;
}

void FolderLister::interpretFolderResult( KIO::Job *job )
{
  KIO::DavJob *davjob = dynamic_cast<KIO::DavJob*>( job );
  Q_ASSERT( davjob );
  if ( !davjob ) return;
  
  QDomDocument doc = davjob->response();
  kdDebug(7000) << " Doc: " << doc.toString() << endl;

  QDomElement docElement = doc.documentElement();
  QDomNode n;
  for( n = docElement.firstChild(); !n.isNull();
    n = n.nextSibling() ) {

    QDomElement ee1 = n.toElement();

    QDomNode n2 = n.namedItem( "propstat" );
    QDomNode n3 = n2.namedItem( "prop" );

    QString href = n.namedItem( "href" ).toElement().text();
    QString displayName = n3.namedItem( "displayname" ).toElement().text();
    
    FolderType type = getFolderType( n3 );
    
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
    }
    QString hassubs = n3.namedItem( "hassubs" ).toElement().text();
kdDebug()<<"hassubs="<<hassubs<<endl;
    if ( hassubs == "1" ) {
      doRetrieveFolder( href );
    } else {
      KURL u( href );
kdDebug()<<"Not descending into "<< href<<", adding path "<<u.path(-1)<<" to the list of processed URLS"<<endl;
      mProcessedUrls.append( u.path(-1) );
    }
  }
}

void FolderLister::slotListJobResult( KIO::Job *job )
{
  kdDebug(7000) << "OpenGroupware::slotListJobResult(): " << endl;
  kdDebug()<<"URLS ("<<mUrls.count()<<"): "<<mUrls.join(" | ") << endl;
  kdDebug()<<"Processed URLS ("<<mProcessedUrls.count()<<"): "<<mProcessedUrls.join(" | ") << endl;
  KIO::SimpleJob *j = dynamic_cast<KIO::SimpleJob*>(job);
  if ( j ) {
    mUrls.remove( j->url().path(-1) );
    mProcessedUrls.append( j->url().path(-1) );
  }

  if ( job->error() ) {
    kdError() << "Unable to retrieve folders." << endl;
  } else {
    interpretFolderResult( job );
  }
  kdDebug()<<"After URLS ("<<mUrls.count()<<"): "<<mUrls.join(" | ") << endl;
  kdDebug()<<"After Processed URLS ("<<mProcessedUrls.count()<<"): "<<mProcessedUrls.join(" | ") << endl;
  if ( mUrls.isEmpty() ) {
    kdDebug()<<"No more URLS to download, emitting foldersRead()"<<endl;
    emit foldersRead();
  }
}

#include "folderlister.moc"
