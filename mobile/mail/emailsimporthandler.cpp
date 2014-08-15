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

#include "emailsimporthandler.h"

#include <kmbox/mbox.h>
#include <kmime/kmime_message.h>
#include <KLocalizedString>
#include <kmessagebox.h>

QString EmailsImportHandler::fileDialogNameFilter() const
{
  return QLatin1String( "*.mbox|MBox" );
}

QString EmailsImportHandler::fileDialogTitle() const
{
  return i18n( "Select MBox to Import" );
}

QString EmailsImportHandler::collectionDialogText() const
{
  return i18n( "Select the folder the imported email(s) shall be saved in:" );
}

QString EmailsImportHandler::collectionDialogTitle() const
{
  return i18n( "Select Folder" );
}

QString EmailsImportHandler::importDialogText( int count, const QString &collectionName ) const
{
  return i18np( "Importing one email to %2", "Importing %1 emails to %2", count, collectionName );
}

QString EmailsImportHandler::importDialogTitle() const
{
  return i18n( "Import Emails" );
}

QStringList EmailsImportHandler::mimeTypes() const
{
  return QStringList( KMime::Message::mimeType() );
}

Akonadi::Item::List EmailsImportHandler::createItems( const QStringList &fileNames, bool *ok )
{
  *ok = true;

  Akonadi::Item::List items;

  QList<KMime::Message::Ptr> messages;

  foreach ( const QString &fileName, fileNames ) {
    KMBox::MBox mbox;

    if ( mbox.load( fileName ) ) {

      const KMBox::MBoxEntry::List entries = mbox.entries();
      mbox.lock();
      foreach ( const KMBox::MBoxEntry &entry, entries ) {
        KMime::Message *message = mbox.readMessage( entry );
        if ( message )
          messages << KMime::Message::Ptr( message );
      }
      mbox.unlock();
    } else {
      const QString caption( i18n( "MBox Import Failed" ) );
      const QString msg = xi18nc( "@info",
                                 "<para>When trying to read the MBox, there was an error opening the file <filename>%1</filename>:</para>",
                                 fileName );
      KMessageBox::error( 0, msg, caption );
      *ok = false;
    }
  }

  if ( messages.isEmpty() ) {
    if ( !(*ok) && fileNames.count() > 1 )
      KMessageBox::information( 0, i18n( "No emails were imported, due to errors with the MBox." ) );
    else if ( *ok )
      KMessageBox::information( 0, i18n( "The MBox does not contain any emails." ) );

    return items; // nothing to import
  }

  foreach ( const KMime::Message::Ptr &message, messages ) {
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>( message );
    item.setMimeType( KMime::Message::mimeType() );

    items << item;
  }

  return items;
}
