/*
    This file is part of kdepim.

    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "exchangeaddressbookadaptor.h"
#include "exchangeconvertercontact.h"
#include <davgroupwareglobals.h>
#include "exchangeglobals.h"
#include <webdavhandler.h>
#include <kdebug.h>
#include <kio/davjob.h>

using namespace KABC;

ExchangeAddressBookUploadItem::ExchangeAddressBookUploadItem( AddressBookAdaptor *adaptor, KABC::Addressee addr, KPIM::GroupwareUploadItem::UploadType type )
    : GroupwareUploadItem( type )
{
  if ( adaptor && !addr.isEmpty() ) {
    setUrl( addr.custom( adaptor->identifier(), "storagelocation" ) );
    setUid( addr.uid() );

    ExchangeConverterContact format;
    mDavData = format.createWebDAV( addr );
  }
}

KIO::TransferJob *ExchangeAddressBookUploadItem::createUploadJob( KPIM::GroupwareDataAdaptor *adaptor, const KURL &url )
{
  Q_ASSERT( adaptor );
  if ( !adaptor ) return 0;
  KIO::DavJob *job = KIO::davPropPatch( url, mDavData, false );
  return job;
}

KIO::TransferJob *ExchangeAddressBookUploadItem::createUploadNewJob( KPIM::GroupwareDataAdaptor *adaptor, const KURL &baseurl )
{
  KURL url( baseurl );
  url.addPath( uid() + ".EML" );
  kdDebug() << "Upload Job's URL: " << url.url() << endl;
  return createUploadJob( adaptor, url );
}


ExchangeAddressBookAdaptor::ExchangeAddressBookAdaptor() : AddressBookAdaptor()
{
}

void ExchangeAddressBookAdaptor::adaptDownloadUrl( KURL &url )
{
  url = WebdavHandler::toDAV( url );
}

void ExchangeAddressBookAdaptor::adaptUploadUrl( KURL &url )
{
  url = WebdavHandler::toDAV( url );
//   url.setPath( url.path() + "/NewItem.EML" );
}

QString ExchangeAddressBookAdaptor::mimeType() const
{
  return "message/rfc822";
}

KIO::TransferJob *ExchangeAddressBookAdaptor::createListItemsJob( const KURL &url )
{
  return ExchangeGlobals::createListItemsJob( url );
}

QString ExchangeAddressBookAdaptor::extractFingerprint( KIO::TransferJob *job,
                                                   const QString &rawText )
{
  return ExchangeGlobals::extractFingerprint( job, rawText );
}

KABC::Addressee::List ExchangeAddressBookAdaptor::parseData( KIO::TransferJob *job, const QString &/*rawText*/ )
{
  KIO::DavJob *davjob = dynamic_cast<KIO::DavJob*>(job);
  if (!davjob) return KABC::Addressee::List();

kdDebug() << "ExchangeAddressBookAdaptor::parseData(): QDomDocument=" << endl << davjob->response().toString() << endl;
  KABC::ExchangeConverterContact conv;
  KABC::Addressee::List addressees = conv.parseWebDAV( davjob->response() );
  return addressees;
}

KIO::TransferJob *ExchangeAddressBookAdaptor::createDownloadItemJob( const KURL &url, KPIM::GroupwareJob::ContentType ctype )
{
kdDebug()<<"ExchangeAddressBookAdaptor::createDownloadItemJob()"<<endl;
  // Don't use an <allprop/> request!
  KIO::DavJob *job = 0;
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement(  doc, doc, "d:propfind" );
  QDomElement prop = WebdavHandler::addElement( doc, root, "d:prop" );
  QDomAttr att_h = doc.createAttribute( "xmlns:h" );
  att_h.setValue( "urn:schemas:mailheader:" );
  root.setAttributeNode( att_h );

  QDomAttr att_m = doc.createAttribute( "xmlns:m" );
  att_m.setValue( "urn:schemas:httpmail:" );
  root.setAttributeNode( att_m );

  if ( ctype == KPIM::GroupwareJob::Contact ) {
    KABC::ExchangeConverterContact::createRequest( doc, root );
    kdDebug(7000) << "doc: " << doc.toString() << endl;
    job = KIO::davPropFind( url, doc, "0", false );
  }
  return job;
}

bool ExchangeAddressBookAdaptor::itemsForDownloadFromList( KIO::Job *job, QStringList &currentlyOnServer, QMap<QString,KPIM::GroupwareJob::ContentType> &itemsForDownload )
{
  return ExchangeGlobals::itemsForDownloadFromList( this, job, currentlyOnServer, itemsForDownload );
}

KIO::Job *ExchangeAddressBookAdaptor::createRemoveItemsJob( const KURL &uploadurl, KPIM::GroupwareUploadItem::List deletedItems )
{
  return ExchangeGlobals::createRemoveItemsJob( uploadurl, deletedItems );
}

KPIM::GroupwareUploadItem *ExchangeAddressBookAdaptor::newUploadItem( KABC::Addressee addr,
           KPIM::GroupwareUploadItem::UploadType type )
{
  return new ExchangeAddressBookUploadItem( this, addr, type );
}
