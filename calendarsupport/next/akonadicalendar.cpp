/*
   Copyright (C) 2011 Sérgio Martins <sergio.martins@kdab.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "akonadicalendar.h"
#include "akonadicalendar_p.h"
#include "incidencefetchjob.h"

using namespace CalendarSupport;
using namespace KCalCore;

AkonadiCalendar::Private::Private( AkonadiCalendar *qq ) : q( qq )
{
  IncidenceFetchJob *job = new IncidenceFetchJob();
  connect( job, SIGNAL(result( KJob*)),
           SLOT(slotSearchJobFinished(KJob*)) );
}

AkonadiCalendar::Private::~Private()
{

}

void AkonadiCalendar::Private::slotSearchJobFinished( KJob *job )
{
  IncidenceFetchJob *searchJob = static_cast<CalendarSupport::IncidenceFetchJob*>( job );
  if ( searchJob->error() ) {
    //TODO: Error
    kWarning() << "Unable to fetch incidences:" << searchJob->errorText();
  } else {
    foreach( const Akonadi::Item &item, searchJob->items() ) {
      if ( item.hasPayload<KCalCore::Incidence::Ptr>() ) {
        const Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();
        if ( incidence )
          q->addIncidence( incidence );
      }
    }
  }
  // emit loaded() in a delayed manner, due to freezes because of execs.
  QMetaObject::invokeMethod( q, "loaded", Qt::QueuedConnection );
}

AkonadiCalendar::AkonadiCalendar( const KDateTime::Spec &timeSpec ) : MemoryCalendar( timeSpec )
                                                                      , d( new Private( this ) )
{
}

AkonadiCalendar::~AkonadiCalendar()
{
  delete d;
}

Akonadi::Item AkonadiCalendar::item( Akonadi::Item::Id id ) const
{
  Akonadi::Item i;
  if ( d->mItemById.contains( id ) ) {
    i = d->mItemById[id];
  } else {
    kDebug() << "Can't find any item with id " << id;
  }
  return i;
}

Akonadi::Item AkonadiCalendar::item( const QString &uid ) const
{
  Q_ASSERT( !uid.isEmpty() );
  Akonadi::Item i;
  if ( d->mItemIdByUid.contains( uid ) ) {
    const Akonadi::Item::Id id = d->mItemIdByUid[uid];
    Q_ASSERT( d->mItemById.contains( id ) );
    i = d->mItemById[id];
  } else {
    kDebug() << "Can't find any incidence with uid " << uid;
  }
  return i;
}

#include "akonadicalendar.moc"
#include "akonadicalendar_p.moc"
