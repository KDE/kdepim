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

#include "groupwareuploadjob.h"

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

using namespace KPIM;

GroupwareUploadJob::GroupwareUploadJob( GroupwareDataAdaptor *adaptor )
  : GroupwareJob( adaptor )
{
}

void GroupwareUploadJob::run()
{
  deleteItems();
}

void GroupwareUploadJob::deleteItems()
{
  if ( mUrlsForDeletion.isEmpty() ) {
    uploadItem();
  } else {
    kdDebug(7000) << " URLs for deletion: " << mUrlsForDeletion << endl;

    mDeletionJob = KIO::del( mUrlsForDeletion, false, false );
    connect( mDeletionJob, SIGNAL( result( KIO::Job* ) ),
             SLOT( slotDeletionResult( KIO::Job* ) ) );
  }
}

void GroupwareUploadJob::slotDeletionResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug(5006) << "slotDeletionResult failed " << endl;
    error( job->errorString() );
  } else {
    kdDebug(5006) << "slotDeletionResult successfull " << endl;
    
    QStringList::Iterator it = mIncidencesForDeletion.begin();
    for ( ; it != mIncidencesForDeletion.end(); ++it ) {
      KURL url( *it );
      const QString &remote = url.path();
      const QString &local = adaptor()->idMapper()->localId( remote );
      if ( !local.isEmpty() ) {
        adaptor()->deleteItem( local );
      }
    }
    
    uploadItem();
  }
}

void GroupwareUploadJob::uploadItem()
{
  if ( !mDataForUpload.isEmpty() ) {
    QString uid = adaptor()->extractUid( mDataForUpload.front() );
    const QString remote = adaptor()->idMapper()->remoteId( uid );
    KURL url;
    if ( !remote.isEmpty() ) {
      url = KURL( mBaseUrl );
      url.setPath( remote );
    } else {
      url = KURL( adaptor()->folderLister()->writeDestinationId() );
      adaptor()->adaptUploadUrl( url );
      kdDebug() << "Put new URL: " << url.url() << endl;
    }
    const QString inc = mDataForUpload.front();
    //kdDebug(7000) << "Uploading to: " << inc << endl;
    //kdDebug(7000) << "Uploading: " << url.prettyURL() << endl;
    mUploadJob = KIO::storedPut( inc.utf8(), url, -1, true, false, false );
    mUploadJob->addMetaData( "PropagateHttpHeader", "true" );
    mUploadJob->addMetaData( "content-type", adaptor()->mimeType() );
    connect( mUploadJob, SIGNAL( result( KIO::Job * ) ),
      SLOT( slotUploadJobResult( KIO::Job * ) ) );

    mUploadProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(),
      adaptor()->uploadProgressMessage() );
    connect( mUploadProgress,
      SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
      SLOT( cancelSave() ) );
  } else {
    if ( mUploadProgress ) {
      mUploadProgress->setComplete();
      mUploadProgress = 0;
    }
    success();
  }
}

void GroupwareUploadJob::slotUploadJobResult( KIO::Job *job )
{
  kdDebug(7000) << " slotUploadJobResult " << endl;
  if ( job->error() ) {
    error( job->errorString() );
  } else {
    QString uid = adaptor()->extractUid( mDataForUpload.front() );
    adaptor()->clearChange( uid );

    const QString& headers = job->queryMetaData( "HTTP-Headers" );
    const QString& etag = WebdavHandler::getEtagFromHeaders( headers );
    adaptor()->idMapper()->setFingerprint( uid, etag );

    mDataForUpload.pop_front();
  }

  if ( mUploadProgress ) {
    mUploadProgress->incCompletedItems();
    mUploadProgress->updateProgress();
  }
  mUploadJob = 0;

  uploadItem();
}

void GroupwareUploadJob::kill()
{
  cancelSave();
}

void GroupwareUploadJob::cancelSave()
{
  if ( mUploadJob ) mUploadJob->kill();
  mUploadJob = 0;
  if ( mUploadProgress ) mUploadProgress->setComplete();
  mUploadProgress = 0;
}

#include "groupwareuploadjob.moc"
