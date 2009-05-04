/*
    This file is part of KContactManager.
    Copyright (c) 2000 - 2009 Oliver Strutynski <olistrut@gmx.de>
                              Tobias Koenig <tokoe@kde.org>
                              Sebastian Sauer <sebsauer@kdab.net>

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

#include "ldap_xxport.h"
#include "ldapsearchdialog.h"

//#include <QtCore/QFile>
//#include <QtCore/QTextStream>

//#include <kabc/ldifconverter.h>
//#include <kcodecs.h>
//#include <kfiledialog.h>
//#include <kio/netaccess.h>
//#include <klocale.h>
//#include <kmessagebox.h>
//#include <ktemporaryfile.h>
//#include <kurl.h>
#include <kdebug.h>

LDAPXXPort::LDAPXXPort( QWidget *parentWidget )
  : XXPort( parentWidget )
{
}

KABC::Addressee::List LDAPXXPort::importContacts() const
{
  kDebug()<<"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";


  KABC::Addressee::List contacts;

  KABC::AddressBook *ab = 0;
  KABCore *core = 0;
  LDAPSearchDialog dlg(ab, core, parentWidget());
  dlg.exec();
  contacts = dlg.m_result;

/*
  const QString fileName = KFileDialog::getOpenFileName( QDir::homePath(), "text/x-ldif", 0 );
  if ( fileName.isEmpty() )
    return contacts;

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
    const QString msg = i18n( "<qt>Unable to open <b>%1</b> for reading.</qt>", fileName );
    KMessageBox::error( parentWidget(), msg );
    return contacts;
  }

  QTextStream stream( &file );
  stream.setCodec( "ISO 8859-1" );

  const QString wholeFile = stream.readAll();
  const QDateTime dtDefault = QFileInfo( file ).lastModified();
  file.close();

  KABC::LDAPConverter::LDAPToAddressee( wholeFile, contacts, dtDefault );
*/
  return contacts;
}

bool LDAPXXPort::exportContacts( const KABC::Addressee::List &contacts ) const
{
    Q_UNUSED(contacts);
    return false;
}
