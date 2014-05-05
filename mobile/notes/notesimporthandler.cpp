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

#include "notesimporthandler.h"

#include <kmbox/mbox.h>
#include <kmime/kmime_message.h>
#include <klocale.h>
#include <kmessagebox.h>

QString NotesImportHandler::fileDialogNameFilter() const
{
  return QLatin1String( "*.mbox|MBox" );
}

QString NotesImportHandler::fileDialogTitle() const
{
  return i18n( "Select MBox to Import" );
}

QString NotesImportHandler::collectionDialogText() const
{
  return i18n( "Select the folder the imported note(s) shall be saved in:" );
}

QString NotesImportHandler::collectionDialogTitle() const
{
  return i18n( "Select Folder" );
}

QString NotesImportHandler::importDialogText( int count, const QString &collectionName ) const
{
  return i18np( "Importing one note to %2", "Importing %1 notes to %2", count, collectionName );
}

QString NotesImportHandler::importDialogTitle() const
{
  return i18n( "Import Notes" );
}

QStringList NotesImportHandler::mimeTypes() const
{
  return QStringList( QLatin1String( "text/x-vnd.akonadi.note" ) );
}

Akonadi::Item::List NotesImportHandler::createItems( const QStringList &fileNames, bool *ok )
{
  *ok = true;

  Akonadi::Item::List items;

  QList<KMime::Message::Ptr> notes;

  foreach ( const QString &fileName, fileNames ) {
    KMBox::MBox mbox;

    if ( mbox.load( fileName ) ) {

      const KMBox::MBoxEntry::List entries = mbox.entries();
      mbox.lock();
      foreach ( const KMBox::MBoxEntry &entry, entries ) {
        KMime::Message *note = mbox.readMessage( entry );
        if ( note )
          notes << KMime::Message::Ptr( note );
      }
      mbox.unlock();
    } else {
      const QString caption( i18n( "MBox Import Failed" ) );
      const QString msg = i18nc( "@info",
                                 "<para>When trying to read the MBox, there was an error opening the file <filename>%1</filename>:</para>",
                                 fileName );
      KMessageBox::error( 0, msg, caption );
      *ok = false;
    }
  }

  if ( notes.isEmpty() ) {
    if ( !(*ok) && fileNames.count() > 1 )
      KMessageBox::information( 0, i18n( "No notes were imported, due to errors with the MBox." ) );
    else if ( *ok )
      KMessageBox::information( 0, i18n( "The MBox does not contain any notes." ) );

    return items; // nothing to import
  }

  foreach ( const KMime::Message::Ptr &note, notes ) {
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>( note );
    item.setMimeType( QLatin1String( "text/x-vnd.akonadi.note" ) );

    items << item;
  }

  return items;
}
