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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <kabc/stdaddressbook.h>
#include <kdebug.h>

#include "addresseeutil.h"

QString AddresseeUtil::addresseeToClipboard( KABC::Addressee &a )
{
  return a.uid();
}
   
QString AddresseeUtil::addresseesToClipboard( KABC::Addressee::List &list )
{
  QStringList emails;

  KABC::Addressee::List::Iterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    if ( !(*it).fullEmail().isEmpty() )
      emails.append( (*it).fullEmail() );
  }

  return emails.join( "," );
}

KABC::Addressee::List AddresseeUtil::clipboardToAddressees( const QString& )
{
  kdDebug(5700) << "Not yet implemented" << endl;
  return KABC::Addressee::List();
}
