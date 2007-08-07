/*
    This file is part of KAddressbook.
    Copyright (c) 2000 - 2002 Oliver Strutynski <olistrut@gmx.de>
    Copyright (c) 2002 - 2003 Helge Deller <deller@kde.org>
                              Tobias Koenig <tokoe@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

/*
    Description:
    The LDAP Data Interchange Format (LDIF) is a common ASCII-text based
    Internet interchange format. Most programs allow you to export data in
    LDIF format and e.g. Netscape and Mozilla store by default their
    Personal Address Book files in this format.
    This import and export filter reads and writes any LDIF version 1 files
    into your KDE Addressbook.
*/

#include <QFile>
//Added by qt3to4:
#include <QTextStream>

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>
#include <kurl.h>
#include <kabc/ldifconverter.h>

#include <kdebug.h>

#include "ldif_xxport.h"

K_EXPORT_KADDRESSBOOK_XXFILTER( libkaddrbk_ldif_xxport, LDIFXXPort )

LDIFXXPort::LDIFXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::XXPort( ab, parent, name )
{
  createImportAction( i18n( "Import LDIF Addressbook..." ) );
  createExportAction( i18n( "Export LDIF Addressbook..." ) );
}

/* import */

KABC::Addressee::List LDIFXXPort::importContacts( const QString& ) const
{
  KABC::Addressee::List addrList;

  QString fileName = KFileDialog::getOpenFileName( QDir::homePath(),
                      "text/x-ldif", 0 );
  if ( fileName.isEmpty() )
    return addrList;

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    QString msg = i18n( "<qt>Unable to open <b>%1</b> for reading.</qt>", fileName );
    KMessageBox::error( parentWidget(), msg );
    return addrList;
  }

  QTextStream t( &file );
  t.setCodec( "ISO 8859-1" );

  QString wholeFile = t.readAll();
  QDateTime dtDefault = QFileInfo(file).lastModified();
  file.close();

  // FIXME porting conversion temporarily needed
  KABC::AddresseeList l;
  KABC::LDIFConverter::LDIFToAddressee( wholeFile, l, dtDefault );

  KABC::AddresseeList::ConstIterator it( l.constBegin() );
  for ( ; it != l.constEnd(); ++ it ) {
    addrList.append( *it );
  }

  return addrList;
}


/* export */

bool LDIFXXPort::exportContacts( const KABC::AddresseeList &list, const QString& )
{
  KUrl url = KFileDialog::getSaveUrl( KUrl( QDir::homePath() + "/addressbook.ldif" ),
			"text/x-ldif" );
  if ( url.isEmpty() )
      return true;

  if ( !url.isLocalFile() ) {
    KTemporaryFile tmpFile;
    if ( !tmpFile.open() ) {
      QString txt = i18n( "<qt>Unable to open file <b>%1</b></qt>", url.url());
      KMessageBox::error( parentWidget(), txt );
      return false;
    }

    doExport( &tmpFile, list );
    tmpFile.flush();

    return KIO::NetAccess::upload( tmpFile.fileName(), url, parentWidget() );
  } else {
    QString filename = url.path();
    QFile file( filename );

    if ( !file.open( QIODevice::WriteOnly ) ) {
      QString txt = i18n( "<qt>Unable to open file <b>%1</b>.</qt>", filename );
      KMessageBox::error( parentWidget(), txt );
      return false;
    }

    doExport( &file, list );
    file.close();

    return true;
  }
}

void LDIFXXPort::doExport( QFile *fp, const KABC::AddresseeList &list )
{
  QString str;
  KABC::LDIFConverter::addresseeToLDIF( list, str );

  QTextStream t( fp );
  t.setCodec( "UTF-8" );
  t << str;
}

#include "ldif_xxport.moc"

