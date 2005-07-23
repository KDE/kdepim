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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "calendaradaptor.h"

#include <libkcal/incidence.h>
#include <libkcal/calendar.h>
#include <libkcal/resourcecached.h>
#include <libkcal/icalformat.h>

#include <kio/job.h>

#include <kdebug.h>

using namespace KCal;


CalendarUploadItem::CalendarUploadItem( CalendarAdaptor *adaptor, KCal::Incidence *incidence, KPIM::GroupwareUploadItem::UploadType type )
    : GroupwareUploadItem( type )
{
  if ( incidence && adaptor ) {
    if ( incidence->type() == "Event" ) mItemType = KPIM::FolderLister::Event;
    else if ( incidence->type() == "Todo" ) mItemType = KPIM::FolderLister::Todo;
    else if ( incidence->type() == "Journal" ) mItemType = KPIM::FolderLister::Journal;

    setUrl( incidence->customProperty( adaptor->identifier(), "storagelocation" ) );
    setUid( incidence->uid() );

    ICalFormat format;
    format.setTimeZone( adaptor->resource()->timeZoneId(), true );
    setData( format.toICalString( incidence ) );
  }
}



CalendarAdaptor::CalendarAdaptor()
{
}

QString CalendarAdaptor::mimeType() const
{
  return "text/calendar";
}

bool CalendarAdaptor::localItemExists( const QString &localId )
{
  KCal::Incidence *i = mResource->incidence( localId );
  return i;
}

bool CalendarAdaptor::localItemHasChanged( const QString &localId )
{
  KCal::Incidence *i = mResource->incidence( localId );
  if ( !i ) return false;

  if ( !mResource->deletedIncidences().isEmpty() &&
      mResource->deletedIncidences().find( i )
   != mResource->deletedIncidences().end() )
    return true;
  if ( !mResource->changedIncidences().isEmpty() &&
       mResource->changedIncidences().find( i )
    != mResource->changedIncidences().end() )
    return true;

  return false;
}

void CalendarAdaptor::deleteItem( const QString &localId )
{
  mResource->disableChangeNotification();
  KCal::Incidence *i = mResource->incidence( localId );
  if ( i ) {
    mResource->deleteIncidence( i );
    mResource->clearChange( i );
  }
  mResource->enableChangeNotification();
}

void CalendarAdaptor::addItem( KCal::Incidence *i)
{
  if ( i ) {
    mResource->disableChangeNotification();
    Incidence *existing = mResource->incidence( i->uid() );
    if ( existing ) {
      mResource->deleteIncidence( i );
    }
    mResource->addIncidence( i );
    mResource->clearChange( i );
    mResource->enableChangeNotification();
  }
}


void CalendarAdaptor::calendarItemDownloaded( KCal::Incidence *inc,
    const QString &newLocalId, const KURL &remoteId, const QString &fingerprint,
    const QString &storagelocation )
{
kdDebug() << "CalendarAdaptor::calendarItemDownloaded, inc=" << inc->summary() << ", local=" << newLocalId << ", remote=" << remoteId.url() << ", fpr=" << fingerprint << ", storagelocation="<< storagelocation << endl;
  // remove the currently existing item from the cache
  deleteItem( newLocalId );
  QString localId = idMapper()->localId( remoteId.path() );
  if ( !localId.isEmpty() ) deleteItem( localId );
  
  // add the new item
  inc->setCustomProperty( identifier(), "storagelocation", storagelocation );
  if ( !localId.isEmpty() ) inc->setUid( localId );
  addItem( inc );
  
  // update the fingerprint and the ids in the idMapper
  idMapper()->removeRemoteId( localId );
  idMapper()->removeRemoteId( newLocalId );

  emit itemDownloaded( inc->uid(), remoteId, fingerprint );
}


void CalendarAdaptor::clearChange( const QString &uid )
{
  KCal::Incidence *i = mResource->incidence( uid );
  mResource->clearChange( i );
}

KPIM::GroupwareUploadItem *CalendarAdaptor::newUploadItem( KCal::Incidence*it,
             KPIM::GroupwareUploadItem::UploadType type )
{
  return new CalendarUploadItem( this, it, type );
}

#include "calendaradaptor.moc"
