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

#include "addresseeutil.h"

#include <kdebug.h>

QString AddresseeUtil::addresseeToClipboard(KABC::Addressee &a)
{
  // The temp solution is to use the email address. See the
  // AddresseeUtil documentation.
  return a.fullEmail();
}
   
QString AddresseeUtil::addresseesToClipboard(KABC::Addressee::List &aList)
{
  QString clipboard;
  bool first = true;
  KABC::Addressee::List::Iterator iter;
  for (iter = aList.begin(); iter != aList.end(); ++iter)
  {
    if (!first)
      clipboard += ", ";
      
    clipboard += (*iter).fullEmail();
    first = false;
  }
  
  return clipboard;
}

KABC::Addressee::List AddresseeUtil::clipboardToAddressees(const QString &)
{
  // Need a VCard parser
  kdDebug() << "AddresseeUtil::clipboardToAddressees: not implemented" << endl;
  
  return KABC::Addressee::List();
}
