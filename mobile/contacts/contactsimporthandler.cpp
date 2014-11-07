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

#include "contactsimporthandler.h"

#include <kcontacts/addressee.h>
#include <kcontacts/vcardconverter.h>
#include <KLocalizedString>
#include <kmessagebox.h>

#include <QtCore/QFile>

QString ContactsImportHandler::fileDialogNameFilter() const
{
  return QLatin1String( "*.vcf|vCards" );
}

QString ContactsImportHandler::fileDialogTitle() const
{
  return i18n( "Select vCard to Import" );
}

QString ContactsImportHandler::collectionDialogText() const
{
  return i18n( "Select the address book the imported contact(s) shall be saved in:" );
}

QString ContactsImportHandler::collectionDialogTitle() const
{
  return i18n( "Select Address Book" );
}

QString ContactsImportHandler::importDialogText( int count, const QString &collectionName ) const
{
  return i18np( "Importing one contact to %2", "Importing %1 contacts to %2", count, collectionName );
}

QString ContactsImportHandler::importDialogTitle() const
{
  return i18n( "Import Contacts" );
}

QStringList ContactsImportHandler::mimeTypes() const
{
  return QStringList( KContacts::Addressee::mimeType() );
}

Akonadi::Item::List ContactsImportHandler::createItems( const QStringList &fileNames, bool *ok )
{
  *ok = true;

  Akonadi::Item::List items;

  KContacts::VCardConverter converter;
  KContacts::Addressee::List contacts;

  foreach ( const QString &fileName, fileNames ) {
    QFile file( fileName );

    if ( file.open( QIODevice::ReadOnly ) ) {
      const QByteArray data = file.readAll();
      file.close();
      if ( data.size() > 0 ) {
        contacts += converter.parseVCards( data );
      }
    } else {
      const QString caption( i18n( "vCard Import Failed" ) );
      const QString msg = xi18nc( "@info",
                                 "<para>When trying to read the vCard, there was an error opening the file <filename>%1</filename>:</para>"
                                 "<para>%2</para>",
                                 fileName,
                                 i18nc( "QFile", file.errorString().toLatin1() ) );
      KMessageBox::error( 0, msg, caption );
      *ok = false;
    }
  }

  if ( contacts.isEmpty() ) {
    if ( !(*ok) && fileNames.count() > 1 )
      KMessageBox::information( 0, i18n( "No contacts were imported, due to errors with the vCards." ) );
    else if ( *ok )
      KMessageBox::information( 0, i18n( "The vCard does not contain any contacts." ) );

    return items; // nothing to import
  }

  foreach ( const KContacts::Addressee &contact, contacts ) {
    Akonadi::Item item;
    item.setPayload<KContacts::Addressee>( contact );
    item.setMimeType( KContacts::Addressee::mimeType() );

    items << item;
  }

  return items;
}
