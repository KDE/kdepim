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

#include <qlayout.h>

#include <kdebug.h>
#include <klocale.h>

#include "core.h"
#include "addresseeeditorwidget.h"

#include "addresseeeditordialog.h"

AddresseeEditorDialog::AddresseeEditorDialog( KAB::Core *core,
                                              QWidget *parent, const char *name )
  : KDialogBase( KDialogBase::Plain, i18n( "Edit Contact" ), 
                 KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Apply,
                 KDialogBase::Ok, parent, name, false )
{
  kdDebug(5720) << "AddresseeEditorDialog()" << endl;

  QWidget *page = plainPage();

  QVBoxLayout *layout = new QVBoxLayout( page );

  mEditorWidget = new AddresseeEditorWidget( core, false, page );
  connect( mEditorWidget, SIGNAL( modified( const KABC::Addressee::List& ) ),
           SLOT( widgetModified() ) );
  layout->addWidget( mEditorWidget );

  enableButton( KDialogBase::Apply, false );
}

AddresseeEditorDialog::~AddresseeEditorDialog()
{
  kdDebug(5720) << "~AddresseeEditorDialog()" << endl;

  emit editorDestroyed( mEditorWidget->addressee().uid() );
}

void AddresseeEditorDialog::setAddressee( const KABC::Addressee &addr )
{
  enableButton( KDialogBase::Apply, false );

  mEditorWidget->setAddressee( addr );
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
  if ( mEditorWidget->dirty() ) {
    mEditorWidget->save();
    emit contactModified( mEditorWidget->addressee() );
  }

  enableButton( KDialogBase::Apply, false );

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
  enableButton( KDialogBase::Apply, true );
}

void AddresseeEditorDialog::slotCancel()
{
  KDialogBase::slotCancel();

  // Destroy this dialog
  delayedDestruct();
}

#include "addresseeeditordialog.moc"
