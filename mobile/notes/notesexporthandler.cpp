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

#include "notesexporthandler.h"

#include <KLocalizedString>
#include <kmbox/mbox.h>
#include <kmessagebox.h>
#include <kmime/kmime_message.h>
#include <QFileDialog>

QString NotesExportHandler::dialogText() const
{
  return i18n( "Which notes shall be exported?" );
}

QString NotesExportHandler::dialogAllText() const
{
  return i18n( "All Notes" );
}

QString NotesExportHandler::dialogLocalOnlyText() const
{
  return i18n( "Notes in current folder" );
}

QStringList NotesExportHandler::mimeTypes() const
{
  return QStringList( QLatin1String( "text/x-vnd.akonadi.note" ) );
}

bool NotesExportHandler::exportItems( const Akonadi::Item::List &items )
{
  QList<KMime::Message::Ptr> notes;

  foreach ( const Akonadi::Item &item, items ) {
    if ( item.hasPayload<KMime::Message::Ptr>() )
      notes << item.payload<KMime::Message::Ptr>();
  }

  const QString fileName = QFileDialog::getSaveFileName(0, QString(), QLatin1String("notes.mbox"), QLatin1String( "*.mbox" ) );
  if ( fileName.isEmpty() ) // user canceled export
    return true;

  KMBox::MBox mbox;
  if ( !mbox.load( fileName ) ) {
    KMessageBox::error( 0, i18n( "Unable to open MBox file %1", fileName ) );
    return false;
  }

  foreach ( const KMime::Message::Ptr &note, notes ) {
    mbox.appendMessage( note );
  }

  if ( !mbox.save() ) {
    KMessageBox::error( 0, i18n( "Unable to save notes to MBox file %1", fileName ) );
    return false;
  }

  return true;
}
