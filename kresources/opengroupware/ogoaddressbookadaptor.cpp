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

#include "ogoaddressbookadaptor.h"
#include "ogoglobals.h"
#include "davgroupwareglobals.h"
#include "webdavhandler.h"

#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#include <kabcresourcecached.h>

#include <kio/job.h>
#include <kdebug.h>

using namespace KABC;

OGoAddressBookAdaptor::OGoAddressBookAdaptor()
{
}

void OGoAddressBookAdaptor::adaptDownloadUrl( KURL &url )
{
  url = WebdavHandler::toDAV( url );
}

QCString OGoAddressBookAdaptor::identifier() const
{
  return "KABCResourceOpengroupware";
}

void OGoAddressBookAdaptor::adaptUploadUrl( KURL &url )
{
kdDebug()<<"OGoAddressBookAdaptor::adaptUploadUrl( "<<url.url()<<")"<<endl;
  url = WebdavHandler::toDAV( url );
  url = WebdavHandler::toDAV( url );
  url.setPath( url.path() + "/new.vcf" );
// url.addPath( "new.vcf" );
kdDebug()<<"after OGoAddressBookAdaptor::adaptUploadUrl( "<<url.url()<<")"<<endl;
}

QString OGoAddressBookAdaptor::mimeType() const
{
  return "text/x-vcard";
}

KABC::Addressee::List OGoAddressBookAdaptor::parseData( KIO::TransferJob */*job*/, const QString &rawText )
{
  KABC::VCardConverter conv;
  return conv.parseVCards( rawText );
}

QString OGoAddressBookAdaptor::extractFingerprint( KIO::TransferJob *job,
                                                   const QString &rawText )
{
  return OGoGlobals::extractFingerprint( job, rawText );
}

KIO::TransferJob *OGoAddressBookAdaptor::createDownloadItemJob( const KURL &url, KPIM::GroupwareJob::ContentType ctype )
{
  return OGoGlobals::createDownloadItemJob( this, url, ctype );
}

KIO::TransferJob *OGoAddressBookAdaptor::createListItemsJob( const KURL &url )
{
  return DAVGroupwareGlobals::createListItemsJob( url );
}

bool OGoAddressBookAdaptor::itemsForDownloadFromList( KIO::Job *job, QStringList &currentlyOnServer, QMap<QString,KPIM::GroupwareJob::ContentType> &itemsForDownload )
{
  return DAVGroupwareGlobals::itemsForDownloadFromList( this, job, currentlyOnServer, itemsForDownload );
}

void OGoAddressBookAdaptor::uploadFinished( KIO::TransferJob *trfjob, KPIM::GroupwareUploadItem *item )
{
//   OGoGlobals::uploadFinished( this, trfjob, item );
  Addressee addr( resource()->findByUid( item->uid() ) );
  if ( !addr.isEmpty() ) {
    resource()->removeAddressee( addr );
/*    addr.insertCustom( identifier(), "storagelocation",
               idMapper()->remoteId( item->uid() ) );*/
  }
}

KIO::Job *OGoAddressBookAdaptor::createRemoveItemsJob( const KURL &uploadurl, KPIM::GroupwareUploadItem::List deletedItems )
{
  return OGoGlobals::createRemoveItemsJob( uploadurl, deletedItems );
}
