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

#include "groupwaredataadaptor.h"
#include <kdebug.h>
#include <kio/job.h>
#include <libemailfunctions/idmapper.h>

using namespace KPIM;

GroupwareUploadItem::GroupwareUploadItem( UploadType type ) : mType( type )
{
}

KIO::TransferJob *GroupwareUploadItem::createUploadNewJob(
      GroupwareDataAdaptor *adaptor, const KURL &baseurl )
{
kdDebug()<<"GroupwareUploadItem::createUploadNewJob, baseurl=" << baseurl.url() << endl;
  setUrl( adaptNewItemUrl( adaptor, baseurl ) );
  return createUploadJob( adaptor, baseurl );
}

KURL GroupwareUploadItem::adaptNewItemUrl( GroupwareDataAdaptor *adaptor,
                                           const KURL &baseurl )
{
kdDebug()<<"GroupwareUploadItem::adaptNewItemUrl, baseurl=" << baseurl.url() << endl;
  if ( adaptor ) {
    QString path( adaptor->defaultNewItemName( this ) );
kdDebug() << "path=" << path << endl;
    KURL u( baseurl );
    if ( path.isEmpty() ) return u;
    else {
      u.addPath( path );
kdDebug() << "Final Path for new item: " << u.url() << endl;
      return u;
    }
  } else return baseurl;
}

KIO::TransferJob *GroupwareUploadItem::createUploadJob(
                                GroupwareDataAdaptor *adaptor, const KURL &/*baseurl*/ )
{
  Q_ASSERT( adaptor );
  if ( !adaptor ) return 0;
  const QString dta = data();
  //kdDebug(7000) << "Uploading: " << data << endl;
  KURL upUrl( url() );
  if ( adaptor )
    adaptor->adaptUploadUrl( upUrl );
  kdDebug(7000) << "Uploading to: " << upUrl.prettyURL() << endl;
  KIO::TransferJob *job = KIO::storedPut( dta.utf8(), upUrl, -1, true,
                                          false, false );
  job->addMetaData( "PropagateHttpHeader", "true" );
  if ( adaptor )
    job->addMetaData( "content-type", adaptor->mimeType() );
  return job;
}


GroupwareDataAdaptor::GroupwareDataAdaptor()
  : QObject(), mFolderLister( 0 ), mIdMapper( 0 )
{
}

GroupwareDataAdaptor::~GroupwareDataAdaptor()
{
}

void GroupwareDataAdaptor::setUserPassword( KURL &url )
{
  kdDebug(5800) << "GroupwareDataAdaptor::setUserPassword, mUser="
                << mUser << endl;
  url.setUser( mUser );
  url.setPass( mPassword );
}

FolderLister::Entry::List GroupwareDataAdaptor::defaultFolders()
{
  return FolderLister::Entry::List();
}

KIO::TransferJob *GroupwareDataAdaptor::createUploadJob( const KURL &url,
                                                     GroupwareUploadItem *item )
{
  if ( item )
    return item->createUploadJob( this, url );
  else return 0;
}

KIO::TransferJob *GroupwareDataAdaptor::createUploadNewJob( const KURL &url,
                                                     GroupwareUploadItem *item )
{
kdDebug()<<"GroupwareDataAdaptor::createUploadNewJob, url=" << url.url() << endl;
  if ( item )
    return item->createUploadNewJob( this, url );
  else return 0;
}

void GroupwareDataAdaptor::processDownloadListItem( const QString &entry,
        const QString &newFingerprint, KPIM::GroupwareJob::ContentType type )
{
  bool download = false;
  KURL url ( entry );
  const QString &location = url.path();

  emit itemOnServer( location );
  // if not locally present, download
  const QString &localId = idMapper()->localId( location );
  kdDebug(5800) << "Looking up remote: " << location
                << " found: " << localId << endl;
  if ( localId.isEmpty() || !localItemExists( localId ) ) {
    //kdDebug(7000) << "Not locally present, download: " << location << endl;
    download = true;
  } else {
    kdDebug(5800) << "Locally present " << endl;
    // locally present, let's check if it's newer than what we have
    const QString &oldFingerprint = idMapper()->fingerprint( localId );
      if ( oldFingerprint != newFingerprint ) {
      kdDebug(5800) << "Fingerprint changed old: " << oldFingerprint <<
        " new: " << newFingerprint << endl;
      // something changed on the server, check if we also changed it locally
      if ( localItemHasChanged( localId ) ) {
        // TODO conflict resolution
        kdDebug(5800) << "TODO conflict resolution" << endl;
        download = true;
      } else {
        download = true;
      }
    } else {
      kdDebug(5800) << "Fingerprint not changed, don't download this " << endl;
    }
  }
  if ( download ) {
    emit itemToDownload( entry, type );
  }
}

bool GroupwareDataAdaptor::interpretRemoveJob( KIO::Job *job, const QString &/*jobData*/ )
{
  if ( !job ) return false;
  KIO::DeleteJob *deljob = dynamic_cast<KIO::DeleteJob*>(job);
  bool error = job->error();
  const QString err = job->errorString();

  if ( deljob ) {
    KURL::List urls( deljob->urls() );
    for ( KURL::List::Iterator it = urls.begin(); it != urls.end(); ++it ) {
      if ( error ) {
        emit itemDeletionError( (*it).url(), err );
      } else {
        emit itemDeleted( QString::null, (*it).url() );
      }
    }
    return true;
  } else {
    return false;
  }
}


bool GroupwareDataAdaptor::interpretUploadJob( KIO::Job *job, const QString &/*jobData*/ )
{
  kdDebug(7000) << "GroupwareDataAdaptor::interpretUploadJob " << endl;
  KIO::TransferJob *trfjob = dynamic_cast<KIO::TransferJob*>(job);
  bool error = job->error();
  const QString err = job->errorString();

  if ( trfjob ) {
    KURL url( trfjob->url() );
    if ( error ) {
      emit itemUploadError( url.url(), err );
    } else {
      // We don't know the local id here (and we don't want to extract it from
      // the idMapper, that's the task of the receiver
      emit itemUploaded( QString::null, url.url() );
    }
    return true;
  } else {
    return false;
  }
}

bool GroupwareDataAdaptor::interpretUploadNewJob( KIO::Job *job, const QString &/*jobData*/ )
{
// TODO: How does the incidence mapper know the old/new ids???
  kdDebug(7000) << "GroupwareDataAdaptor::interpretUploadNewJob " << endl;
  KIO::TransferJob *trfjob = dynamic_cast<KIO::TransferJob*>(job);
  bool error = job->error();
  const QString err = job->errorString();

  if ( trfjob ) {
    KURL url( trfjob->url() );
    if ( error ) {
      emit itemUploadNewError( url.url(), err );
    } else {
      // We don't know the local id here (and we don't want to extract it from
      // the idMapper, that's the task of the receiver
      emit itemUploadedNew( QString::null, url.url() );
    }
    return true;
  } else {
    return false;
  }
}


#include "groupwaredataadaptor.moc"
