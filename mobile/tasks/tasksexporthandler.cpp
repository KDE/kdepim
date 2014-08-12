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

#include "tasksexporthandler.h"

#include <KCalCore/FileStorage>
#include <KCalCore/ICalFormat>
#include <KCalCore/MemoryCalendar>
#include <KCalCore/Todo>
#include <kfiledialog.h>
#include <klocale.h>

QString TasksExportHandler::dialogText() const
{
  return i18n( "Which tasks shall be exported?" );
}

QString TasksExportHandler::dialogAllText() const
{
  return i18n( "All Tasks" );
}

QString TasksExportHandler::dialogLocalOnlyText() const
{
  return i18n( "Tasks in current folder" );
}

QStringList TasksExportHandler::mimeTypes() const
{
  return QStringList( KCalCore::Todo::todoMimeType() );
}

bool TasksExportHandler::exportItems( const Akonadi::Item::List &items )
{
  const QString fileName = KFileDialog::getSaveFileName( QUrl( QLatin1String("calendar.ics") ), QLatin1String( "*.ics" ) );
  if ( fileName.isEmpty() )
    return true;

  KCalCore::MemoryCalendar::Ptr calendar( new KCalCore::MemoryCalendar( QLatin1String( "UTC" ) ) );
  calendar->startBatchAdding();
  foreach ( const Akonadi::Item &item, items ) {
    if ( item.hasPayload<KCalCore::Todo::Ptr>() )
      calendar->addIncidence( item.payload<KCalCore::Todo::Ptr>() );
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
