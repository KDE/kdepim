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

#include "davgroupwareglobals.h"
#include "webdavhandler.h"
#include "groupwaredataadaptor.h"

#include <libemailfunctions/idmapper.h>
#include <kio/job.h>
#include <kio/davjob.h>
#include <kdebug.h>

#include <qdom.h>

KIO::TransferJob *DAVGroupwareGlobals::createListItemsJob( const KURL &url )
{
  QDomDocument props = WebdavHandler::createItemsAndVersionsPropsRequest();
  kdDebug() << "props = "<< props.toString() << endl;
  return KIO::davPropFind( url, props, "1", false );
}


bool DAVGroupwareGlobals::itemsForDownloadFromList( KPIM::GroupwareDataAdaptor *adaptor, 
    KIO::Job *job, QStringList &currentlyOnServer, QStringList &itemsForDownload )
{
  KIO::DavJob *davjob = dynamic_cast<KIO::DavJob *>(job);
  
  if ( !davjob ) {
    return false;
  }
  QDomDocument doc = davjob->response();

  kdDebug(7000) << " Doc: " << doc.toString() << endl;
  kdDebug(7000) << " IdMapper: " << adaptor->idMapper()->asString() << endl;

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
    const QString &newFingerprint = e.text();
    i++;

    // If the fingerprint is empty, this item does not have an etag, 
    // like for example in exchange the folder itself (which is also returned!)
    if ( newFingerprint.isEmpty() )
      continue;


    bool download = false;
    KURL url ( entry );
    const QString &location = url.path();

    currentlyOnServer << location;
    /* if not locally present, download */
    const QString &localId = adaptor->idMapper()->localId( location );
    kdDebug(5006) << "Looking up remote: " << location << " found: " << localId << endl;
    if ( localId.isEmpty() || !adaptor->localItemExists( localId ) ) {
      //kdDebug(7000) << "Not locally present, download: " << location << endl;
      download = true;
    } else {
      kdDebug(7000) << "Locally present " << endl;
      /* locally present, let's check if it's newer than what we have */
      const QString &oldFingerprint = adaptor->idMapper()->fingerprint( localId );
      if ( oldFingerprint != newFingerprint ) {
        kdDebug(7000) << "Fingerprint changed old: " << oldFingerprint <<
          " new: " << newFingerprint << endl;
        // something changed on the server, let's see if we also changed it locally
        if ( adaptor->localItemHasChanged( localId ) ) {
          // TODO conflict resolution
          kdDebug(7000) << "TODO conflict resolution" << endl;
          download = true;
        } else {
          download = true;
        }
      } else {
        kdDebug(7000) << "Fingerprint did not change, don't download this one " << endl;
      }
    }
    if ( download ) {
      itemsForDownload << entry;
    }
  }

  kdDebug()<<"currentlyOnServer="<<currentlyOnServer.join(", ")<<endl;
  kdDebug()<<"itemsForDownload="<<itemsForDownload.join(", ")<<endl;
  return true;
}
