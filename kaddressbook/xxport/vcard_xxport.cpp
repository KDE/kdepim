/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <qfile.h>

#include <kabc/vcardconverter.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kurl.h>

#include "xxportmanager.h"

#include "vcard_xxport.h"

class VCardXXPortFactory : public XXPortFactory
{
  public:
    XXPortObject *xxportObject( KABC::AddressBook *ab, QWidget *parent, const char *name )
    {
      return new VCardXXPort( ab, parent, name );
    }
};

extern "C"
{
  void *init_libkaddrbk_vcard_xxport()
  {
    return ( new VCardXXPortFactory() );
  }
}


VCardXXPort::VCardXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : XXPortObject( ab, parent, name )
{
  createImportAction( i18n( "Import vCard..." ) );
  createExportAction( i18n( "Export vCard 2.1..." ), "v21" );
  createExportAction( i18n( "Export vCard 3.0..." ), "v30" );
}

bool VCardXXPort::exportContacts( const KABC::AddresseeList &list, const QString &data )
{
  QString name;

  if ( list.count() == 1 )
    name = list[ 0 ].givenName() + "_" + list[ 0 ].familyName() + ".vcf";
  else
    name = "addressbook.vcf";

  QString fileName = KFileDialog::getSaveFileName( name );
  if ( fileName.isEmpty() )
    return true;

  QFile outFile( fileName );
  if ( !outFile.open( IO_WriteOnly ) ) {
    QString text = i18n( "<qt>Unable to open file <b>%1</b> for export.</qt>" );
    KMessageBox::error( parentWidget(), text.arg( fileName ) );
    return false;
  }

  QTextStream t( &outFile );
  t.setEncoding( QTextStream::UnicodeUTF8 );

  KABC::Addressee::List::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    KABC::VCardConverter converter;
    QString vcard;

    KABC::VCardConverter::Version version;
    if ( data == "v21" )
      version = KABC::VCardConverter::v2_1;
    else
      version = KABC::VCardConverter::v3_0;

    converter.addresseeToVCard( *it, vcard, version );
    t << vcard << "\r\n\r\n";
  }

  outFile.close();

  return true;
}

KABC::AddresseeList VCardXXPort::importContacts( const QString& ) const
{
  QString fileName;
  KABC::AddresseeList addrList;
  KURL url;

  if ( XXPortManager::importURL.isEmpty() )
    url = KFileDialog::getOpenURL( QString::null, "*.vcf|vCards", 0,
                                   i18n( "Select vCard to Import" ) );
  else
    url = XXPortManager::importURL;

  if ( url.isEmpty() )
    return addrList;

  QString caption( i18n( "vCard Import Failed" ) );
  if ( KIO::NetAccess::download( url, fileName ) ) {
    KABC::VCardConverter converter;
    QFile file( fileName );

    file.open( IO_ReadOnly );
    QByteArray rawData = file.readAll();
    file.close();

    QString data = QString::fromUtf8( rawData.data(), rawData.size() + 1 );
    uint numVCards = data.contains( "BEGIN:VCARD", false );
    QStringList dataList = QStringList::split( "\r\n\r\n", data );

    for ( uint i = 0; i < numVCards && i < dataList.count(); ++i ) {
      KABC::Addressee addr;
      bool ok = false;

      if ( dataList[ i ].contains( "VERSION:3.0" ) )
        ok = converter.vCardToAddressee( dataList[ i ], addr, KABC::VCardConverter::v3_0 );
      else if ( dataList[ i ].contains( "VERSION:2.1" ) )
        ok = converter.vCardToAddressee( dataList[ i ], addr, KABC::VCardConverter::v2_1 );
      else {
        KMessageBox::sorry( parentWidget(), i18n( "Not supported vCard version." ),
                            caption );
        continue;
      }

      if ( !addr.isEmpty() && ok )
        addrList.append( addr );
      else {
        QString text = i18n( "The selected file does not include a valid vCard. "
                             "Please check the file and try again." );
        KMessageBox::sorry( parentWidget(), text, caption );
      }
    }

    if ( !url.isLocalFile() )
      KIO::NetAccess::removeTempFile( fileName );

  } else {
    QString text = i18n( "<qt>Unable to access <b>%1</b>.</qt>" );
    KMessageBox::error( parentWidget(), text.arg( url.url() ), caption );
  }

  return addrList;
}

#include "vcard_xxport.moc"
