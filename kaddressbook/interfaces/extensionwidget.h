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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#ifndef KAB_EXTENSIONWIDGET_H
#define KAB_EXTENSIONWIDGET_H

#include <qwidget.h>

#include <kabc/addressbook.h>
#include <klibloader.h>

#define KAB_EXTENSIONWIDGET_PLUGIN_VERSION 1

namespace KAB {
class Core;
class ConfigureWidget;

class ExtensionWidget : public QWidget
{
  Q_OBJECT
  
  public:
    ExtensionWidget( Core *core, QWidget *parent, const char *name = 0 );
    ~ExtensionWidget();

    /**
      @return A pointer to the core object
    */
    KAB::Core *core() const;

    /**
      Returns whether there are selected contacts in the view.
     */
    bool contactsSelected() const;

    /**
      Returns a list of contacts that are selected in the view.
      Use @ref addresseesSelected() to test if there exists selected
      contacts.
     */
    KABC::Addressee::List selectedContacts();

    /**
      This method is called whenever the selection in the view changed.
     */
    virtual void contactsSelectionChanged();

    /**
      This method should be reimplemented and return the i18ned title of this
      widget.
     */
    virtual QString title() const = 0;

    /**
      This method should be reimplemented and return a unique identifier.
     */
    virtual QString identifier() const = 0;

  signals:
    void modified( const KABC::Addressee::List &list );

  private:
    KAB::Core *mCore;

    class ExtensionWidgetPrivate;
    ExtensionWidgetPrivate *d;
};

class ExtensionFactory : public KLibFactory
{
  public:
    virtual ExtensionWidget *extension( KAB::Core *core, QWidget *parent,
                                        const char *name = 0 ) = 0;

    virtual ConfigureWidget *configureWidget( QWidget*, const char* = 0 )
    {
      return 0;
    }

    virtual bool configureWidgetAvailable() { return false; }

    /**
      This method should return the same identifier like the config
      widget.
     */
    virtual QString identifier() const = 0;

  protected:
    virtual QObject* createObject( QObject*, const char*, const char*,
                                   const QStringList & )
    {
      return 0;
    }
};

}

#endif
