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
  url = WebdavHandler::toDAV( url );
  url.addPath( "new.vcf" );
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

KIO::TransferJob *OGoAddressBookAdaptor::createDownloadItemJob( const KURL &url )
{
  return OGoGlobals::createDownloadItemJob( this, url );
}

KIO::TransferJob *OGoAddressBookAdaptor::createListItemsJob( const KURL &url )
{
  return DAVGroupwareGlobals::createListItemsJob( url );
}

bool OGoAddressBookAdaptor::itemsForDownloadFromList( KIO::Job *job, QStringList &currentlyOnServer, QStringList &itemsForDownload )
{
  return DAVGroupwareGlobals::itemsForDownloadFromList( this, job, currentlyOnServer, itemsForDownload );
}

void OGoAddressBookAdaptor::updateFingerprintId( KIO::TransferJob *trfjob, KPIM::GroupwareUploadItem *item )
{
  return OGoGlobals::updateFingerprintId( this, trfjob, item );
}

KIO::Job *OGoAddressBookAdaptor::createRemoveItemsJob( const KURL &uploadurl, KPIM::GroupwareUploadItem::List deletedItems )
{
  return OGoGlobals::createRemoveItemsJob( uploadurl, deletedItems );
}
