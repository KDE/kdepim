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

#include "ogoglobals.h"
#include "groupwaredataadaptor.h"
#include "webdavhandler.h"
#include <libemailfunctions/idmapper.h>

#include <kio/job.h>

KIO::TransferJob *OGoGlobals::createDownloadItemJob( KPIM::GroupwareDataAdaptor *adaptor, const KURL &url )
{
  KIO::TransferJob *job = KIO::get( url, false, false );
  job->addMetaData( "accept", adaptor->mimeType() );
  job->addMetaData( "PropagateHttpHeader", "true" );
  return job;
}

QString OGoGlobals::extractFingerprint( KIO::TransferJob *job, 
          const QString &/*rawText*/ ) 
{
  const QString& headers = job->queryMetaData( "HTTP-Headers" );
  return WebdavHandler::getEtagFromHeaders( headers );
}

void OGoGlobals::updateFingerprintId( KPIM::GroupwareDataAdaptor *adaptor, 
         KIO::TransferJob *trfjob, KPIM::GroupwareUploadItem *item )
{
  Q_ASSERT( item && trfjob );
  if ( !item || !trfjob ) return;
  QString uid( item->uid() );
  adaptor->clearChange( uid );

  const QString& headers = trfjob->queryMetaData( "HTTP-Headers" );
  const QString& etag = WebdavHandler::getEtagFromHeaders( headers );

  QString remoteId = trfjob->url().path();
  remoteId.truncate( remoteId.findRev( "/" )+1 );
  remoteId = remoteId + etag.left( etag.findRev( ":" ) ) + ".ics";
  adaptor->idMapper()->setRemoteId( uid, remoteId );
  adaptor->idMapper()->setFingerprint( uid, etag );
/*
  kdDebug() << "Setting remoteID for: " << uid << " to: " << remoteId << endl;
  kdDebug() << "Setting etag for: " << uid << " to: " << etag << endl;
  kdDebug() << adaptor()->idMapper()->asString() << endl;
*/
}

