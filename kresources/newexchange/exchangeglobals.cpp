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

#include "exchangeglobals.h"
#include <webdavhandler.h>
#include <groupwaredataadaptor.h>

#include <libemailfunctions/idmapper.h>
#include <kio/job.h>
#include <kio/davjob.h>
#include <kdebug.h>

#include <qdom.h>

QString ExchangeGlobals::extractFingerprint( KIO::TransferJob *job,
          const QString &/*rawText*/ )
{
  KIO::DavJob *davjob = dynamic_cast<KIO::DavJob*>(job);
  if ( !davjob ) return QString::null;

  QDomDocument doc = davjob->response();
//   kdDebug(7000) << " Doc: " << doc.toString() << endl;

  QDomNodeList fingerprints = doc.elementsByTagNameNS( "DAV:", "getetag" );
  if ( fingerprints.count() == 0 ) return QString::null;

  QDomElement e = fingerprints.item(0).toElement();
kdDebug() << "Fingerprint (etag): " << e.text() << endl;
  return e.text();
}

KIO::Job *ExchangeGlobals::createRemoveItemsJob( const KURL &uploadurl,
       KPIM::GroupwareUploadItem::List deletedItems )
{
  QStringList urls;
  KPIM::GroupwareUploadItem::List::iterator it;
  kdDebug(5800) << " ExchangeGlobals::createRemoveItemsJob: , URL="<<uploadurl.url()<<endl;
  for ( it = deletedItems.begin(); it != deletedItems.end(); ++it ) {
    //kdDebug(7000) << "Delete: " << endl << format.toICalString(*it) << endl;
    kdDebug(7000) << "Delete: " <<   (*it)->url().url() << endl;
    KURL url( uploadurl );
    url.setPath( (*it)->url().url() );
    if ( !(*it)->url().isEmpty() )
      urls << url.url();
    kdDebug(5700) << "Delete (Mod) : " <<   url.url() << endl;
  }
//  return KIO::del( urls, false, false );
  // TODO
  return 0;
}

KPIM::GroupwareJob::ContentType ExchangeGlobals::getContentType( const QDomElement &prop )
{
  const QString &contentclass = prop.namedItem("contentclass").toElement().text();
kdDebug()<<"contentclass: "<<contentclass<<endl;
  return getContentType( contentclass );
}

KPIM::GroupwareJob::ContentType ExchangeGlobals::getContentType( const QString &contentclass )
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

// FIXME: This is exactly the same code, except that it calls getContentType of the ExchangeGlobals class, instead of the one from DAVGroupwareGlobals!!!!
bool ExchangeGlobals::itemsForDownloadFromList( KPIM::GroupwareDataAdaptor *adaptor,
    KIO::Job *job, QStringList &currentlyOnServer, QMap<QString,KPIM::GroupwareJob::ContentType> &itemsForDownload )
{
kdDebug()<<"ExchangeGlobals::itemsForDownloadFromList"<<endl;
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

    adaptor->processDownloadListItem( currentlyOnServer, itemsForDownload,
        entry, newFingerprint, type );
  }

  kdDebug(5800)<<"currentlyOnServer="<<currentlyOnServer.join(", ")<<endl;
  kdDebug(5800)<<"itemsForDownload="<<QStringList( itemsForDownload.keys() ).join(", ")<<endl;
  return true;
}

KIO::TransferJob *ExchangeGlobals::createListItemsJob( const KURL &url )
{
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement(  doc, doc, "d:propfind" );
  QDomElement prop = WebdavHandler::addElement(  doc, root, "d:prop" );
  WebdavHandler::addElement( doc, prop, "d:getetag" );
  WebdavHandler::addElement( doc, prop, "d:contentclass" );
  kdDebug(5800) << "props = "<< doc.toString() << endl;
  return KIO::davPropFind( url, doc, "1", false );
}


