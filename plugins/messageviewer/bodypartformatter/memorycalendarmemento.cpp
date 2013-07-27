/* Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
   Author: Sérgio Martins <sergio.martins@kdab.com>

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
#include "memorycalendarmemento.h"

#include <calendarsupport/next/incidencefetchjob.h>
#include <KSystemTimeZones>

using namespace MessageViewer;

MemoryCalendarMemento::MemoryCalendarMemento()
  : QObject( 0 ), mFinished( false )
{
  CalendarSupport::IncidenceFetchJob *job = new CalendarSupport::IncidenceFetchJob();
  connect( job, SIGNAL(result(KJob*)), this, SLOT(slotSearchJobFinished(KJob*)) );
}

void MemoryCalendarMemento::slotSearchJobFinished( KJob *job )
{
  kDebug();
  mFinished = true;
  CalendarSupport::IncidenceFetchJob *searchJob = static_cast<CalendarSupport::IncidenceFetchJob*>( job );
  if ( searchJob->error() ) {
    kWarning() << "Unable to fetch incidences:" << searchJob->errorText();
  } else {
    mCalendar = KCalCore::MemoryCalendar::Ptr( new KCalCore::MemoryCalendar( KSystemTimeZones::local() ) );
    foreach( const Akonadi::Item &item, searchJob->items() ) {
      if ( item.hasPayload<KCalCore::Incidence::Ptr>() ) {
        const KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();
        mCalendar->addIncidence( incidence );
      }
    }
  }
  emit update( Viewer::Delayed );
}

bool MemoryCalendarMemento::finished() const
{
  return mFinished;
}

KCalCore::MemoryCalendar::Ptr MemoryCalendarMemento::calendar() const
{
  Q_ASSERT( mFinished );
  return mCalendar;
}

void MemoryCalendarMemento::detach()
{
  disconnect( this, SIGNAL(update(MessageViewer::Viewer::UpdateMode)), 0, 0 );
}

#include "memorycalendarmemento.moc"
