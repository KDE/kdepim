/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>                   
                                                                        
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

#include <kabc/stdaddressbook.h>
#include <kabc/vcardconverter.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "addresseeutil.h"

QString AddresseeUtil::addresseesToClipboard( const KABC::Addressee::List &list )
{
  KABC::VCardConverter converter;
  QString vcard;

  KABC::Addressee::List::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    QString tmp;
    if ( converter.addresseeToVCard( *it, tmp ) )
      vcard += tmp + "\r\n";
  }

  return vcard;
}

KABC::Addressee::List AddresseeUtil::clipboardToAddressees( const QString &data )
{
  uint numVCards = data.contains( "BEGIN:VCARD", false );
  QStringList dataList = QStringList::split( "\r\n\r\n", data );

  KABC::Addressee::List addrList;
  for ( uint i = 0; i < numVCards && i < dataList.count(); ++i ) {
    KABC::VCardConverter converter;
    KABC::Addressee addr;

    if ( !converter.vCardToAddressee( dataList[ i ].stripWhiteSpace(), addr ) ) {
      KMessageBox::error( 0, i18n( "Invalid vCard format in clipboard" ) );
      continue;
    }

    addrList.append( addr );
  }

  return addrList;
}
