/*
    Copyright (c) 2005 by Volker Krause <volker.krause@rwth-aachen.de>
    Copyright (c) 2005 by Florian Schröder <florian@deltatauchi.de>

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

#include <qfile.h>
#include <qdom.h>
#include <qstring.h>

#include <kdebug.h>
#include <kio/job.h>
#include <kio/davjob.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "sloxbase.h"
#include "sloxfolder.h"
#include "sloxfoldermanager.h"
#include "webdavhandler.h"


SloxFolderManager::SloxFolderManager( SloxBase *res, const KURL & baseUrl ) :
  mDownloadJob( 0 ),
  mBaseUrl( baseUrl ),
  mRes( res )
{
  kdDebug() << k_funcinfo << baseUrl << endl;
  readFolders();
}

SloxFolderManager::~SloxFolderManager()
{
  if ( mDownloadJob )
    mDownloadJob->kill();
  QMap<QString, SloxFolder*>::Iterator it;
  for ( it = mFolders.begin(); it != mFolders.end(); ++it )
    delete *it;
  mFolders.clear();
}

void SloxFolderManager::requestFolders()
{
  kdDebug() << k_funcinfo << endl;

  if ( mDownloadJob ) {
    kdDebug() << k_funcinfo << "Download still in progress" << endl;
    return;
  }

  KURL url = mBaseUrl;
  url.setPath( "/servlet/webdav.folders/file.xml" );

  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement( doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement( doc, root, "prop" );
  WebdavHandler::addSloxElement( mRes, doc, prop, "objectmode", "NEW_AND_MODIFIED" );
  WebdavHandler::addSloxElement( mRes, doc, prop, "lastsync", "0" );
  WebdavHandler::addSloxElement( mRes, doc, prop, "foldertype", "PRIVATE" );
  WebdavHandler::addSloxElement( mRes, doc, prop, "foldertype", "PUBLIC" );
  WebdavHandler::addSloxElement( mRes, doc, prop, "foldertype", "SHARED" );
  WebdavHandler::addSloxElement( mRes, doc, prop, "foldertype", "GLOBALADDRESSBOOK" );
  WebdavHandler::addSloxElement( mRes, doc, prop, "foldertype", "INTERNALUSERS" );

  kdDebug() << k_funcinfo << doc.toString( 2 ) << endl;

  mDownloadJob = KIO::davPropFind( url, doc, "0", false );

  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotResult( KIO::Job * ) ) );
}

void SloxFolderManager::slotResult( KIO::Job *job )
{
  kdDebug() << k_funcinfo << endl;

  if ( job->error() ) {
    job->showErrorDialog( 0 );
  } else {
    kdDebug() << k_funcinfo << " success, writing to " << cacheFile() << endl;
    QFile f( cacheFile() );
    if ( !f.open( IO_WriteOnly ) ) {
      kdDebug() << "Unable to open '" << cacheFile() << "'" << endl;
      return;
    }
    QTextStream stream ( &f );
    stream << mDownloadJob->response();
    f.close();
    readFolders();
  }

  mDownloadJob = 0;
  emit foldersUpdated();
}

QString SloxFolderManager::cacheFile() const
{
  QString host = mBaseUrl.host();

  QString file = locateLocal( "cache", "slox/folders_" + host );

  kdDebug() << k_funcinfo << file << endl;

  return file;
}

void SloxFolderManager::readFolders()
{
  kdDebug() << k_funcinfo << endl;

  QFile f( cacheFile() );
  if ( !f.open( IO_ReadOnly ) ) {
    kdDebug() << "Unable to open '" << cacheFile() << "'" << endl;
    requestFolders();
    return;
  }

  QDomDocument doc;
  doc.setContent( &f );

  mFolders.clear();

  QDomNodeList nodes = doc.elementsByTagName( "D:prop" );
  for( uint i = 0; i < nodes.count(); ++i ) {
    QDomElement element = nodes.item(i).toElement();
    QString id = "-1", parentId = "-1"; // OX default folder
    bool def = false;
    QString name, type;
    QDomNode n;
    for( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
      QDomElement e = n.toElement();
      QString tag = e.tagName();
      QString value = e.text();
      if ( tag == "ox:object_id" ) id = value;
      else if ( tag == "ox:folder_id" ) parentId = value;
      else if ( tag == "ox:title" ) name = value;
      else if ( tag == "ox:module" ) type = value;
      else if ( tag == "ox:defaultfolder" ) def = (value == "true");
    }
    if ( id != "-1" && parentId != "-1" ) {
      SloxFolder *folder = new SloxFolder( id, parentId, type, name, def );
      mFolders[id] = folder;
      kdDebug() << k_funcinfo << "Found folder: " << folder->name() << endl;
    }
  }

  // add top-level system folders that are not contained in the folder listing
  SloxFolder *folder = new SloxFolder( "1", "0", "unbound", i18n("Private Folder") );
  mFolders[folder->id()] = folder;
  folder = new SloxFolder( "2", "0", "unbound", i18n("Public Folder") );
  mFolders[folder->id()] = folder;
  folder = new SloxFolder( "3", "0", "unbound", i18n("Shared Folder") );
  mFolders[folder->id()] = folder;
  folder = new SloxFolder( "4", "0", "unbound", i18n("System Folder") );
  mFolders[folder->id()] = folder;
}


#include "sloxfoldermanager.moc"
