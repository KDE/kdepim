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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpainter.h>
#include <tqpushbutton.h>
#include <tqvalidator.h>
#include <tqstring.h>
#include <tqtoolbutton.h>
#include <tqtooltip.h>

#include <kaccelmanager.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "emaileditwidget.h"

class EmailValidator : public QRegExpValidator
{
  public:
    EmailValidator()
      : TQRegExpValidator( 0, "EmailValidator" )
    {
      TQRegExp rx( ".*@.*\\.[A-Za-z]+" );
      setRegExp( rx );
    }
};

class EmailItem : public QListBoxText
{
  public:
    EmailItem( TQListBox *parent, const TQString &text, bool preferred )
      : TQListBoxText( parent, text ), mPreferred( preferred )
    {}

    void setPreferred( bool preferred ) { mPreferred = preferred; }
    bool preferred() const { return mPreferred; }

    void setText( const TQString &text )
    {
      TQListBoxText::setText( text );
    }

  protected:
    virtual void paint( TQPainter *p )
    {
      if ( mPreferred ) {
        TQFont font = p->font();
        font.setBold( true );
        p->setFont( font );
      }

      TQListBoxText::paint( p );
    }

  private:
    bool mPreferred;
};

EmailEditWidget::EmailEditWidget( TQWidget *parent, const char *name )
  : TQWidget( parent, name )
{
  TQGridLayout *topLayout = new TQGridLayout( this, 2, 2, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  TQLabel *label = new TQLabel( i18n( "Email:" ), this );
  topLayout->addWidget( label, 0, 0 );

  mEmailEdit = new KLineEdit( this );
  mEmailEdit->setValidator( new EmailValidator );
  connect( mEmailEdit, TQT_SIGNAL( textChanged( const TQString& ) ),
           TQT_SLOT( textChanged( const TQString& ) ) );
  connect( mEmailEdit, TQT_SIGNAL( textChanged( const TQString& ) ),
           TQT_SIGNAL( modified() ) );
  label->setBuddy( mEmailEdit );
  topLayout->addWidget( mEmailEdit, 0, 1 );

  mEditButton = new TQPushButton( i18n( "Edit Email Addresses..." ), this);
  connect( mEditButton, TQT_SIGNAL( clicked() ), TQT_SLOT( edit() ) );
  topLayout->addMultiCellWidget( mEditButton, 1, 1, 0, 1 );

  topLayout->activate();
}

EmailEditWidget::~EmailEditWidget()
{
}

void EmailEditWidget::setReadOnly( bool readOnly )
{
  mEmailEdit->setReadOnly( readOnly );
  mEditButton->setEnabled( !readOnly );
}

void EmailEditWidget::setEmails( const TQStringList &list )
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

TQStringList EmailEditWidget::emails()
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

void EmailEditWidget::textChanged( const TQString &text )
{
  if ( mEmailList.count() > 0 )
    mEmailList.remove( mEmailList.begin() );

  mEmailList.prepend( text );
}


EmailEditDialog::EmailEditDialog( const TQStringList &list, TQWidget *parent,
                                  const char *name )
  : KDialogBase( KDialogBase::Plain, i18n( "Edit Email Addresses" ),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Help,
                 parent, name, true )
{
  TQWidget *page = plainPage();

  TQGridLayout *topLayout = new TQGridLayout( page, 4, 3, 0, spacingHint() );

  mEmailListBox = new TQListBox( page );

  // Make sure there is room for the scrollbar
  mEmailListBox->setMinimumHeight( mEmailListBox->sizeHint().height() + 30 );
  connect( mEmailListBox, TQT_SIGNAL( highlighted( int ) ),
           TQT_SLOT( selectionChanged( int ) ) );
  connect( mEmailListBox, TQT_SIGNAL( selected( int ) ),
           TQT_SLOT( edit() ) );
  topLayout->addMultiCellWidget( mEmailListBox, 0, 3, 0, 1 );

  mAddButton = new TQPushButton( i18n( "Add..." ), page );
  connect( mAddButton, TQT_SIGNAL( clicked() ), TQT_SLOT( add() ) );
  topLayout->addWidget( mAddButton, 0, 2 );

  mEditButton = new TQPushButton( i18n( "Edit..." ), page );
  connect( mEditButton, TQT_SIGNAL( clicked() ), TQT_SLOT( edit() ) );
  topLayout->addWidget( mEditButton, 1, 2 );

  mRemoveButton = new TQPushButton( i18n( "Remove" ), page );
  connect( mRemoveButton, TQT_SIGNAL( clicked() ), TQT_SLOT( remove() ) );
  topLayout->addWidget( mRemoveButton, 2, 2 );

  mStandardButton = new TQPushButton( i18n( "Set Standard" ), page );
  connect( mStandardButton, TQT_SIGNAL( clicked() ), TQT_SLOT( standard() ) );
  topLayout->addWidget( mStandardButton, 3, 2 );

  topLayout->activate();

  TQStringList items = list;
  if ( items.remove( "" ) > 0 )
    mChanged = true;
  else
    mChanged = false;

  TQStringList::ConstIterator it;
  bool preferred = true;
  for ( it = items.begin(); it != items.end(); ++it ) {
    new EmailItem( mEmailListBox, *it, preferred );
    preferred = false;
  }

  // set default state
  selectionChanged( -1 );
  KAcceleratorManager::manage( this );

  setInitialSize( TQSize( 400, 200 ) );
}

EmailEditDialog::~EmailEditDialog()
{
}

TQStringList EmailEditDialog::emails() const
{
  TQStringList emails;

  for ( uint i = 0; i < mEmailListBox->count(); ++i ) {
    EmailItem *item = static_cast<EmailItem*>( mEmailListBox->item( i ) );
    if ( item->preferred() )
      emails.prepend( item->text() );
    else
      emails.append( item->text() );
  }

  return emails;
}

void EmailEditDialog::add()
{
  EmailValidator *validator = new EmailValidator;
  bool ok = false;

  TQString email = KInputDialog::getText( i18n( "Add Email" ), i18n( "New Email:" ),
                                         TQString::null, &ok, this, "EmailEditDialog",
                                         validator );

  if ( !ok )
    return;

  // check if item already available, ignore if so...
  for ( uint i = 0; i < mEmailListBox->count(); ++i ) {
    if ( mEmailListBox->text( i ) == email )
      return;
  }

  new EmailItem( mEmailListBox, email, (mEmailListBox->count() == 0) );

  mChanged = true;
}

void EmailEditDialog::edit()
{
  EmailValidator *validator = new EmailValidator;
  bool ok = false;

  int editPos = mEmailListBox->currentItem();

  TQString email = KInputDialog::getText( i18n( "Edit Email" ), i18n( "Email:" ),
                                         mEmailListBox->text( editPos ), &ok, this,
                                         "EmailEditDialog", validator );

  if ( !ok )
    return;

  // check if item already available, ignore if so...
  for ( uint i = 0; i < mEmailListBox->count(); ++i ) {
    if ( mEmailListBox->text( i ) == email )
      return;
  }

  EmailItem *item = static_cast<EmailItem*>( mEmailListBox->item( editPos ) );
  item->setText( email );
  mEmailListBox->triggerUpdate( true );

  mChanged = true;
}

void EmailEditDialog::remove()
{
  TQString address = mEmailListBox->currentText();

  TQString text = i18n( "<qt>Are you sure that you want to remove the email address <b>%1</b>?</qt>" ).arg( address );
  TQString caption = i18n( "Confirm Remove" );

  if ( KMessageBox::warningContinueCancel( this, text, caption, KGuiItem( i18n("&Delete"), "editdelete") ) == KMessageBox::Continue) {
    EmailItem *item = static_cast<EmailItem*>( mEmailListBox->item( mEmailListBox->currentItem() ) );

    bool preferred = item->preferred();
    mEmailListBox->removeItem( mEmailListBox->currentItem() );
    if ( preferred ) {
      item = dynamic_cast<EmailItem*>( mEmailListBox->item( 0 ) );
      if ( item )
        item->setPreferred( true );
    }

    mChanged = true;
  }
}

bool EmailEditDialog::changed() const
{
  return mChanged;
}

void EmailEditDialog::standard()
{
  for ( uint i = 0; i < mEmailListBox->count(); ++i ) {
    EmailItem *item = static_cast<EmailItem*>( mEmailListBox->item( i ) );
    if ( (int)i == mEmailListBox->currentItem() )
      item->setPreferred( true );
    else
      item->setPreferred( false );
  }

  mEmailListBox->triggerUpdate( true );

  mChanged = true;
}

void EmailEditDialog::selectionChanged( int index )
{
  bool value = ( index >= 0 ); // An item is selected

  mRemoveButton->setEnabled( value );
  mEditButton->setEnabled( value );
  mStandardButton->setEnabled( value );
}

#include "emaileditwidget.moc"
