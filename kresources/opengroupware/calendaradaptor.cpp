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

using namespace KPIM;

CalendarAdaptor::CalendarAdaptor()
{
}

void CalendarAdaptor::adaptDownloadUrl( KURL &url )
{
  url.addPath( "/Calendar" );
}

void CalendarAdaptor::adaptUploadUrl( KURL &url )
{
  url.setPath( url.path() + "/Calendar/new.ics" );
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

  if ( mResource->deletedIncidences().find( i ) !=
    mResource->deletedIncidences().end()
    || mResource->changedIncidences().find( i ) !=
    mResource->changedIncidences().end() ) {
    return true;
  } else {
    return false;
  }
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

QString CalendarAdaptor::addItem( const QString &rawText,
  const QString &localId, const QString &storageLocation )
{
  KCal::CalendarLocal calendar;
  KCal::ICalFormat ical;
  if ( !ical.fromString( &calendar, rawText ) ) {
    kdError() << "Unable to parse iCalendar" << endl;
    return QString::null;
  } else {
    KCal::Incidence::List incidences = calendar.incidences();
    if ( incidences.count() > 1 ) {
      kdError() << "More than one event in iCalendar" << endl;
      return QString::null;
    }
    
    mResource->disableChangeNotification();
    KCal::Incidence *i = (*(incidences.begin()))->clone();
    if ( !localId.isEmpty() ) i->setUid( localId );
    i->setCustomProperty( "KCalResourceOpengroupware", "storagelocation" ,
      storageLocation );
    mResource->addIncidence( i );
    mResource->enableChangeNotification();
  
    return i->uid();
  }
}

QString CalendarAdaptor::extractUid( const QString &data )
{
  KCal::ICalFormat ical;
  KCal::Incidence *i = ical.fromString( data );
  if ( i ) return i->uid();
  else return QString::null;
}

void CalendarAdaptor::clearChange( const QString &uid )
{
  KCal::Incidence *i = mResource->incidence( uid );
  mResource->clearChange( i );
}
