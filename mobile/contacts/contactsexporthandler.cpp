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

#include "contactsexporthandler.h"

#include <KABC/addressee.h>
#include <KABC/vcardconverter.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QFile>

static QString contactFileName( const KABC::Addressee &contact )
{
  if ( !contact.givenName().isEmpty() && !contact.familyName().isEmpty() )
    return QString::fromLatin1( "%1_%2" ).arg( contact.givenName() ).arg( contact.familyName() );

  if ( !contact.familyName().isEmpty() )
    return contact.familyName();

  if ( !contact.givenName().isEmpty() )
    return contact.givenName();

  if ( !contact.organization().isEmpty() )
    return contact.organization();

  return contact.uid();
}

static bool exportVCard( const QString &fileName, const QByteArray &data )
{
  KUrl url( fileName );
  if ( url.isLocalFile() && QFileInfo( url.toLocalFile() ).exists() ) {
    if ( KMessageBox::questionYesNo( 0, i18n( "Do you want to overwrite file \"%1\"?", url.toLocalFile() ) ) == KMessageBox::No )
      return false;
  }

  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly ) )
    return false;

  file.write( data );
  file.close();

  return true;
}

QString ContactsExportHandler::dialogText() const
{
  return i18n( "Which contacts shall be exported?" );
}

QString ContactsExportHandler::dialogAllText() const
{
  return i18n( "All Contacts" );
}

QString ContactsExportHandler::dialogLocalOnlyText() const
{
  return i18n( "Contacts in current folder" );
}

QStringList ContactsExportHandler::mimeTypes() const
{
  return QStringList( KABC::Addressee::mimeType() );
}

bool ContactsExportHandler::exportItems( const Akonadi::Item::List &items )
{
  KABC::Addressee::List contacts;

  foreach ( const Akonadi::Item &item, items ) {
    if ( item.hasPayload<KABC::Addressee>() )
      contacts << item.payload<KABC::Addressee>();
  }

  KABC::VCardConverter converter;
  QString fileName;

  bool ok = true;
  if ( contacts.count() == 1 ) {
    fileName = KFileDialog::getSaveFileName( QString(contactFileName( contacts.first() ) + QLatin1String( ".vcf" )), QLatin1String( "*.vcf" )  );
    if ( fileName.isEmpty() ) // user canceled export
      return true;

    ok = exportVCard( fileName, converter.createVCards( contacts, KABC::VCardConverter::v3_0 ) );
  } else {
    const QString msg = i18n( "You have selected a list of contacts, shall they be "
                              "exported to several files?" );

    switch ( KMessageBox::questionYesNo( 0, msg, QString(), KGuiItem(i18n( "Export to Several Files" ) ),
                                         KGuiItem( i18n( "Export to One File" ) ) ) ) {
      case KMessageBox::Yes:
        {
          const QString path = KFileDialog::getExistingDirectory();
          if ( path.isEmpty() )
            return true; // user canceled export

          foreach ( const KABC::Addressee &contact, contacts ) {
            fileName = path + QDir::separator() + contactFileName( contact ) + QLatin1String( ".vcf" );

            const bool tmpOk = exportVCard( fileName, converter.createVCard( contact, KABC::VCardConverter::v3_0 ) );

            ok = ok && tmpOk;
          }
        }
        break;
      case KMessageBox::No: // fall through
      default:
        {
          fileName = KFileDialog::getSaveFileName( KUrl( "addressbook.vcf" ), QLatin1String( "*.vcf" ) );
          if ( fileName.isEmpty() )
            return true; // user canceled export

          ok = exportVCard( fileName, converter.createVCards( contacts, KABC::VCardConverter::v3_0 ) );
        }
    }
  }

  return ok;
}
