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

KCal::Incidence::List CalendarAdaptor::parseData( KIO::TransferJob */*job*/, const QString &rawText )
{
kdDebug(5800)<<"CalendarAdaptor::parseData, iCalendar="<<endl;
kdDebug(5800)<<rawText<<endl;
  KCal::CalendarLocal calendar;
  KCal::ICalFormat ical;
  calendar.setTimeZoneId( mResource->timeZoneId() );
  KCal::Incidence::List incidences;
  if ( ical.fromString( &calendar, rawText ) ) {
    KCal::Incidence::List raw = calendar.rawIncidences();
    KCal::Incidence::List::Iterator it = raw.begin();
    for ( ; it != raw.end(); ++it ) {
      incidences.append( (*it)->clone() );
    }
  } else {
    kdError() << "Unable to parse iCalendar" << endl;
  }
  return incidences;
}

QString CalendarAdaptor::addItem( KIO::TransferJob *job,
     const QString &rawText, QString &fingerprint,
     const QString &localId, const QString &storageLocation )
{
  fingerprint = extractFingerprint( job, rawText );

  KCal::Incidence::List incidences = parseData( job, rawText );
  if ( incidences.count() < 1 ) {
    kdError() << "Parsed iCalendar contains no event." << endl;
    return QString::null;
  }
  if ( incidences.count() > 1 ) {
    kdError() << "More than one event in iCalendar" << endl;
    KCal::Incidence::List::Iterator it = incidences.begin();
    for ( ; it != incidences.end(); ++it ) {
      delete (*it);
    }
    return QString::null;
  }

  mResource->disableChangeNotification();
  // TODO: Remove existing incidence. Make sure it was not changed meanwhile!
  Incidence *inc = mResource->incidence( localId );
  if ( inc )
    mResource->deleteIncidence( inc );

  KCal::Incidence *i = (incidences.front())->clone();
  if ( !localId.isEmpty() ) i->setUid( localId );
  i->setCustomProperty( identifier(), "storagelocation", storageLocation );
  mResource->addIncidence( i );
  mResource->enableChangeNotification();

  return i->uid();
}

QString CalendarAdaptor::extractUid( KIO::TransferJob *job, const QString &data )
{
  KCal::Incidence::List incidences = parseData( job, data );
  if ( incidences.count() > 0 ) {
    return incidences.first()->uid();
  }
  else return QString::null;
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

void CalendarAdaptor::uploadFinished( KIO::TransferJob *trfjob, KPIM::GroupwareUploadItem *item )
{
//   OGoGlobals::uploadFinished( this, trfjob, item );
  Incidence *inc = resource()->incidence( item->uid() );
  if ( inc ) {
//     resource()->disableChangeNotification();
    resource()->deleteIncidence( inc );
/*    inc->setCustomProperty( identifier(), "storagelocation",
               idMapper()->remoteId( item->uid() ) );*/
//    resource()->addIncidence( inc );
//     resource()->enableChangeNotification();
  }
}

