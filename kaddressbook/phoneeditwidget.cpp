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

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qsignalmapper.h>
#include <qstring.h>
#include <qtooltip.h>

#include <kapplication.h>
#include <kbuttonbox.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klistview.h>
#include <klocale.h>

#include <kabc/phonenumber.h>

#include "phoneeditwidget.h"

PhoneTypeCombo::PhoneTypeCombo( QWidget *parent )
  : KComboBox( parent, "TypeCombo" ),
    mType( KABC::PhoneNumber::Home ),
    mLastSelected( 0 ),
    mTypeList( KABC::PhoneNumber::typeList() )
{
  mTypeList.append( -1 ); // Others...

  update();

  connect( this, SIGNAL( activated( int ) ),
           this, SLOT( selected( int ) ) );
  connect( this, SIGNAL( activated( int ) ),
           this, SIGNAL( modified() ) );
}

PhoneTypeCombo::~PhoneTypeCombo()
{
}

void PhoneTypeCombo::setType( int type )
{
  if ( !mTypeList.contains( type ) )
    mTypeList.insert( mTypeList.at( mTypeList.count() - 1 ), type );

  mType = type;
  update();
}

int PhoneTypeCombo::type() const
{
  return mType;
}

void PhoneTypeCombo::update()
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  clear();
  QValueList<int>::ConstIterator it;
  for ( it = mTypeList.begin(); it != mTypeList.end(); ++it ) {
    if ( *it == -1 ) { // "Other..." entry
      insertItem( i18n( "Other..." ) );
    } else {
      insertItem( KABC::PhoneNumber::typeLabel( *it ) );
    }
  }

  setCurrentItem( mTypeList.findIndex( mType ) );

  blockSignals( blocked );
}

void PhoneTypeCombo::selected( int pos )
{
  if ( mTypeList[ pos ] == -1 )
    otherSelected();
  else {
    mType = mTypeList[ pos ];
    mLastSelected = pos;
  }
}

void PhoneTypeCombo::otherSelected()
{
  PhoneTypeDialog dlg( mType, this );
  if ( dlg.exec() ) {
    mType = dlg.type();
    if ( !mTypeList.contains( mType ) )
      mTypeList.insert( mTypeList.at( mTypeList.count() - 1 ), mType );
  } else {
    setType( mTypeList[ mLastSelected ] );
  }

  update();
}

PhoneNumberWidget::PhoneNumberWidget( QWidget *parent )
  : QWidget( parent )
{
  QHBoxLayout *layout = new QHBoxLayout( this, 6, 11 );

  mTypeCombo = new PhoneTypeCombo( this );
  mNumberEdit = new KLineEdit( this );

  layout->addWidget( mTypeCombo );
  layout->addWidget( mNumberEdit );

  connect( mTypeCombo, SIGNAL( modified() ), SIGNAL( modified() ) );
  connect( mNumberEdit, SIGNAL( textChanged( const QString& ) ), SIGNAL( modified() ) );
}

void PhoneNumberWidget::setNumber( const KABC::PhoneNumber &number )
{
  mNumber = number;

  mTypeCombo->setType( number.type() );
  mNumberEdit->setText( number.number() );
}

KABC::PhoneNumber PhoneNumberWidget::number() const
{
  KABC::PhoneNumber number( mNumber );

  number.setType( mTypeCombo->type() );
  number.setNumber( mNumberEdit->text() );

  return number;
}

void PhoneNumberWidget::setReadOnly( bool readOnly )
{
  mTypeCombo->setEnabled( !readOnly );
  mNumberEdit->setReadOnly( readOnly );
}


PhoneEditWidget::PhoneEditWidget( QWidget *parent, const char *name )
  : QWidget( parent, name ), mReadOnly( false )
{
  QGridLayout *layout = new QGridLayout( this, 2, 2 );
  layout->setSpacing( KDialog::spacingHint() );

  mWidgetLayout = new QVBoxLayout( layout );
  layout->addMultiCellLayout( mWidgetLayout, 0, 0, 0, 1 );

  mAddButton = new QPushButton( i18n( "Add" ), this );
  mAddButton->setMaximumSize( mAddButton->sizeHint() );
  layout->addWidget( mAddButton, 1, 0 );

  mRemoveButton = new QPushButton( i18n( "Remove" ), this );
  mRemoveButton->setMaximumSize( mRemoveButton->sizeHint() );
  layout->addWidget( mRemoveButton, 1, 1 );

  mMapper = new QSignalMapper( this );
  connect( mMapper, SIGNAL( mapped( int ) ), SLOT( changed( int ) ) );

  connect( mAddButton, SIGNAL( clicked() ), SLOT( add() ) );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( remove() ) );
}

