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

#include <kabc/vcardtool.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kurl.h>

#include "xxportmanager.h"

#include "vcard_xxport.h"

class VCardXXPortFactory : public KAB::XXPortFactory
{
  public:
    KAB::XXPort *xxportObject( KABC::AddressBook *ab, QWidget *parent, const char *name )
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
  : KAB::XXPort( ab, parent, name )
{
  createImportAction( i18n( "Import vCard..." ) );
  createExportAction( i18n( "Export vCard 2.1..." ), "v21" );
  createExportAction( i18n( "Export vCard 3.0..." ), "v30" );
}

bool VCardXXPort::exportContacts( const KABC::AddresseeList &list, const QString &data )
{
  KABC::VCardTool tool;
  KURL url;

  bool ok = true;
  if ( list.count() == 1 ) {
    url = KFileDialog::getSaveURL( list[ 0 ].givenName() + "_" + list[ 0 ].familyName() + ".vcf" );
    if ( url.isEmpty() )
      return true;

    if ( data == "v21" )
      ok = doExport( url, tool.createVCards( list, KABC::VCard::v2_1 ) );
    else
      ok = doExport( url, tool.createVCards( list, KABC::VCard::v3_0 ) );
  } else {
    QString msg = i18n( "You have selected a list of contacts, shall they be"
                        "exported to several files?" );

    switch ( KMessageBox::questionYesNo( parentWidget(), msg ) ) {
      case KMessageBox::Yes: {
        KURL baseUrl = KFileDialog::getExistingURL();
        if ( baseUrl.isEmpty() )
          return true;

        KABC::AddresseeList::ConstIterator it;
        for ( it = list.begin(); it != list.end(); ++it ) {
          url = baseUrl.url() + "/" + (*it).givenName() + "_" + (*it).familyName() + ".vcf";

          bool tmpOk;
          KABC::AddresseeList tmpList;
          tmpList.append( *it );

          if ( data == "v21" )
            tmpOk = doExport( url, tool.createVCards( tmpList, KABC::VCard::v2_1 ) );
          else
            tmpOk = doExport( url, tool.createVCards( tmpList, KABC::VCard::v3_0 ) );

          ok = ok && tmpOk;
        }
        break;
      }
      case KMessageBox::No:
      default: {
        url = KFileDialog::getSaveURL( "addressbook.vcf" );
        if ( url.isEmpty() )
          return true;

        if ( data == "v21" )
          ok = doExport( url, tool.createVCards( list, KABC::VCard::v2_1 ) );
        else
          ok = doExport( url, tool.createVCards( list, KABC::VCard::v3_0 ) );
      }
    }
  }

  return ok;
}

KABC::AddresseeList VCardXXPort::importContacts( const QString& ) const
{
  QString fileName;
  KABC::AddresseeList addrList;
  KURL::List urls;

  if ( !XXPortManager::importData.isEmpty() )
    addrList = parseVCard( XXPortManager::importData );
  else {
    if ( XXPortManager::importURL.isEmpty() )
      urls = KFileDialog::getOpenURLs( QString::null, "*.vcf|vCards", parentWidget(),
                                       i18n( "Select vCard to Import" ) );
    else
      urls.append( XXPortManager::importURL );

    if ( urls.count() == 0 )
      return addrList;

    QString caption( i18n( "vCard Import Failed" ) );
    KURL::List::Iterator it;
    for ( it = urls.begin(); it != urls.end(); ++it ) {
      if ( KIO::NetAccess::download( *it, fileName ) ) {

        QFile file( fileName );

        file.open( IO_ReadOnly );
        QByteArray rawData = file.readAll();
        file.close();

        QString data = QString::fromUtf8( rawData.data(), rawData.size() + 1 );
        addrList += parseVCard( data );

        KIO::NetAccess::removeTempFile( fileName );
      } else {
        QString text = i18n( "<qt>Unable to access <b>%1</b>.</qt>" );
        KMessageBox::error( parentWidget(), text.arg( (*it).url() ), caption );
      }
    }
  }

  return addrList;
}

KABC::AddresseeList VCardXXPort::parseVCard( const QString &data ) const
{
  KABC::VCardTool tool;

  return tool.parseVCards( data );
}

bool VCardXXPort::doExport( const KURL &url, const QString &data )
{
  KTempFile tmpFile;
  tmpFile.setAutoDelete( true );

  QTextStream stream( tmpFile.file() );
  stream.setEncoding( QTextStream::UnicodeUTF8 );

  stream << data;
  tmpFile.close();

  return KIO::NetAccess::upload( tmpFile.name(), url, parentWidget() );
}

#include "vcard_xxport.moc"
