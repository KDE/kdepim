#ifndef VIEWWRAPPER_H
#define VIEWWRAPPER_H

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

class QWidget;
class KAddressBookView;
class ConfigureViewDialog;

namespace KABC { class AddressBook; }

/** The ViewWrapper class is a lightweight class that will be instantiated
* for each view type. It's job it to provide meta data about the
* view type as well as some utility functions for creating the actual
* view object and its config dialog.
*
* A view implementer will need to subclass from this class to fill in
* the createView() and createViewConfigDialog() methods.
*/
class ViewWrapper
{
  public:
    ViewWrapper();
    virtual ~ViewWrapper();
    
    /** @return The type of the view. This is normally a small one word
    * string (ie: Table, Icon, Tree, etc).
    */
    virtual QString type() const = 0;
    
    /** @return The description of the view. This should be a 3 to
    * 4 line string (don't actually use return characters in the string)
    * describing the features offered by the view.
    */
    virtual QString description() const = 0;
    
    /** Creates a view of the given type and returns the view. The caller
    * is responsible for managing the memory allocated by the view.
    */
    virtual KAddressBookView *createView(KABC::AddressBook *doc,
                                         QWidget *parent, 
                                         const char *name = 0) = 0;
                                 
    /** Creates a config dialog for the view type. The default 
    * implementation will return a ViewConfigDialog. This default
    * dialog will allow the user to set the visible fields only. If
    * you need more config options (as most views will), this method
    * can be overloaded to return your sublcass of ViewConfigDialog.
    * If this method is over loaded the base classes method should
    * <B>not</B> be called.
    */
    virtual ConfigureViewDialog *createConfigureViewDialog(
                                                     const QString &viewName,
                                                     KABC::AddressBook *doc,
                                                     QWidget *parent,
                                                     const char *name = 0); 
};

#endif
