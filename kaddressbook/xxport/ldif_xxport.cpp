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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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

#include <qfile.h>

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kurl.h>
#include <kabc/ldifconverter.h>

#include <kdebug.h>

#include "ldif_xxport.h"

class LDIFXXPortFactory : public KAB::XXPortFactory
{
  public:
    KAB::XXPort *xxportObject( KABC::AddressBook *ab, QWidget *parent, const char *name )
    {
      return new LDIFXXPort( ab, parent, name );
    }
};

K_EXPORT_COMPONENT_FACTORY( libkaddrbk_ldif_xxport, LDIFXXPortFactory )

LDIFXXPort::LDIFXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::XXPort( ab, parent, name )
{
  createImportAction( i18n( "Import LDIF Addressbook..." ) );
  createExportAction( i18n( "Export LDIF Addressbook..." ) );
}

/* import */

KABC::AddresseeList LDIFXXPort::importContacts( const QString& ) const
{
  KABC::AddresseeList addrList;

  QString fileName = KFileDialog::getOpenFileName( QDir::homeDirPath(), 
                      "text/x-ldif", 0 );
  if ( fileName.isEmpty() )
    return addrList;

  QFile file( fileName );
  if ( !file.open( IO_ReadOnly ) ) {
    QString msg = i18n( "<qt>Unable to open <b>%1</b> for reading.</qt>" );
    KMessageBox::error( parentWidget(), msg.arg( fileName ) );
    return addrList;
  }

  QTextStream t( &file );
  t.setEncoding( QTextStream::Latin1 );
  QString wholeFile = t.read();
  QDateTime dtDefault = QFileInfo(file).lastModified();
  file.close();

  KABC::LDIFConverter::LDIFToAddressee( wholeFile, addrList, dtDefault );

  return addrList;
}


/* export */

bool LDIFXXPort::exportContacts( const KABC::AddresseeList &list, const QString& )
{
  KURL url = KFileDialog::getSaveURL( QDir::homeDirPath() + "/addressbook.ldif", 
			"text/x-ldif" );
  if ( url.isEmpty() )
      return true;

  if ( !url.isLocalFile() ) {
    KTempFile tmpFile;
    if ( tmpFile.status() != 0 ) {
      QString txt = i18n( "<qt>Unable to open file <b>%1</b>.%2.</qt>" );
      KMessageBox::error( parentWidget(), txt.arg( url.url() )
                          .arg( strerror( tmpFile.status() ) ) );
      return false;
    }

    doExport( tmpFile.file(), list );
    tmpFile.close();

    return KIO::NetAccess::upload( tmpFile.name(), url, parentWidget() );
  } else {
    QString filename = url.path();
    QFile file( filename );

    if ( !file.open( IO_WriteOnly ) ) {
      QString txt = i18n( "<qt>Unable to open file <b>%1</b>.</qt>" );
      KMessageBox::error( parentWidget(), txt.arg( filename ) );
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
  t.setEncoding( QTextStream::UnicodeUTF8 );
  t << str;
}

#include "ldif_xxport.moc"

