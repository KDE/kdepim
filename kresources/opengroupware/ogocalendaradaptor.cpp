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

#include "ogocalendaradaptor.h"
#include "ogoglobals.h"
#include "davgroupwareglobals.h"
#include "webdavhandler.h"
#include <libemailfunctions/idmapper.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>
#include <libkcal/resourcecached.h>

#include <kdebug.h>

using namespace KCal;

OGoCalendarAdaptor::OGoCalendarAdaptor()
{
}

void OGoCalendarAdaptor::adaptDownloadUrl( KURL &url )
{
  url = WebdavHandler::toDAV( url );
}

void OGoCalendarAdaptor::adaptUploadUrl( KURL &url )
{
  url = WebdavHandler::toDAV( url );
  url.setPath( url.path() + "/new.ics" );
}

QString OGoCalendarAdaptor::extractFingerprint( KIO::TransferJob *job,
                                             const QString &rawText )
{
  return OGoGlobals::extractFingerprint( job, rawText );
}

KIO::TransferJob *OGoCalendarAdaptor::createDownloadItemJob( const KURL &url, KPIM::GroupwareJob::ContentType ctype )
{
  return OGoGlobals::createDownloadItemJob( this, url,ctype );
}

KIO::TransferJob *OGoCalendarAdaptor::createListItemsJob( const KURL &url )
{
  return DAVGroupwareGlobals::createListItemsJob( url );
}

bool OGoCalendarAdaptor::itemsForDownloadFromList( KIO::Job *job, QStringList &currentlyOnServer, QMap<QString,KPIM::GroupwareJob::ContentType> &itemsForDownload )
{
  return DAVGroupwareGlobals::itemsForDownloadFromList( this, job, currentlyOnServer, itemsForDownload );
}

void OGoCalendarAdaptor::updateFingerprintId( KIO::TransferJob *trfjob, KPIM::GroupwareUploadItem *item )
{
  OGoGlobals::updateFingerprintId( this, trfjob, item );
//  idMapper()->setFingerprint( item->uid(), "" );
  Incidence *inc = resource()->incidence( item->uid() );
  if ( inc ) {
    resource()->disableChangeNotification();
    inc->setCustomProperty( identifier(), "storagelocation",
               idMapper()->remoteId( item->uid() ) );
//    resource()->addIncidence( inc );
    resource()->enableChangeNotification();
  }
}

KIO::Job *OGoCalendarAdaptor::createRemoveItemsJob( const KURL &uploadurl, KPIM::GroupwareUploadItem::List deletedItems )
{
  return OGoGlobals::createRemoveItemsJob( uploadurl, deletedItems );
}
