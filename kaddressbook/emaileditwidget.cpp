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

#include "emaileditwidget.h"

#include <QtCore/QString>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

#include <kacceleratormanager.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <KListWidget>
#include <klocale.h>
#include <kmessagebox.h>

class EmailValidator : public QRegExpValidator
{
  public:
    EmailValidator() : QRegExpValidator( 0 )
    {
      setObjectName( "EmailValidator" );
      QRegExp rx( ".*@.*\\.[A-Za-z]+" );
      setRegExp( rx );
    }
};

class EmailItem : public QListWidgetItem
{
  public:
    EmailItem( const QString &text, QListWidget *parent, bool preferred )
      : QListWidgetItem( text, parent ), mPreferred( preferred )
    {
      format();
    }

    void setPreferred( bool preferred ) { mPreferred = preferred; format(); }
    bool preferred() const { return mPreferred; }

  private:
    void format()
    {
      QFont f = font();
      f.setBold( mPreferred );
      setFont( f );
    }

  private:
    bool mPreferred;
};

EmailEditWidget::EmailEditWidget( QWidget *parent, const char *name )
  : QWidget( parent )
{
  setObjectName( name );
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( KDialog::marginHint() );

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
  topLayout->addWidget( mEditButton, 1, 0, 1, 2 );

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
  if ( mEmailList.count() > 0 )
    mEmailList.removeFirst();
  if ( !mEmailEdit->text().isEmpty() ) {
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
      if ( mEmailList.count() > 0 ) {
        mEmailEdit->setText( mEmailList[ 0 ] );
      } else {
        mEmailEdit->setText( QString() );
      }
      emit modified();
    }
  }
}

void EmailEditWidget::textChanged( const QString &text )
{
  if ( mEmailList.count() > 0 )
    mEmailList.removeFirst();

  mEmailList.prepend( text );
}


EmailEditDialog::EmailEditDialog( const QStringList &list, QWidget *parent )
  : KDialog( parent)
{
  setCaption( i18n( "Edit Email Addresses" ) );
  setButtons( KDialog::Ok | KDialog::Cancel );
  setDefaultButton( KDialog::Help );
  QWidget *page = new QWidget( this);
  setMainWidget( page );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );

  mEmailListBox = new KListWidget( page );
  mEmailListBox->setSelectionMode( QAbstractItemView::SingleSelection );

  // Make sure there is room for the scrollbar
  mEmailListBox->setMinimumHeight( mEmailListBox->sizeHint().height() + 30 );
  connect( mEmailListBox, SIGNAL( currentItemChanged( QListWidgetItem *, QListWidgetItem * ) ),
           SLOT( selectionChanged() ) );
  connect( mEmailListBox, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ),
           SLOT( edit() ) );
  topLayout->addWidget( mEmailListBox, 0, 0, 4, 2 );

  mAddButton = new QPushButton( i18n( "Add..." ), page );
  connect( mAddButton, SIGNAL( clicked() ), SLOT( add() ) );
  topLayout->addWidget( mAddButton, 0, 2 );

  mEditButton = new QPushButton( i18n( "Edit..." ), page );
  mEditButton->setEnabled( false );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( edit() ) );
  topLayout->addWidget( mEditButton, 1, 2 );

  mRemoveButton = new QPushButton( i18n( "Remove" ), page );
  mRemoveButton->setEnabled( false );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( remove() ) );
  topLayout->addWidget( mRemoveButton, 2, 2 );

  mStandardButton = new QPushButton( i18n( "Set Standard" ), page );
  mStandardButton->setEnabled( false );
  connect( mStandardButton, SIGNAL( clicked() ), SLOT( standard() ) );
  topLayout->addWidget( mStandardButton, 3, 2 );

  topLayout->activate();

  QStringList items = list;
  if ( items.removeAll( "" ) > 0 )
    mChanged = true;
  else
    mChanged = false;

  QStringList::ConstIterator it;
  bool preferred = true;
  for ( it = items.constBegin(); it != items.constEnd(); ++it ) {
    new EmailItem( *it, mEmailListBox, preferred );
    preferred = false;
  }

  // set default state
  KAcceleratorManager::manage( this );

  setInitialSize( QSize( 400, 200 ) );
}

EmailEditDialog::~EmailEditDialog()
{
}

QStringList EmailEditDialog::emails() const
{
  QStringList emails;

  for ( int i = 0; i < mEmailListBox->count(); ++i ) {
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
                                         QString(), &ok, this, validator );

  if ( !ok )
    return;

  // check if item already available, ignore if so...
  for ( int i = 0; i < mEmailListBox->count(); ++i ) {
    if ( mEmailListBox->item( i )->text() == email )
      return;
  }

  new EmailItem( email, mEmailListBox, (mEmailListBox->count() == 0) );

  mChanged = true;
}

void EmailEditDialog::edit()
{
  EmailValidator *validator = new EmailValidator;
  bool ok = false;

  QListWidgetItem *item = mEmailListBox->currentItem();

  QString email = KInputDialog::getText( i18n( "Edit Email" ), i18n( "Email:" ),
                                         item->text(), &ok, this,
                                         validator );

  if ( !ok )
    return;

  // check if item already available, ignore if so...
  for ( int i = 0; i < mEmailListBox->count(); ++i ) {
    if ( mEmailListBox->item( i )->text() == email )
      return;
  }

  EmailItem *eitem = static_cast<EmailItem*>( item );
  eitem->setText( email );

  mChanged = true;
}

void EmailEditDialog::remove()
{
  QString address = mEmailListBox->currentItem()->text();

  QString text = i18n( "<qt>Are you sure that you want to remove the email address <b>%1</b>?</qt>", address );
  QString caption = i18n( "Confirm Remove" );

  if ( KMessageBox::warningContinueCancel( this, text, caption, KGuiItem( i18n( "&Delete" ), "edit-delete" ) ) == KMessageBox::Continue ) {
    EmailItem *item = static_cast<EmailItem*>( mEmailListBox->currentItem() );

    bool preferred = item->preferred();
    mEmailListBox->takeItem( mEmailListBox->currentRow() );
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
  for ( int i = 0; i < mEmailListBox->count(); ++i ) {
    EmailItem *item = static_cast<EmailItem*>( mEmailListBox->item( i ) );
    if ( i == mEmailListBox->currentRow() )
      item->setPreferred( true );
    else
      item->setPreferred( false );
  }

  mChanged = true;
}

void EmailEditDialog::selectionChanged()
{
  int index = mEmailListBox->currentRow();
  bool value = ( index >= 0 ); // An item is selected

  mRemoveButton->setEnabled( value );
  mEditButton->setEnabled( value );
  mStandardButton->setEnabled( value );
}

#include "emaileditwidget.moc"
