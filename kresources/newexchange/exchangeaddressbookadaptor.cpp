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
    mItemType = KPIM::FolderLister::Contact;

    setUrl( addr.custom( adaptor->identifier(), "storagelocation" ) );
    setUid( addr.uid() );

    ExchangeConverterContact format;
    mDavData = format.createWebDAV( addr );
  }
}

KIO::TransferJob *ExchangeAddressBookUploadItem::createUploadJob( KPIM::GroupwareDataAdaptor *adaptor, const KURL &/*baseurl*/ )
{
kdDebug()<<"ExchangeAddressBookUploadItem::createUploadJob"<<endl;
  Q_ASSERT( adaptor );
  if ( !adaptor ) return 0;
  KURL upUrl( url() );
  adaptor->adaptUploadUrl( upUrl );
  kdDebug() << "Uploading to: " << upUrl.prettyURL() << endl;
  KIO::DavJob *job = KIO::davPropPatch( upUrl, mDavData, false );
  return job;
}

KIO::TransferJob *ExchangeAddressBookUploadItem::createUploadNewJob( KPIM::GroupwareDataAdaptor *adaptor, const KURL &baseurl )
{
kdDebug()<<"ExchangeAddressBookUploadItem::createUploadNewJob"<<endl;
  KURL url( baseurl );
  // TODO: Check that this URL doesn't exist yet
  url.addPath( uid() + ".EML" );
  setUrl( url );
//url.addPath("newItem.EML");
kdDebug()<<"Upload path: "<<url.url()<<endl;
  return createUploadJob( adaptor, url );
}

ExchangeAddressBookAdaptor::ExchangeAddressBookAdaptor() : DavAddressBookAdaptor()
{
}

void ExchangeAddressBookAdaptor::customAdaptDownloadUrl( KURL &url )
{
  url = WebdavHandler::toDAV( url );
}

void ExchangeAddressBookAdaptor::customAdaptUploadUrl( KURL &url )
{
  url = WebdavHandler::toDAV( url );
//   url.setPath( url.path() + "/NewItem.EML" );
}

QString ExchangeAddressBookAdaptor::defaultNewItemName( KPIM::GroupwareUploadItem *item ) {
  if ( item ) return item->uid()+".EML";
  else return QString::null;
}


KPIM::GroupwareUploadItem *ExchangeAddressBookAdaptor::newUploadItem( KABC::Addressee addr,
           KPIM::GroupwareUploadItem::UploadType type )
{
  return new ExchangeAddressBookUploadItem( this, addr, type );
}
