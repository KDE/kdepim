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
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement(  doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement(  doc, root, "prop" );
  WebdavHandler::addDavElement( doc, prop, "getetag" );
//  WebdavHandler::addDavElement( doc, prop, "contentclass" );
  kdDebug(5800) << "props = "<< doc.toString() << endl;
  return KIO::davPropFind( url, doc, "1", false );
}

KPIM::GroupwareJob::ContentType DAVGroupwareGlobals::contentClass( const QString &contentclass )
{
  if ( contentclass == "urn:content-classes:appointment" )
    return KPIM::GroupwareJob::Appointment;
  if ( contentclass == "urn:content-classes:task" )
    return KPIM::GroupwareJob::Task;
  if ( contentclass == "urn:content-classes:message" )
    return KPIM::GroupwareJob::Message;
  if ( contentclass == "urn:content-classes:person" )
    return KPIM::GroupwareJob::Contact;
  return KPIM::GroupwareJob::Unknown;
}

bool DAVGroupwareGlobals::itemsForDownloadFromList( KPIM::GroupwareDataAdaptor *adaptor,
    KIO::Job *job, QStringList &currentlyOnServer, QMap<QString,KPIM::GroupwareJob::ContentType> &itemsForDownload )
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
  QDomNodeList ctypes = doc.elementsByTagNameNS( "DAV:", "contentclass" );
  int c = entries.count();
  int i = 0;
  while ( i < c ) {
    QDomElement e = entries.item( i ).toElement();
    const QString &entry = e.text();
    e = fingerprints.item( i ).toElement();
    const QString &newFingerprint = e.text();
    e = ctypes.item( i ).toElement();
    const QString &newContentClass = e.text();
    i++;

    // If the fingerprint is empty, this item does not have an etag,
    // like for example in exchange the folder itself (which is also returned!)
    if ( newFingerprint.isEmpty() )
      continue;

    KPIM::GroupwareJob::ContentType type = contentClass( newContentClass );

    bool download = false;
    KURL url ( entry );
    const QString &location = url.path();

    currentlyOnServer << location;
    /* if not locally present, download */
    const QString &localId = adaptor->idMapper()->localId( location );
    kdDebug(5800) << "Looking up remote: " << location << " found: " << localId << endl;
    if ( localId.isEmpty() || !adaptor->localItemExists( localId ) ) {
      //kdDebug(7000) << "Not locally present, download: " << location << endl;
      download = true;
    } else {
      kdDebug(5800) << "Locally present " << endl;
      /* locally present, let's check if it's newer than what we have */
      const QString &oldFingerprint = adaptor->idMapper()->fingerprint( localId );
      if ( oldFingerprint != newFingerprint ) {
        kdDebug(5800) << "Fingerprint changed old: " << oldFingerprint <<
          " new: " << newFingerprint << endl;
        // something changed on the server, let's see if we also changed it locally
        if ( adaptor->localItemHasChanged( localId ) ) {
          // TODO conflict resolution
          kdDebug(5800) << "TODO conflict resolution" << endl;
          download = true;
        } else {
          download = true;
        }
      } else {
        kdDebug(5800) << "Fingerprint did not change, don't download this one " << endl;
      }
    }
    if ( download ) {
      itemsForDownload[ entry ] = type;
    }
  }

  kdDebug(5800)<<"currentlyOnServer="<<currentlyOnServer.join(", ")<<endl;
  kdDebug(5800)<<"itemsForDownload="<<QStringList( itemsForDownload.keys() ).join(", ")<<endl;
  return true;
}
