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

#include <Akonadi/Calendar/FetchJobCalendar>
#include <KSystemTimeZones>

using namespace MessageViewer;
using namespace Akonadi;

MemoryCalendarMemento::MemoryCalendarMemento()
  : QObject( 0 ), mFinished( false )
{
  FetchJobCalendar::Ptr calendar = FetchJobCalendar::Ptr( new FetchJobCalendar( this ) );
  mCalendar = calendar;
  connect(calendar.data(), SIGNAL(loadFinished(bool,QString)),
          SLOT(slotCalendarLoaded(bool,QString)) );
}

void MemoryCalendarMemento::slotCalendarLoaded( bool success, const QString &errorMessage )
{
  kDebug();
  mFinished = true;

  if ( !success ) {
    kWarning() << "Unable to fetch incidences:" << errorMessage;
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

