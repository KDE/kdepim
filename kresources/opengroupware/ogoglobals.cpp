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
#include <webdavhandler.h>
#include <libemailfunctions/idmapper.h>
#include <calendaradaptor.h>
#include <addressbookadaptor.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>
#include <libkcal/resourcecached.h>
#include <kabc/vcardconverter.h>

#include <kdebug.h>
#include <kio/davjob.h>
#include <kio/job.h>

QString OGoGlobals::extractFingerprint( KIO::Job *job, const QString &/*jobData*/ )
{
  const QString& headers = job->queryMetaData( "HTTP-Headers" );
  return WebdavHandler::getEtagFromHeaders( headers );
}


KPIM::GroupwareJob::ContentType OGoGlobals::getContentType( const QDomElement &prop )
{
  QDomElement ctype = prop.namedItem("getcontenttype").toElement();
  if ( ctype.isNull() ) return KPIM::GroupwareJob::Unknown;
  const QString &type = ctype.text();
kdDebug()<<"Found content type: "<<type<<endl;
  /// TODO: Not yet implemented in OGo!
  return KPIM::GroupwareJob::Unknown;
}


KPIM::FolderLister::FolderType OGoGlobals::getFolderType( const QDomNode &folderNode )
{
  QDomNode n4;
kdDebug()<<"OGoGlobals::getFolderType(...)"<<endl;
  for( n4 = folderNode.firstChild(); !n4.isNull(); n4 = n4.nextSibling() ) {
    QDomElement e = n4.toElement();

    if ( e.tagName() == "resourcetype" ) {
      if ( !e.namedItem( "vevent-collection" ).isNull() )
        return KPIM::FolderLister::CalendarFolder;
      if ( !e.namedItem( "vtodo-collection" ).isNull() )
        return KPIM::FolderLister::TasksFolder;
      if ( !e.namedItem( "vjournal-collection" ).isNull() )
        return KPIM::FolderLister::JournalsFolder;
      if ( !e.namedItem( "vcard-collection" ).isNull() )
        return KPIM::FolderLister::ContactsFolder;
      if ( !e.namedItem( "collection" ).isNull() )
        return KPIM::FolderLister::Folder;
    }
  }
  return KPIM::FolderLister::Unknown;
}

bool OGoGlobals::getFolderHasSubs( const QDomNode &folderNode )
{
  // a folder is identified by the collection item in the resourcetype:
  // <a:resourcetype xmlns:a="DAV:"><a:collection xmlns:a="DAV:"/>...</a:resourcetype>
  QDomElement e = folderNode.namedItem("resourcetype").toElement();
  if ( !e.namedItem( "collection" ).isNull() )
    return true;
  else return false;
}




KIO::Job *OGoGlobals::createListFoldersJob( const KURL &url )
{
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement(  doc, doc, "d:propfind" );
  QDomElement prop = WebdavHandler::addElement(  doc, root, "d:prop" );
  WebdavHandler::addElement( doc, prop, "d:displayname" );
  WebdavHandler::addElement( doc, prop, "d:resourcetype" );
//  WebdavHandler::addElement( doc, prop, "d:hassubs" );

  kdDebug(7000) << "props: " << doc.toString() << endl;
  return KIO::davPropFind( url, doc, "1", false );
}


KIO::TransferJob *OGoGlobals::createListItemsJob( const KURL &url )
{
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement(  doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement(  doc, root, "prop" );
  WebdavHandler::addDavElement( doc, prop, "getetag" );
//  WebdavHandler::addDavElement( doc, prop, "getcontenttype" );
  kdDebug(5800) << "props = "<< doc.toString() << endl;
  return KIO::davPropFind( url, doc, "1", false );
}


KIO::TransferJob *OGoGlobals::createDownloadJob( KPIM::GroupwareDataAdaptor *adaptor,
                    const KURL &url, KPIM::GroupwareJob::ContentType /*ctype*/ )
{
kdDebug()<<"OGoGlobals::createDownloadJob, url="<<url.url()<<endl;
  KIO::TransferJob *job = KIO::get( url, false, false );
  if ( adaptor ) {
    QString mt = adaptor->mimeType();
    job->addMetaData( "accept", mt );
  }
  job->addMetaData( "PropagateHttpHeader", "true" );
  return job;
}


KIO::Job *OGoGlobals::createRemoveJob( KPIM::GroupwareDataAdaptor *adaptor, const KURL &uploadurl,
       const KPIM::GroupwareUploadItem::List &deletedItems )
{
  QStringList urls;
  KPIM::GroupwareUploadItem::List::const_iterator it;
  kdDebug(5800) << " OGoGlobals::createRemoveJob, BaseURL="<<uploadurl.url()<<endl;
  for ( it = deletedItems.constBegin(); it != deletedItems.constEnd(); ++it ) {
    //kdDebug(7000) << "Delete: " << endl << format.toICalString(*it) << endl;
    KURL url( (*it)->url() );
    if ( adaptor ) {
      adaptor->adaptUploadUrl( url );
    }
/*    KURL url( uploadurl );
    url.setPath( (*it)->url().path() );
    if ( !(*it)->url().isEmpty() )*/
    if ( !url.isEmpty() ) {
kdDebug() << "Deleting item at "<< url.url() << endl;
      urls << url.url();
    }
    kdDebug(5700) << "Delete (Mod) : " <<   url.url() << endl;
  }
  return KIO::del( urls, false, false );
}




bool OGoGlobals::interpretListItemsJob( KPIM::GroupwareDataAdaptor *adaptor,
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
  }

  return true;
}


bool OGoGlobals::interpretCalendarDownloadItemsJob( KCal::CalendarAdaptor *adaptor,
                                         KIO::Job *job, const QString &jobData )
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


bool OGoGlobals::interpretAddressBookDownloadItemsJob(
      KABC::AddressBookAdaptor *adaptor, KIO::Job *job, const QString &jobData )
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


