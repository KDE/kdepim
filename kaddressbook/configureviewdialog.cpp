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

#include <qvbox.h>

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kiconloader.h>

#include "selectfieldswidget.h"
#include "configureviewfilterpage.h"
#include "configureviewdialog.h"

ConfigureViewDialog::ConfigureViewDialog(const QString &viewName,
                                         KABC::AddressBook *document, 
                                         QWidget *parent, 
                                         const char *name)
    : KDialogBase(KDialogBase::IconList, 
                  QString( i18n("Modify View: ") ) + viewName,
                  KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                  parent, name, true, true)
{
    initGUI( document );
}

ConfigureViewDialog::~ConfigureViewDialog()
{
}

void ConfigureViewDialog::readConfig(KConfig *config)
{
  KABC::Field::List fields = KABC::Field::restoreFields( config, "KABCFields" );

  if ( fields.isEmpty() ) {
    fields = KABC::Field::defaultFields();
  }
  
  mSelectFieldsWidget->setOldFields( fields );
  mFilterPage->readConfig(config);
}

void ConfigureViewDialog::writeConfig(KConfig *config)
{
  kdDebug() << "ConfigureViewDialog::writeConfig()" << endl;

  KABC::Field::List fields = mSelectFieldsWidget->chosenFields();

  KABC::Field::saveFields( config, "KABCFields", fields );
  
  mFilterPage->writeConfig(config);
}
    
void ConfigureViewDialog::initGUI( KABC::AddressBook *document )
{
  // Add the first page, the attributes
  QVBox *page = addVBoxPage(i18n("Select Fields"), QString::null,
                            KGlobal::iconLoader()
                            ->loadIcon("view_detailed", KIcon::Desktop));

  // Add the select fields widget 
  mSelectFieldsWidget = new SelectFieldsWidget( document, page,
                                                "mSelectFieldsWidget");
                                                
  // Add the second page, the filter selection
  page = addVBoxPage(i18n("Default Filter"), QString::null,
                     KGlobal::iconLoader()
                     ->loadIcon("filter", KIcon::Desktop));
  mFilterPage = new ConfigureViewFilterPage(page, "mFilterPage");
}

#include "configureviewdialog.moc"
