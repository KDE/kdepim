/*
    This file is part of kdepim.

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

#include "groupwaredownloadjob.h"

#include "webdavhandler.h"
#include "folderlister.h"
#include "groupwaredataadaptor.h"

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kio/job.h>
#include <kio/davjob.h>

#include <qapplication.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qdom.h>

using namespace KCal;
using namespace KPIM;

GroupwareDownloadJob::GroupwareDownloadJob( GroupwareDataAdaptor *adaptor )
  : GroupwareJob( adaptor )
{
  QTimer::singleShot( 0, this, SLOT( run() ) );
}

void GroupwareDownloadJob::run()
{
  mFoldersForDownload = mAdaptor->folderLister()->activeFolderIds();

  mItemsForDownload.clear();
  mCurrentlyOnServer.clear();

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(),
    mAdaptor->downloadProgressMessage() );
  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );

  listItems();
}

void GroupwareDownloadJob::listItems()
{
  if ( mFoldersForDownload.isEmpty() ) {
    if ( mProgress ) {
      mProgress->setTotalItems( mItemsForDownload.count() );
      mProgress->setCompletedItems( 1 );
      mProgress->updateProgress();
    }

    /* Delete incidences no longer present on the server */
    deleteIncidencesGoneFromServer();
    downloadItem();
  } else {
    QDomDocument props = WebdavHandler::createItemsAndVersionsPropsRequest();

    //kdDebug(7000) << "listItems: " << url.prettyURL() << endl;
    //kdDebug(7000) << "props: " << props.toString() << endl;

    KURL url = mFoldersForDownload.front();
    mFoldersForDownload.pop_front();

    mAdaptor->setUserPassword( url );
    mAdaptor->adaptDownloadUrl( url );

    kdDebug() << "OpenGroupware::listIncidences(): " << url << endl;

    mListEventsJob = KIO::davPropFind( url, props, "1", false );

    connect( mListEventsJob, SIGNAL( result( KIO::Job * ) ),
             SLOT( slotListJobResult( KIO::Job * ) ) );
  }
}

void GroupwareDownloadJob::slotListJobResult( KIO::Job *job )
{
  kdDebug() << "GroupwareDownloadJob::slotListJobResult(): " << endl;

  if ( job->error() ) {
    if ( mProgress ) {
      mProgress->setComplete();
      mProgress = 0;
    }
    error( job->errorString() );
  } else {
    QDomDocument doc = mListEventsJob->response();

    //kdDebug(7000) << " Doc: " << doc.toString() << endl;

    //kdDebug(7000) << idMapper().asString() << endl;
    QDomNodeList entries = doc.elementsByTagNameNS( "DAV:", "href" );
    QDomNodeList fingerprints = doc.elementsByTagNameNS( "DAV:", "getetag" );
    int c = entries.count();
    int i = 0;
    while ( i < c ) {
      QDomNode node = entries.item( i );
      QDomElement e = node.toElement();
      const QString &entry = e.text();
      node = fingerprints.item( i );
      e = node.toElement();
      i++;

      bool download = false;
      KURL url ( entry );
      const QString &location = url.path();
      const QString &newFingerprint = e.text();

      mCurrentlyOnServer << location;
      /* if not locally present, download */
      const QString &localId = mAdaptor->idMapper()->localId( location );
      if ( !localId.isNull() && mAdaptor->localItemExists( localId ) ) {
         //kdDebug(7000) << "Not locally present, download: " << location << endl;
        download = true;
      } else {
         //kdDebug(7000) << "Locally present " << endl;
        /* locally present, let's check if it's newer than what we have */
        const QString &oldFingerprint =
          mAdaptor->idMapper()->fingerprint( localId );
        if ( oldFingerprint != newFingerprint ) {
          kdDebug(7000) << "Fingerprint changed old: " << oldFingerprint <<
            " new: " << newFingerprint << endl;
          // something changed on the server, let's see if we also changed it locally
          if ( mAdaptor->localItemHasChanged( localId ) ) {
            // TODO conflict resolution
            kdDebug(7000) << "TODO conflict resolution" << endl;
            download = true;
          } else {
            download = true;
          }
        } else {
          //kdDebug(7000) << "Fingerprint did not change, don't download this one " << endl;
        }
      }
      if ( download ) {
        mItemsForDownload << entry;
      }
    }
  }

  mListEventsJob = 0;

  listItems();
}

void GroupwareDownloadJob::deleteIncidencesGoneFromServer()
{
  QMap<QString, QString> remoteIds( mAdaptor->idMapper()->remoteIdMap() );
  QStringList::ConstIterator it = mCurrentlyOnServer.begin();
  while ( it != mCurrentlyOnServer.end() ) {
    remoteIds.remove( (*it) );
    ++it;
  }
  QMap<QString, QString>::ConstIterator it2;
  for (it2 = remoteIds.begin(); it2 != remoteIds.end(); ++it2) {
    mAdaptor->deleteItem( remoteIds[ it2.key() ] );
  }
}

void GroupwareDownloadJob::downloadItem()
{
  kdDebug(7000) << " downloadItem " << endl;
  if ( !mItemsForDownload.isEmpty() ) {
    const QString entry = mItemsForDownload.front();
    mItemsForDownload.pop_front();

    KURL url( entry );
    url.setProtocol( "webdav" );
    mCurrentGetUrl = url.url(); // why can't I ask the job?
    mAdaptor->setUserPassword( url );

    mJobData = QString::null;

    mDownloadJob = KIO::get( url, false, false );
    mDownloadJob->addMetaData( "accept", mAdaptor->mimeType() );
    mDownloadJob->addMetaData( "PropagateHttpHeader", "true" );
    connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
        SLOT( slotJobResult( KIO::Job * ) ) );
    connect( mDownloadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
        SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );
  } else {
    if ( mProgress ) mProgress->setComplete();
    mProgress = 0;
    success();
  }
}

void GroupwareDownloadJob::slotJobResult( KIO::Job *job )
{
  kdDebug() << "OpenGroupware::slotJobResult(): " << endl;

  if ( job->error() ) {
    error( job->errorString() );
  } else {
    const QString& headers = job->queryMetaData( "HTTP-Headers" );
    const QString& etag = WebdavHandler::getEtagFromHeaders( headers );

    const QString &remote = KURL( mCurrentGetUrl ).path();
    const QString &local = mAdaptor->idMapper()->localId( remote );

    QString id = mAdaptor->addItem( mJobData, local, remote );

    if ( id.isEmpty() ) {
      error( i18n("Error parsing calendar data.") );
    } else {
      if ( local.isEmpty() ) {
        mAdaptor->idMapper()->setRemoteId( id, remote );
      }
      mAdaptor->idMapper()->setFingerprint( id, etag );
    }
  }

  if ( mProgress ) {
    mProgress->incCompletedItems();
    mProgress->updateProgress();
  }
  mJobData = QString::null;
  mDownloadJob = 0;
  mCurrentGetUrl = QString::null;

  downloadItem();
}

void GroupwareDownloadJob::slotJobData( KIO::Job *, const QByteArray &data )
{
//  kdDebug() << "OpenGroupware::slotJobData()" << endl;

  mJobData.append( data.data() );
}

void GroupwareDownloadJob::kill()
{
  cancelLoad();
}

void GroupwareDownloadJob::cancelLoad()
{
  if ( mDownloadJob ) mDownloadJob->kill();
  mDownloadJob = 0;
  if ( mListEventsJob ) mListEventsJob->kill();
  mListEventsJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
}

#include "groupwaredownloadjob.moc"
