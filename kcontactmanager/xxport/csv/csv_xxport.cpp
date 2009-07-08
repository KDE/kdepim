/*
    This file is part of KContactManager.
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "csv_xxport.h"

#include "contactfields.h"
#include "csvimportdialog.h"

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>
#include <kurl.h>

#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>

CsvXXPort::CsvXXPort( QWidget *parent )
  : XXPort( parent )
{
}

bool CsvXXPort::exportContacts( const KABC::Addressee::List &contacts ) const
{
  KUrl url = KFileDialog::getSaveUrl( KUrl( "addressbook.csv" ) );
  if ( url.isEmpty() )
      return true;

  if ( QFileInfo(url.isLocalFile() ? url.toLocalFile() : url.path()).exists() ) {
    if ( KMessageBox::questionYesNo( parentWidget(), i18n( "Do you want to overwrite file \"%1\"", url.isLocalFile() ? url.toLocalFile() : url.path() ) ) == KMessageBox::No )
      return true;
  }

  if ( !url.isLocalFile() ) {
    KTemporaryFile tmpFile;
    if ( !tmpFile.open() ) {
      const QString msg = i18n( "<qt>Unable to open file <b>%1</b></qt>", url.url() );
      KMessageBox::error( parentWidget(), msg );
      return false;
    }

    exportToFile( &tmpFile, contacts );
    tmpFile.flush();

    return KIO::NetAccess::upload( tmpFile.fileName(), url, parentWidget() );

  } else {
    QFile file( url.toLocalFile() );
    if ( !file.open( QIODevice::WriteOnly ) ) {
      const QString msg = i18n( "<qt>Unable to open file <b>%1</b>.</qt>", url.toLocalFile() );
      KMessageBox::error( parentWidget(), msg );
      return false;
    }

    exportToFile( &file, contacts );
    file.close();

    return true;
  }
}

void CsvXXPort::exportToFile( QFile *file, const KABC::Addressee::List &contacts ) const
{
  QTextStream stream( file );
  stream.setCodec( QTextCodec::codecForLocale() );

  ContactFields::Fields fields = ContactFields::allFields();
  fields.remove( ContactFields::Undefined );

  bool first = true;

  // First output the column headings
  for ( int i = 0; i < fields.count(); ++i ) {
    if ( !first )
      stream << ",";

    stream << "\"" << ContactFields::label( fields.at( i ) ) << "\"";
    first = false;
  }
  stream << "\n";

  // Then all the contacts
  for ( int i = 0; i < contacts.count(); ++i ) {

    const KABC::Addressee contact = contacts.at( i );
    first = true;

    for ( int j = 0; j < fields.count(); ++j ) {
      if ( !first )
        stream << ",";

      stream << '\"' << ContactFields::value( fields.at( j ), contact ).replace( '\n', "\\n" ) << '\"';
      first = false;
    }

    stream << "\n";
  }
}

KABC::Addressee::List CsvXXPort::importContacts() const
{
  CSVImportDialog dlg( parentWidget() );
  if ( !dlg.exec() )
    return KABC::Addressee::List();

  return dlg.contacts();
}
