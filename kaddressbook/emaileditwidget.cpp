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
#include <qpainter.h>
#include <qpushbutton.h>
#include <qvalidator.h>
#include <qstring.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

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
      : QRegExpValidator( 0, "EmailValidator" )
    {
      QRegExp rx( "[A-Za-z\\-\\.]+@[A-Za-z\\-\\.]+\\.[A-Za-z]+" );
      setRegExp( rx );
    }
};

class EmailItem : public QListBoxText
{
  public:
    EmailItem( QListBox *parent, const QString &text, bool preferred )
      : QListBoxText( parent, text ), mPreferred( preferred )
    {}

    void setPreferred( bool preferred ) { mPreferred = preferred; }
    bool preferred() const { return mPreferred; }

    void setText( const QString &text )
    {
      QListBoxText::setText( text );
    }

  protected:
    virtual void paint( QPainter *p )
    {
      if ( mPreferred ) {
        QFont font = p->font();
        font.setBold( true );
        p->setFont( font );
      }

      QListBoxText::paint( p );
    }

  private:
    bool mPreferred;
};

EmailEditWidget::EmailEditWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *topLayout = new QGridLayout( this, 2, 2, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Email:" ), this );
  topLayout->addWidget( label, 0, 0 );

  mEmailEdit = new KLineEdit( this );
  mEmailEdit->setValidator( new EmailValidator );
  connect( mEmailEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  connect( mEmailEdit, SIGNAL( textChanged( const QString& ) ),
           SIGNAL( modified() ) );
  label->setBuddy( mEmailEdit );
  topLayout->addWidget( mEmailEdit, 0, 1 );

  mEditButton = new QPushButton( i18n( "Edit Email Addresses..." ), this);
  connect( mEditButton, SIGNAL( clicked() ), SLOT( edit() ) );
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
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Help,
                 parent, name, true )
{
  QWidget *page = plainPage();

  QGridLayout *topLayout = new QGridLayout( page, 4, 3, marginHint(),
                                            spacingHint() );

  mEmailListBox = new QListBox( page );

  // Make sure there is room for the scrollbar
  mEmailListBox->setMinimumHeight( mEmailListBox->sizeHint().height() + 30 );
  connect( mEmailListBox, SIGNAL( highlighted( int ) ),
           SLOT( selectionChanged( int ) ) );
  connect( mEmailListBox, SIGNAL( selected( int ) ),
           SLOT( edit() ) );
  topLayout->addMultiCellWidget( mEmailListBox, 0, 3, 0, 1 );

  mAddButton = new QPushButton( i18n( "Add..." ), page );
  connect( mAddButton, SIGNAL( clicked() ), SLOT( add() ) );
  topLayout->addWidget( mAddButton, 0, 2 );

  mEditButton = new QPushButton( i18n( "Edit..." ), page );
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

  QStringList::Iterator it;
  bool preferred = true;
  for ( it = items.begin(); it != items.end(); ++it ) {
    new EmailItem( mEmailListBox, *it, preferred );
    preferred = false;
  }

  // set default state
  selectionChanged( -1 );
  KAcceleratorManager::manage( this );

  setInitialSize( QSize( 400, 200 ) );
}

EmailEditDialog::~EmailEditDialog()
{
}

QStringList EmailEditDialog::emails() const
{
  QStringList emails;

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

  QString email = KInputDialog::getText( i18n( "Add Email" ), i18n( "New Email:" ),
                                         QString::null, &ok, this, "EmailEditDialog",
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

  QString email = KInputDialog::getText( i18n( "Edit Email" ), i18n( "Email:" ),
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
  QString address = mEmailListBox->currentText();

  QString text = i18n( "<qt>Are you sure that you want to remove the email address <b>%1</b>?</qt>" ).arg( address );
  QString caption = i18n( "Confirm Remove" );

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
