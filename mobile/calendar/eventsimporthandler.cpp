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

#include "eventsimporthandler.h"

#include <KCalCore/Event>
#include <KCalCore/FileStorage>
#include <KCalCore/ICalFormat>
#include <KCalCore/MemoryCalendar>
#include <klocale.h>
#include <kmessagebox.h>

QString EventsImportHandler::fileDialogNameFilter() const
{
  return QLatin1String( "*.ics|iCals" );
}

QString EventsImportHandler::fileDialogTitle() const
{
  return i18n( "Select iCal to Import" );
}

QString EventsImportHandler::collectionDialogText() const
{
  return i18n( "Select the calendar the imported event(s) shall be saved in:" );
}

QString EventsImportHandler::collectionDialogTitle() const
{
  return i18n( "Select Calendar" );
}

QString EventsImportHandler::importDialogText( int count, const QString &collectionName ) const
{
  return i18np( "Importing one event to %2", "Importing %1 events to %2", count, collectionName );
}

QString EventsImportHandler::importDialogTitle() const
{
  return i18n( "Import Events" );
}

QStringList EventsImportHandler::mimeTypes() const
{
  return QStringList( KCalCore::Event::eventMimeType() );
}

Akonadi::Item::List EventsImportHandler::createItems( const QStringList &fileNames, bool *ok )
{
  *ok = true;

  Akonadi::Item::List items;

  KCalCore::Event::List events;

  foreach ( const QString &fileName, fileNames ) {
    KCalCore::MemoryCalendar::Ptr calendar( new KCalCore::MemoryCalendar( QLatin1String( "UTC" ) ) );
    KCalCore::FileStorage::Ptr storage( new KCalCore::FileStorage( calendar, fileName, new KCalCore::ICalFormat() ) );

    if ( storage->load() ) {
      events << calendar->events();
    } else {
      const QString caption( i18n( "iCal Import Failed" ) );
      const QString msg = i18nc( "@info",
                                 "<para>Error when trying to read the iCal <filename>%1</filename>:</para>",
                                 fileName );
      KMessageBox::error( 0, msg, caption );
      *ok = false;
    }
  }

  if ( events.isEmpty() ) {
    if ( !(*ok) && fileNames.count() > 1 )
      KMessageBox::information( 0, i18n( "No events were imported, due to errors with the iCals." ) );
    else if ( *ok )
      KMessageBox::information( 0, i18n( "The iCal does not contain any events." ) );

    return items; // nothing to import
  }

  foreach ( const KCalCore::Event::Ptr &event, events ) {
    Akonadi::Item item;
    item.setPayload<KCalCore::Event::Ptr>( event );
    item.setMimeType( KCalCore::Event::eventMimeType() );

    items << item;
  }

  return items;
}
