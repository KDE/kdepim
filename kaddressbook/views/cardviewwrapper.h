#ifndef CARDVIEWWRAPPER_H
#define CARDVIEWWRAPPER_H

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

#include <qstring.h>
#include <klocale.h>
#include "kaddressbookcardview.h"
#include "configurecardviewdialog.h"
#include "viewwrapper.h"

namespace KABC { class AddressBook; }

class CardViewWrapper : public ViewWrapper
{
  public:
    CardViewWrapper() : ViewWrapper() {}
    virtual ~CardViewWrapper() {}
    
    virtual QString type() const { return "Card"; };
    
    virtual QString description() const
    {
      // Needs a better description.
      QString desc = i18n("Rolodex style cards represent contacts.");
      
      return desc;
    }
    
    virtual KAddressBookView *createView(KABC::AddressBook *doc,
                                         QWidget *parent, 
                                         const char *name)
    {
      return new KAddressBookCardView(doc, parent, name);
    }
    
    virtual ConfigureViewDialog *createConfigureViewDialog(
                                                     const QString &viewName,
                                                     KABC::AddressBook *doc,
                                                     QWidget *parent,
                                                     const char *name)
    {
      return new ConfigureCardViewDialog(viewName, doc, parent, name);
    }

};

#endif
