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

#include "addresseeeditordialog.h"

#include <qlayout.h>

#include <klocale.h>
#include <kdebug.h>

#include "addresseeeditorwidget.h"

AddresseeEditorDialog::AddresseeEditorDialog(QWidget *parent, const char *name)
  : KDialogBase(KDialogBase::Plain, i18n("Edit Contact"), 
                KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Apply,
                KDialogBase::Ok, parent, name, false)
{
  QWidget *page = plainPage();
  
  QVBoxLayout *layout = new QVBoxLayout(page);

  mEditorWidget = new AddresseeEditorWidget( page );
  connect(mEditorWidget, SIGNAL(modified()), this, SLOT(widgetModified()));
  layout->addWidget( mEditorWidget );
  
  enableButton(KDialogBase::Apply, false);
}

AddresseeEditorDialog::~AddresseeEditorDialog()
{
  kdDebug() << "~AddresseeEditorDialog()" << endl;

  emit editorDestroyed( mEditorWidget->addressee().uid() );
}

void AddresseeEditorDialog::setAddressee(const KABC::Addressee &a)
{
  enableButton(KDialogBase::Apply, false);
  
  mEditorWidget->setAddressee(a);
}

KABC::Addressee AddresseeEditorDialog::addressee()
{
  return mEditorWidget->addressee();
}

bool AddresseeEditorDialog::dirty()
{
  return mEditorWidget->dirty();
}

void AddresseeEditorDialog::slotApply()
{
  mEditorWidget->save();

  emit addresseeModified(mEditorWidget->addressee());
  
  enableButton(KDialogBase::Apply, false);
  
  KDialogBase::slotApply();
}

void AddresseeEditorDialog::slotOk()
{
  slotApply();
  
  KDialogBase::slotOk();
  
  // Destroy this dialog
  delayedDestruct();
}

void AddresseeEditorDialog::widgetModified()
{
  enableButton(KDialogBase::Apply, true);
}
  
  
void AddresseeEditorDialog::slotCancel()
{
  KDialogBase::slotCancel();
  
  // Destroy this dialog
  delayedDestruct();
}
    
#include "addresseeeditordialog.moc"