PhoneEditWidget::~PhoneEditWidget()
{
}

void PhoneEditWidget::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;

  QPtrListIterator<PhoneNumberWidget> it( mWidgets );
  while ( it.current() ) {
    it.current()->setReadOnly( readOnly );
    ++it;
  }
}

void PhoneEditWidget::setPhoneNumbers( const KABC::PhoneNumber::List &list )
{
  mPhoneNumberList = list;

  KABC::PhoneNumber::TypeList types;
  types << KABC::PhoneNumber::Home;
  types << KABC::PhoneNumber::Work;
  types << KABC::PhoneNumber::Cell;

  // add an empty entry per default
  if ( mPhoneNumberList.count() < 3 )
    for ( int i = mPhoneNumberList.count(); i < 3; ++i )
      mPhoneNumberList.append( KABC::PhoneNumber( "", types[ i ] ) );

  updateWidgets();
}

KABC::PhoneNumber::List PhoneEditWidget::phoneNumbers() const
{
  KABC::PhoneNumber::List list;

  KABC::PhoneNumber::List::ConstIterator it;
  for ( it = mPhoneNumberList.begin(); it != mPhoneNumberList.end(); ++it )
    if ( !(*it).number().isEmpty() )
      list.append( *it );

  return list;
}

void PhoneEditWidget::changed()
{
  if ( !mReadOnly )
    emit modified();
}

void PhoneEditWidget::add()
{
  mPhoneNumberList.append( KABC::PhoneNumber() );

  updateWidgets();
}

void PhoneEditWidget::remove()
{
  mPhoneNumberList.remove( mPhoneNumberList.last() );
  changed();

  updateWidgets();
}

void PhoneEditWidget::updateWidgets()
{
  mWidgets.setAutoDelete( true );
  mWidgets.clear();
  mWidgets.setAutoDelete( false );

  KABC::PhoneNumber::List::ConstIterator it;
  int counter = 0;
  for ( it = mPhoneNumberList.begin(); it != mPhoneNumberList.end(); ++it ) {
    PhoneNumberWidget *wdg = new PhoneNumberWidget( this );
    wdg->setNumber( *it );

    mMapper->setMapping( wdg, counter );
    connect( wdg, SIGNAL( modified() ), mMapper, SLOT( map() ) );

    mWidgetLayout->addWidget( wdg );
    mWidgets.append( wdg );
    wdg->show();

    ++counter;
  }
}

void PhoneEditWidget::changed( int pos )
{
  mPhoneNumberList[ pos ] = mWidgets.at( pos )->number();
  changed();
}

///////////////////////////////////////////
// PhoneTypeDialog
PhoneTypeDialog::PhoneTypeDialog( int type, QWidget *parent )
  : KDialogBase( Plain, i18n( "Edit Phone Number" ), Ok | Cancel, Ok,
                 parent, "PhoneTypeDialog", true ),
    mType( type )
{
  QWidget *page = plainPage();

  QVBoxLayout *layout = new QVBoxLayout( page, spacingHint() );

  mPreferredBox = new QCheckBox( i18n( "This is the preferred phone number" ), page );
  layout->addWidget( mPreferredBox );

  mGroup = new QButtonGroup( 2, Horizontal, i18n( "Types" ), page );
  layout->addWidget( mGroup );

  // fill widgets
  mTypeList = KABC::PhoneNumber::typeList();
  mTypeList.remove( KABC::PhoneNumber::Pref );

  KABC::PhoneNumber::TypeList::ConstIterator it;
  for ( it = mTypeList.begin(); it != mTypeList.end(); ++it )
    new QCheckBox( KABC::PhoneNumber::typeLabel( *it ), mGroup );

  for ( int i = 0; i < mGroup->count(); ++i ) {
    QCheckBox *box = (QCheckBox*)mGroup->find( i );
    box->setChecked( mType & mTypeList[ i ] );
  }

  mPreferredBox->setChecked( mType & KABC::PhoneNumber::Pref );
}

int PhoneTypeDialog::type() const
{
  int type = 0;

  for ( int i = 0; i < mGroup->count(); ++i ) {
    QCheckBox *box = (QCheckBox*)mGroup->find( i );
    if ( box->isChecked() )
      type += mTypeList[ i ];
  }

  if ( mPreferredBox->isChecked() )
    type = type | KABC::PhoneNumber::Pref;
  else
    type = type & ~KABC::PhoneNumber::Pref;

  return type;
}


#include "phoneeditwidget.moc"
