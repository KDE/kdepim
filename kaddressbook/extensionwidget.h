/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>                   
                                                                        
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

#ifndef EXTENSIONWIDGET_H
#define EXTENSIONWIDGET_H

#include <qwidget.h>

#include <kabc/addressbook.h>
#include <klibloader.h>

#include "viewmanager.h"

class ConfigureWidget;

class ExtensionWidget : public QWidget
{
  Q_OBJECT
  
  public:
    ExtensionWidget( ViewManager *vm, QWidget *parent, const char *name = 0 );
    ~ExtensionWidget();

    KABC::AddressBook *addressBook() const;
    /**
      @return A pointer to the view manager
    */
    ViewManager *viewManager() const;

    /**
      Returns whether there are selected contacts in the view.
     */
    bool addresseesSelected() const;

    /**
      Returns a list of contacts that are selected in the view.
      Use @ref addresseesSelected() to test if there exists selected
      contacts.
     */
    KABC::Addressee::List selectedAddressees();

    /**
      This method is called whenever the selection in the view changed.
     */
    virtual void addresseeSelectionChanged();

    /**
      This method should be reimplemented and return the i18ned title of this
      widget.
     */
    virtual QString title() const;

    /**
      This method should be reimplemented and return a unique identifier.
     */
    virtual QString identifier() const;

  signals:
    void modified( KABC::Addressee::List );

  private:
    ViewManager *mViewManager;
};

class ExtensionFactory : public KLibFactory
{
  public:
    virtual ExtensionWidget *extension( ViewManager *vm, QWidget *parent,
                                        const char *name = 0 ) = 0;

    virtual ConfigureWidget *configureWidget( ViewManager *vm,
                                              QWidget *parent,
                                              const char *name = 0 );
  protected:
    virtual QObject* createObject( QObject*, const char*, const char*,
                                   const QStringList & )
    {
      return 0;
    }
};

#endif
