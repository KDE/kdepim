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

#ifndef CONFIGUREVIEWDIALOG_H
#define CONFIGUREVIEWDIALOG_H

#include <kdialogbase.h>

class ConfigureViewFilterPage;
class SelectFieldsWidget;

namespace KABC { class AddressBook; }

/**
  This dialog is the base class for all view configuration dialogs. The 
  author of a view may wish to inherit from this dialog and add config pages
  that add custom config options. The default implementation of this dialog
  is to show a page with the select fields widget. For simple views this may
  be sufficient.

  This dialog is based on an IconList version of KDialogBase. See the 
  KDialogBase documentation for more information on adding pages.
*/
class ConfigureViewDialog : public KDialogBase
{
  Q_OBJECT
    
  public:
    ConfigureViewDialog( const QString &viewName, KABC::AddressBook *ab,
                         QWidget *parent, const char *name = 0 );
    virtual ~ConfigureViewDialog();
    
    /**
      Reads the configuration from the config object and sets the values
      in the GUI. If this method is overloaded, be sure to call the base
      class's method.
    
      Do not change the group of the config object in this method.
     */
    virtual void readConfig( KConfig *config );
    
    /**
      Writes the configuration from the GUI to the config object. If this
      method is overloaded, be sure to call the base class's method.
     
      Do not change the group of the config object in this method.
     */
    virtual void writeConfig( KConfig *config );
 
  private:
    void initGUI( KABC::AddressBook * );
    
    SelectFieldsWidget *mSelectFieldsWidget;
    ConfigureViewFilterPage *mFilterPage;
};

#endif
