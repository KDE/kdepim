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

#include <tqfile.h>

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

K_EXPORT_KADDRESSBOOK_XXFILTER( libkaddrbk_ldif_xxport, LDIFXXPort )

LDIFXXPort::LDIFXXPort( KABC::AddressBook *ab, TQWidget *parent, const char *name )
  : KAB::XXPort( ab, parent, name )
{
  createImportAction( i18n( "Import LDIF Addressbook..." ) );
  createExportAction( i18n( "Export LDIF Addressbook..." ) );
}

/* import */

KABC::AddresseeList LDIFXXPort::importContacts( const TQString& ) const
{
  KABC::AddresseeList addrList;

  TQString fileName = KFileDialog::getOpenFileName( TQDir::homeDirPath(),
                      "text/x-ldif", 0 );
  if ( fileName.isEmpty() )
    return addrList;

  TQFile file( fileName );
  if ( !file.open( IO_ReadOnly ) ) {
    TQString msg = i18n( "<qt>Unable to open <b>%1</b> for reading.</qt>" );
    KMessageBox::error( parentWidget(), msg.arg( fileName ) );
    return addrList;
  }

  TQTextStream t( &file );
  t.setEncoding( TQTextStream::Latin1 );
  TQString wholeFile = t.read();
  TQDateTime dtDefault = TQFileInfo(file).lastModified();
  file.close();

  KABC::LDIFConverter::LDIFToAddressee( wholeFile, addrList, dtDefault );

  return addrList;
}


/* export */

bool LDIFXXPort::exportContacts( const KABC::AddresseeList &list, const TQString& )
{
  KURL url = KFileDialog::getSaveURL( TQDir::homeDirPath() + "/addressbook.ldif",
			"text/x-ldif" );
  if ( url.isEmpty() )
      return true;

  if( TQFileInfo(url.path()).exists() ) {
      if(KMessageBox::questionYesNo( parentWidget(), i18n("Do you want to overwrite file \"%1\"").arg( url.path()) ) == KMessageBox::No)   
        return false;
  }


  if ( !url.isLocalFile() ) {
    KTempFile tmpFile;
    if ( tmpFile.status() != 0 ) {
      TQString txt = i18n( "<qt>Unable to open file <b>%1</b>.%2.</qt>" );
      KMessageBox::error( parentWidget(), txt.arg( url.url() )
                          .arg( strerror( tmpFile.status() ) ) );
      return false;
    }

    doExport( tmpFile.file(), list );
    tmpFile.close();

    return KIO::NetAccess::upload( tmpFile.name(), url, parentWidget() );
  } else {
    TQString filename = url.path();
    TQFile file( filename );

    if ( !file.open( IO_WriteOnly ) ) {
      TQString txt = i18n( "<qt>Unable to open file <b>%1</b>.</qt>" );
      KMessageBox::error( parentWidget(), txt.arg( filename ) );
      return false;
    }

    doExport( &file, list );
    file.close();

    return true;
  }
}

void LDIFXXPort::doExport( TQFile *fp, const KABC::AddresseeList &list )
{
  TQString str;
  KABC::LDIFConverter::addresseeToLDIF( list, str );

  TQTextStream t( fp );
  t.setEncoding( TQTextStream::UnicodeUTF8 );
  t << str;
}

#include "ldif_xxport.moc"

