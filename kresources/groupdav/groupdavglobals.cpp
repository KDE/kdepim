/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "groupdavglobals.h"
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

QString GroupDavGlobals::extractFingerprint( KIO::Job *job, const QString &/*jobData*/ )
{
  const QString& headers = job->queryMetaData( "HTTP-Headers" );
  return WebdavHandler::getEtagFromHeaders( headers );
}


KPIM::FolderLister::ContentType GroupDavGlobals::getContentType( const QDomElement &prop )
{
  QDomElement ctype = prop.namedItem("getcontenttype").toElement();
  if ( ctype.isNull() ) return KPIM::FolderLister::Unknown;
  const QString &type = ctype.text();
kdDebug()<<"Found content type: "<<type<<endl;
  /// TODO: Not yet implemented in GroupDav!
  return KPIM::FolderLister::Unknown;
}


KPIM::FolderLister::ContentType GroupDavGlobals::getContentType( const QDomNode &folderNode )
{
  QDomNode n4;
kdDebug()<<"GroupDavGlobals::getContentType(...)"<<endl;
  int type = KPIM::FolderLister::Unknown;
  for( n4 = folderNode.firstChild(); !n4.isNull(); n4 = n4.nextSibling() ) {
    QDomElement e = n4.toElement();

    if ( e.tagName() == "resourcetype" ) {
      if ( !e.namedItem( "vevent-collection" ).isNull() )
        type |= KPIM::FolderLister::Event;
      if ( !e.namedItem( "vtodo-collection" ).isNull() )
        type |= KPIM::FolderLister::Todo;
      if ( !e.namedItem( "vjournal-collection" ).isNull() )
        type |= KPIM::FolderLister::Journal;
      if ( !e.namedItem( "vcard-collection" ).isNull() )
        type |= KPIM::FolderLister::Contact;
      if ( (type == KPIM::FolderLister::Unknown) &&
           !( e.namedItem( "collection" ).isNull() ) )
        type |= KPIM::FolderLister::Folder;
    }
  }
  return (KPIM::FolderLister::ContentType)type;
}

bool GroupDavGlobals::getFolderHasSubs( const QDomNode &folderNode )
{
  // a folder is identified by the collection item in the resourcetype:
  // <a:resourcetype xmlns:a="DAV:"><a:collection xmlns:a="DAV:"/>...</a:resourcetype>
  QDomElement e = folderNode.namedItem("resourcetype").toElement();
  if ( !e.namedItem( "collection" ).isNull() )
    return true;
  else return false;
}




KIO::Job *GroupDavGlobals::createListFoldersJob( const KURL &url )
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


KIO::TransferJob *GroupDavGlobals::createListItemsJob( const KURL &url )
{
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement(  doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement(  doc, root, "prop" );
  WebdavHandler::addDavElement( doc, prop, "getetag" );
//  WebdavHandler::addDavElement( doc, prop, "getcontenttype" );
  kdDebug(5800) << "props = "<< doc.toString() << endl;
  KIO::TransferJob *job = KIO::davPropFind( url, doc, "1", false );
  if ( job ) {
    job->addMetaData( "accept", "text/xml" );
    job->addMetaData( "customHTTPHeader", "accept-encoding: " );
  }
  return job;
}


KIO::TransferJob *GroupDavGlobals::createDownloadJob( KPIM::GroupwareDataAdaptor *adaptor,
                    const KURL &url, KPIM::FolderLister::ContentType /*ctype*/ )
{
kdDebug()<<"GroupDavGlobals::createDownloadJob, url="<<url.url()<<endl;
  KIO::TransferJob *job = KIO::get( url, false, false );
  if ( adaptor ) {
    QString mt = adaptor->mimeType();
    job->addMetaData( "accept", mt );
  }
  job->addMetaData( "PropagateHttpHeader", "true" );
  return job;
}


KIO::Job *GroupDavGlobals::createRemoveJob( KPIM::GroupwareDataAdaptor *adaptor, const KURL &/*uploadurl*/,
       KPIM::GroupwareUploadItem *deletedItem )
{
  if ( !deletedItem ) return 0;
  //kdDebug(7000) << "Delete: " << endl << format.toICalString(*it) << endl;
  KURL url( deletedItem->url() );
  if ( adaptor ) {
    adaptor->adaptUploadUrl( url );
  }
  KIO::Job *delJob = 0;
  if ( !url.isEmpty() ) {
    kdDebug(5700) << "Delete: " <<   url.url() << endl;
    delJob = KIO::del( url, false, false );
  }
  if ( delJob && adaptor && adaptor->idMapper() ) {
    kdDebug(5800 ) << "Adding If-Match metadata: " << adaptor->idMapper()->fingerprint( deletedItem->uid() ) << endl;
    delJob->addMetaData( "customHTTPHeader", "If-Match: " + adaptor->idMapper()->fingerprint( deletedItem->uid() ) );
  }
  return delJob;

/*  QStringList urls;
  KPIM::GroupwareUploadItem::List::const_iterator it;
  kdDebug(5800) << " GroupDavGlobals::createRemoveJob, BaseURL="<<uploadurl.url()<<endl;
  for ( it = deletedItems.constBegin(); it != deletedItems.constEnd(); ++it ) {
    //kdDebug(7000) << "Delete: " << endl << format.toICalString(*it) << endl;
    KURL url( (*it)->url() );
    if ( adaptor ) {
      adaptor->adaptUploadUrl( url );
    }*/
/*    KURL url( uploadurl );
    url.setPath( (*it)->url().path() );
    if ( !(*it)->url().isEmpty() )*/
/*    if ( !url.isEmpty() ) {
kdDebug() << "Deleting item at "<< url.url() << endl;
      urls << url.url();
    }
    kdDebug(5700) << "Delete (Mod) : " <<   url.url() << endl;
  }
  return KIO::del( urls, false, false );*/
}




bool GroupDavGlobals::interpretListItemsJob( KPIM::GroupwareDataAdaptor *adaptor,
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

    const KURL &entry( e.namedItem("href").toElement().text() );
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

    KPIM::FolderLister::ContentType type = getContentType( prop );

    adaptor->processDownloadListItem( entry, newFingerprint, type );
  }

  return true;
}


bool GroupDavGlobals::interpretCalendarDownloadItemsJob( KCal::CalendarAdaptor *adaptor,
                                         KIO::Job *job, const QString &jobData )
{
kdDebug(5800) << "GroupDavGlobals::interpretCalendarDownloadItemsJob, iCalendar=" << endl;
kdDebug(5800) << jobData << endl;
  if ( !adaptor || !job ) return false;
  KCal::CalendarLocal calendar( QString::fromLatin1("UTC") );
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
    KURL remoteId;
    if ( sjob ) remoteId = sjob->url();
    QString fingerprint = extractFingerprint( job, jobData );
    adaptor->calendarItemDownloaded( inc, inc->uid(), remoteId, fingerprint,
                                     remoteId.prettyURL() );
    return true;
  } else {
    kdError() << "Unable to parse iCalendar" << endl;
  }
  return false;
}


bool GroupDavGlobals::interpretAddressBookDownloadItemsJob(
      KABC::AddressBookAdaptor *adaptor, KIO::Job *job, const QString &jobData )
{
kdDebug(5800) << "GroupDavGlobals::interpretAddressBookDownloadItemsJob, vCard=" << endl;
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
  KURL remoteId;
  if ( sjob ) remoteId = sjob->url();
  QString fingerprint = extractFingerprint( job, jobData );
  adaptor->addressbookItemDownloaded( a, a.uid(), remoteId, fingerprint,
                                      remoteId.prettyURL() );
  return true;
}


