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

#include "emailsexporthandler.h"

#include <kfiledialog.h>
#include <klocale.h>
#include <kmbox/mbox.h>
#include <kmessagebox.h>
#include <kmime/kmime_message.h>

QString EmailsExportHandler::dialogText() const
{
  return i18n( "Which emails shall be exported?" );
}

QString EmailsExportHandler::dialogAllText() const
{
  return i18n( "All Emails" );
}

QString EmailsExportHandler::dialogLocalOnlyText() const
{
  return i18n( "Emails in current folder" );
}

QStringList EmailsExportHandler::mimeTypes() const
{
  return QStringList( KMime::Message::mimeType() );
}

bool EmailsExportHandler::exportItems( const Akonadi::Item::List &items )
{
  QList<KMime::Message::Ptr> messages;

  foreach ( const Akonadi::Item &item, items ) {
    if ( item.hasPayload<KMime::Message::Ptr>() )
      messages << item.payload<KMime::Message::Ptr>();
  }

  const QString fileName = KFileDialog::getSaveFileName( KUrl( QLatin1String( "emails.mbox" ) ), QLatin1String( "*.mbox" ) );
  if ( fileName.isEmpty() ) // user canceled export
    return true;

  KMBox::MBox mbox;
  if ( !mbox.load( fileName ) ) {
    KMessageBox::error( 0, i18n( "Unable to open MBox file %1", fileName ) );
    return false;
  }

  foreach ( const KMime::Message::Ptr &message, messages ) {
    mbox.appendMessage( message );
  }

  if ( !mbox.save() ) {
    KMessageBox::error( 0, i18n( "Unable to save emails to MBox file %1", fileName ) );
    return false;
  }

  return true;
}
