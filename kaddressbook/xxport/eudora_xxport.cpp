/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>

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

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kurl.h>

#include <kdebug.h>

#include "eudora_xxport.h"

#define CTRL_C 3

K_EXPORT_KADDRESSBOOK_XXFILTER( libkaddrbk_eudora_xxport, EudoraXXPort )

EudoraXXPort::EudoraXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::XXPort( ab, parent, name )
{
  createImportAction( i18n( "Import Eudora Addressbook..." ) );
}

KABC::AddresseeList EudoraXXPort::importContacts( const QString& ) const
{
  QString fileName = KFileDialog::getOpenFileName( QDir::homeDirPath(), 
		"*.[tT][xX][tT]|" + i18n("Eudora Light Addressbook (*.txt)"), 0 );
  if ( fileName.isEmpty() )
    return KABC::AddresseeList();

  QFile file( fileName );
  if ( !file.open( IO_ReadOnly ) )
    return KABC::AddresseeList();

  QString line;
  QTextStream stream( &file );
  KABC::Addressee *a = 0;
  int bytesRead = 0;

  KABC::AddresseeList list;

  while( !stream.eof() ) {
    line = stream.readLine();
    bytesRead += line.length();
    QString tmp;

    if ( line.startsWith( "alias" ) ) {
      if ( a ) { // Write it out
        list << *a;
        delete a;
        a = 0;
        a = new KABC::Addressee();
      } else
        a = new KABC::Addressee();

      tmp = key( line ).stripWhiteSpace();
      if ( !tmp.isEmpty() )
        a->setFormattedName( tmp );

      tmp = email( line ).stripWhiteSpace();
      if ( !tmp.isEmpty() )
        a->insertEmail( tmp );
    } else if ( line.startsWith( "note" ) ) {
      if ( !a ) // Must have an alias before a note
        break;

      tmp = comment( line ).stripWhiteSpace();
      if ( !tmp.isEmpty() )
        a->setNote( tmp );

      tmp = get( line, "name" ).stripWhiteSpace();
      if ( !tmp.isEmpty() )
        a->setNameFromString( tmp );

      tmp = get( line, "address" ).stripWhiteSpace();
      if ( !tmp.isEmpty() ) {
        KABC::Address addr;
        kdDebug(5720) << tmp << endl; // dump complete address
        addr.setLabel( tmp );
        a->insertAddress( addr );
      }

      tmp = get( line, "phone" ).stripWhiteSpace();
      if ( !tmp.isEmpty() )
         a->insertPhoneNumber( KABC::PhoneNumber( tmp, KABC::PhoneNumber::Home ) );
    }
  }

  if ( a ) { // Write out address
    list << *a;
    delete a;
    a = 0;
  }

  file.close();

  return list;
}

QString EudoraXXPort::key( const QString& line) const
{
  int e;
  QString result;
  int b = line.find( '\"', 0 );

  if ( b == -1 ) { 
    b = line.find( ' ' );
    if ( b == -1 )
      return result;

    b++;
    e = line.find( ' ', b );
    result = line.mid( b, e - b );

    return result;
  }

  b++;
  e = line.find( '\"', b );
  if ( e == -1 )
    return result;

  result = line.mid( b, e - b );

  return result;
}

QString EudoraXXPort::email( const QString& line ) const
{
  int b;
  QString result;
  b = line.findRev( '\"' );
  if ( b == -1 ) { 
    b = line.findRev( ' ' );
    if ( b == -1 )
      return result;
  }
  result = line.mid( b + 1 );

  return result;
}

QString EudoraXXPort::comment( const QString& line ) const
{
  int b;
  QString result;
  uint i;
  b = line.findRev( '>' );
  if ( b == -1 ) {
    b = line.findRev( '\"' );
    if ( b == -1 )
      return result;
  }

  result = line.mid( b + 1 );
  for ( i = 0; i < result.length(); i++ ) {
    if ( result[ i ] == CTRL_C )
      result[ i ] = '\n';
  }

  return result;
}

QString EudoraXXPort::get( const QString& line, const QString& key ) const
{
  QString fd = "<" + key + ":";
  int b, e;
  uint i;

  // Find formatted key, return on error
  b = line.find( fd );
  if ( b == -1 )
    return QString::null;

  b += fd.length();
  e = line.find( '>', b );
  if ( e == -1 )
    return QString::null;

  e--;
  QString result = line.mid( b, e - b + 1 );
  for ( i = 0; i < result.length(); i++ ) {
    if ( result[ i ] == CTRL_C )
      result[ i ] = '\n';
  }

  return result;
}

#include "eudora_xxport.moc"
