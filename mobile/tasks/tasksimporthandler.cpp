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

#include "tasksimporthandler.h"

#include <KCalCore/filestorage.h>
#include <KCalCore/icalformat.h>
#include <KCalCore/memorycalendar.h>
#include <KCalCore/todo.h>
#include <klocale.h>
#include <kmessagebox.h>

QString TasksImportHandler::fileDialogNameFilter() const
{
  return QLatin1String( "*.ics|iCals" );
}

QString TasksImportHandler::fileDialogTitle() const
{
  return i18n( "Select iCal to Import" );
}

QString TasksImportHandler::collectionDialogText() const
{
  return i18n( "Select the calendar the imported task(s) shall be saved in:" );
}

QString TasksImportHandler::collectionDialogTitle() const
{
  return i18n( "Select Calendar" );
}

QString TasksImportHandler::importDialogText( int count, const QString &collectionName ) const
{
  return i18np( "Importing one task to %2", "Importing %1 tasks to %2", count, collectionName );
}

QString TasksImportHandler::importDialogTitle() const
{
  return i18n( "Import Tasks" );
}

QStringList TasksImportHandler::mimeTypes() const
{
  return QStringList( KCalCore::Todo::todoMimeType() );
}

Akonadi::Item::List TasksImportHandler::createItems( const QStringList &fileNames, bool *ok )
{
  *ok = true;

  Akonadi::Item::List items;

  KCalCore::Todo::List tasks;

  foreach ( const QString &fileName, fileNames ) {
    KCalCore::MemoryCalendar::Ptr calendar( new KCalCore::MemoryCalendar( QLatin1String( "UTC" ) ) );
    KCalCore::FileStorage::Ptr storage( new KCalCore::FileStorage( calendar, fileName, new KCalCore::ICalFormat() ) );

    if ( storage->load() ) {
      tasks << calendar->todos();
    } else {
      const QString caption( i18n( "iCal Import Failed" ) );
      const QString msg = i18nc( "@info",
                                 "<para>Error when trying to read the iCal <filename>%1</filename>:</para>",
                                 fileName );
      KMessageBox::error( 0, msg, caption );
      *ok = false;
    }
  }

  if ( tasks.isEmpty() ) {
    if ( !(*ok) && fileNames.count() > 1 )
      KMessageBox::information( 0, i18n( "No tasks were imported, due to errors with the iCals." ) );
    else if ( *ok )
      KMessageBox::information( 0, i18n( "The iCal does not contain any tasks." ) );

    return items; // nothing to import
  }

  foreach ( const KCalCore::Todo::Ptr &task, tasks ) {
    Akonadi::Item item;
    item.setPayload<KCalCore::Todo::Ptr>( task );
    item.setMimeType( KCalCore::Todo::todoMimeType() );

    items << item;
  }

  return items;
}
