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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

#include <kaccelmanager.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "emaileditwidget.h"

EmailEditWidget::EmailEditWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *topLayout = new QGridLayout( this, 2, 2, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Email:" ), this );
  topLayout->addWidget( label, 0, 0 );

  mEmailEdit = new KLineEdit( this );
  connect( mEmailEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  connect( mEmailEdit, SIGNAL( textChanged( const QString& ) ),
           SIGNAL( modified() ) );
  label->setBuddy( mEmailEdit );
  topLayout->addWidget( mEmailEdit, 0, 1 );

  QPushButton *editButton = new QPushButton( i18n( "Edit Email Addresses..." ), this);
  connect( editButton, SIGNAL( clicked() ), SLOT( edit() ) );
  topLayout->addMultiCellWidget( editButton, 1, 1, 0, 1 );

  topLayout->activate();
}

EmailEditWidget::~EmailEditWidget()
{
}

void EmailEditWidget::setEmails( const QStringList &list )
{
  mEmailList = list;

  bool blocked = mEmailEdit->signalsBlocked();
  mEmailEdit->blockSignals( true );
  if ( list.count() > 0 )
    mEmailEdit->setText( list[ 0 ] );
  else
    mEmailEdit->setText( "" );
  mEmailEdit->blockSignals( blocked );
}

QStringList EmailEditWidget::emails()
{
  if ( mEmailEdit->text().isEmpty() ) {
    if ( mEmailList.count() > 0 )
      mEmailList.remove( mEmailList.begin() );
  } else {
    if ( mEmailList.count() > 0 )
      mEmailList.remove( mEmailList.begin() );

    mEmailList.prepend( mEmailEdit->text() );
  }

  return mEmailList;
}

void EmailEditWidget::edit()
{
  EmailEditDialog dlg( mEmailList, this );

  if ( dlg.exec() ) {
    if ( dlg.changed() ) {
      mEmailList = dlg.emails();
      mEmailEdit->setText( mEmailList[ 0 ] );
      emit modified();
    }
  }
}

void EmailEditWidget::textChanged( const QString &text )
{
  if ( mEmailList.count() > 0 )
    mEmailList.remove( mEmailList.begin() );

  mEmailList.prepend( text );
}


EmailEditDialog::EmailEditDialog( const QStringList &list, QWidget *parent,
                                  const char *name )
  : KDialogBase( KDialogBase::Plain, i18n( "Edit Email Addresses" ),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, true )
{
  QWidget *page = plainPage();

  QGridLayout *topLayout = new QGridLayout( page, 4, 3, marginHint(), 
                                            spacingHint() );

  QLabel *label = new QLabel( i18n( "Email address:" ), page );
  topLayout->addWidget( label, 0, 0 );

  mEmailEdit = new KLineEdit( page );
  label->setBuddy( mEmailEdit );
  topLayout->addWidget( mEmailEdit, 0, 1 );
  connect( mEmailEdit, SIGNAL( returnPressed() ), SLOT( add() ) );
  connect( mEmailEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( emailChanged() ) );

  mAddButton = new QPushButton( i18n( "Add" ), page );
  mAddButton->setEnabled( false );
  connect( mAddButton, SIGNAL( clicked() ), SLOT( add() ) );
  topLayout->addWidget( mAddButton, 0, 2 );

  mEmailListBox = new QListBox( page );

  // Make sure there is room for the scrollbar
  mEmailListBox->setMinimumHeight( mEmailListBox->sizeHint().height() + 30 );
  connect( mEmailListBox, SIGNAL( highlighted( int ) ),
           SLOT( selectionChanged( int ) ) );
  topLayout->addMultiCellWidget( mEmailListBox, 1, 3, 0, 1 );

  mEditButton = new QPushButton( i18n( "Change" ), page );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( edit() ) );
  topLayout->addWidget( mEditButton, 1, 2 );

  mRemoveButton = new QPushButton( i18n( "Remove" ), page );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( remove() ) );
  topLayout->addWidget( mRemoveButton, 2, 2 );

  mStandardButton = new QPushButton( i18n( "Set Standard" ), page );
  connect( mStandardButton, SIGNAL( clicked() ), SLOT( standard() ) );
  topLayout->addWidget( mStandardButton, 3, 2 );

  topLayout->activate();

  QStringList items = list;
  if ( items.remove( "" ) > 0 )
    mChanged = true;
  else
    mChanged = false;
  mEmailListBox->insertStringList( items );

  // set default state
  selectionChanged( -1 );

  KAcceleratorManager::manage( this );
}

EmailEditDialog::~EmailEditDialog()
{
}
    
QStringList EmailEditDialog::emails() const
{
  QStringList emails;
  
  for ( uint i = 0; i < mEmailListBox->count(); ++i )
    emails << mEmailListBox->text( i );

  return emails;
}

void EmailEditDialog::add()
{
  mEmailListBox->insertItem( mEmailEdit->text() );

  mEmailEdit->clear();
  mEmailEdit->setFocus();

  mChanged = true;
}

void EmailEditDialog::edit()
{
  mEmailEdit->setText( mEmailListBox->currentText() );
  mEmailEdit->setFocus();
}

void EmailEditDialog::remove()
{
  QString address = mEmailListBox->currentText();

  QString text = i18n( "<qt>Are you sure that you want to remove the email address <b>%1</b>?</qt>" ).arg( address );
  QString caption = i18n( "Confirm Remove" );
  
  if ( KMessageBox::questionYesNo( this, text, caption ) == KMessageBox::Yes ) {
    mEmailListBox->removeItem( mEmailListBox->currentItem() );
    mChanged = true;
  }
}

bool EmailEditDialog::changed() const
{
  return mChanged;
}

void EmailEditDialog::standard()
{
  QString text = mEmailListBox->currentText();
  mEmailListBox->removeItem( mEmailListBox->currentItem() );
  mEmailListBox->insertItem( text, 0 );
  mEmailListBox->setSelected( 0, true );

  mChanged = true;
}

void EmailEditDialog::selectionChanged( int index )
{
  bool value = ( index >= 0 ); // An item is selected

  mRemoveButton->setEnabled( value );
  mEditButton->setEnabled( value );
  mStandardButton->setEnabled( value );
}

void EmailEditDialog::emailChanged()
{
  mAddButton->setEnabled( !mEmailEdit->text().isEmpty() );
}

#include "emaileditwidget.moc"
