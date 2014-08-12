/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "eventsexporthandler.h"

#include <KCalCore/Event>
#include <KCalCore/FileStorage>
#include <KCalCore/ICalFormat>
#include <KCalCore/MemoryCalendar>
#include <kfiledialog.h>
#include <klocale.h>

QString EventsExportHandler::dialogText() const
{
  return i18n( "Which events shall be exported?" );
}

QString EventsExportHandler::dialogAllText() const
{
  return i18n( "All Events" );
}

QString EventsExportHandler::dialogLocalOnlyText() const
{
  return i18n( "Events in current folder" );
}

QStringList EventsExportHandler::mimeTypes() const
{
  return QStringList( KCalCore::Event::eventMimeType() );
}

bool EventsExportHandler::exportItems( const Akonadi::Item::List &items )
{
  const QString fileName = KFileDialog::getSaveFileName( QUrl( QLatin1String("calendar.ics") ), QLatin1String( "*.ics" ) );
  if ( fileName.isEmpty() )
    return true;

  KCalCore::MemoryCalendar::Ptr calendar( new KCalCore::MemoryCalendar( QLatin1String( "UTC" ) ) );
  calendar->startBatchAdding();
  foreach ( const Akonadi::Item &item, items ) {
    if ( item.hasPayload<KCalCore::Event::Ptr>() )
      calendar->addIncidence( item.payload<KCalCore::Event::Ptr>() );
  }
  calendar->endBatchAdding();

  KCalCore::FileStorage::Ptr storage( new KCalCore::FileStorage( calendar, fileName, new KCalCore::ICalFormat() ) );

  if ( storage->open() ) {
    storage->save();
    storage->close();
  } else
    return false;

  return true;
}
