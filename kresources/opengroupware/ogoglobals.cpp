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
#include <kdebug.h>

#include <kio/job.h>

KIO::TransferJob *OGoGlobals::createDownloadItemJob( KPIM::GroupwareDataAdaptor *adaptor, const KURL &url, KPIM::GroupwareJob::ContentType /*ctype*/ )
{
kdDebug()<<"OGoGlobals::createDownloadItemJob, url="<<url.url()<<endl;
  KIO::TransferJob *job = KIO::get( url, false, false );
  if ( adaptor ) {
    QString mt = adaptor->mimeType();
    job->addMetaData( "accept", mt );
  }
  job->addMetaData( "PropagateHttpHeader", "true" );
  return job;
}

QString OGoGlobals::extractFingerprint( KIO::TransferJob *job,
          const QString &/*rawText*/ )
{
  const QString& headers = job->queryMetaData( "HTTP-Headers" );
  return WebdavHandler::getEtagFromHeaders( headers );
}

KIO::Job *OGoGlobals::createRemoveItemsJob( const KURL &uploadurl,
       KPIM::GroupwareUploadItem::List deletedItems )
{
  QStringList urls;
  KPIM::GroupwareUploadItem::List::iterator it;
  kdDebug(5800) << " OGoGlobals::createRemoveItemsJob: , URL="<<uploadurl.url()<<endl;
  for ( it = deletedItems.begin(); it != deletedItems.end(); ++it ) {
    //kdDebug(7000) << "Delete: " << endl << format.toICalString(*it) << endl;
    KURL url( uploadurl );
    url.setPath( (*it)->url().path() );
    if ( !(*it)->url().isEmpty() )
      urls << url.url();
    kdDebug(5700) << "Delete (Mod) : " <<   url.url() << endl;
  }
  return KIO::del( urls, false, false );
}
