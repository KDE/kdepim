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
#include <calendaradaptor.h>
#include <addressbookadaptor.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>
#include <libkcal/resourcecached.h>
#include <libemailfunctions/idmapper.h>

#include <kabc/vcardconverter.h>
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
//  WebdavHandler::addDavElement( doc, prop, "getcontenttype" );
  kdDebug(5800) << "props = "<< doc.toString() << endl;
  return KIO::davPropFind( url, doc, "1", false );
}

KPIM::GroupwareJob::ContentType DAVGroupwareGlobals::getContentType( const QDomElement &prop )
{
  QDomElement ctype = prop.namedItem("getcontenttype").toElement();
  if ( ctype.isNull() ) return KPIM::GroupwareJob::Unknown;
  const QString &type = ctype.text();
kdDebug()<<"Found content type: "<<type<<endl;
  /// TODO: Not yet implemented in OGo!
  return KPIM::GroupwareJob::Unknown;
}

bool DAVGroupwareGlobals::interpretListItemsJob( KPIM::GroupwareDataAdaptor *adaptor,
    KIO::Job *job )
{
  KIO::DavJob *davjob = dynamic_cast<KIO::DavJob *>(job);

  if ( !davjob ) {
    return false;
  }
  QDomDocument doc = davjob->response();

  kdDebug(7000) << " Doc: " << doc.toString() << endl;
  kdDebug(7000) << " IdMapper: " << adaptor->idMapper()->asString() << endl;

  QDomElement docElem = doc.documentElement();
  QDomNode n = docElem.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    n = n.nextSibling();
    if ( e.isNull() )
      continue;

    const QString &entry = e.namedItem("href").toElement().text();
    QDomElement propstat = e.namedItem("propstat").toElement();
    if ( propstat.isNull() )
      continue;
    QDomElement prop = propstat.namedItem( "prop" ).toElement();
    if ( prop.isNull() )
      continue;
    QDomElement elem = prop.namedItem("getetag").toElement();
    const QString &newFingerprint = elem.text();
    if ( elem.isNull() || newFingerprint.isEmpty() )
      continue;

    KPIM::GroupwareJob::ContentType type = getContentType( prop );

    adaptor->processDownloadListItem( entry, newFingerprint, type );

/*    bool download = false;
    KURL url ( entry );
    const QString &location = url.path();

    currentlyOnServer << location;
    // if not locally present, download
    const QString &localId = adaptor->idMapper()->localId( location );
    kdDebug(5800) << "Looking up remote: " << location << " found: " << localId << endl;
    if ( localId.isEmpty() || !adaptor->localItemExists( localId ) ) {
      //kdDebug(7000) << "Not locally present, download: " << location << endl;
      download = true;
    } else {
      kdDebug(5800) << "Locally present " << endl;
      // locally present, let's check if it's newer than what we have
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
  */
  }


/*  QDomNodeList entries = doc.elementsByTagNameNS( "DAV:", "href" );
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
    // if not locally present, download
    const QString &localId = adaptor->idMapper()->localId( location );
    kdDebug(5800) << "Looking up remote: " << location << " found: " << localId << endl;
    if ( localId.isEmpty() || !adaptor->localItemExists( localId ) ) {
      //kdDebug(7000) << "Not locally present, download: " << location << endl;
      download = true;
    } else {
      kdDebug(5800) << "Locally present " << endl;
      // locally present, let's check if it's newer than what we have
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
*/
  return true;
}

QString DAVGroupwareGlobals::extractFingerprint( KIO::Job *job, const QString &/*jobData*/ )
{
  const QString& headers = job->queryMetaData( "HTTP-Headers" );
  return WebdavHandler::getEtagFromHeaders( headers );
}

bool DAVGroupwareGlobals::interpretCalendarDownloadItemsJob( KCal::CalendarAdaptor *adaptor, KIO::Job *job, const QString &jobData )
{
kdDebug(5800) << "DAVGroupwareGlobals::interpretCalendarDownloadItemsJob, iCalendar=" << endl;
kdDebug(5800) << jobData << endl;
  if ( !adaptor || !job ) return false;
  KCal::CalendarLocal calendar;
  KCal::ICalFormat ical;
  calendar.setTimeZoneId( adaptor->resource()->timeZoneId() );
  KCal::Incidence::List incidences;
  if ( ical.fromString( &calendar, jobData ) ) {
    KCal::Incidence::List raw = calendar.rawIncidences();
    KCal::Incidence::List::Iterator it = raw.begin();
    if ( raw.count() != 1 ) {
      kdError() << "Parsed iCalendar does not contain exactly one event." << endl;
      return false;
    }
    
    KCal::Incidence *inc = (raw.front())->clone();
    if ( !inc ) return false;
    KIO::SimpleJob *sjob = dynamic_cast<KIO::SimpleJob *>(job);
    QString remoteId( QString::null );
    if ( sjob ) remoteId = sjob->url().path();
    QString fingerprint = extractFingerprint( job, jobData );
    adaptor->calendarItemDownloaded( inc, inc->uid(), remoteId, fingerprint,
                                     sjob->url().prettyURL() );
    return true;
  } else {
    kdError() << "Unable to parse iCalendar" << endl;
  }
  return false;
}

bool DAVGroupwareGlobals::interpretAddressBookDownloadItemsJob( KABC::AddressBookAdaptor *adaptor, KIO::Job *job, const QString &jobData )
{
kdDebug(5800) << "DAVGroupwareGlobals::interpretAddressBookDownloadItemsJob, vCard=" << endl;
kdDebug(5800) << jobData << endl;
  if ( !adaptor || !job ) return false;
  
  KABC::VCardConverter conv;
  KABC::Addressee::List addrs( conv.parseVCards( jobData ) );
  
  if ( addrs.count() != 1 ) {
    kdError() << "Parsed vCard does not contain exactly one addressee." << endl;
    return false;
  }

  KABC::Addressee a = addrs.first();

  KIO::SimpleJob *sjob = dynamic_cast<KIO::SimpleJob*>(job);
  QString remoteId( QString::null );
  if ( sjob ) remoteId = sjob->url().path();
  QString fingerprint = extractFingerprint( job, jobData );
  adaptor->addressbookItemDownloaded( a, a.uid(), remoteId, fingerprint,
                                      sjob->url().prettyURL() );
  return true;
}

