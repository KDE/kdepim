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
#include <qlabel.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qbuttongroup.h>

#include <kbuttonbox.h>
#include <klistview.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <kabc/phonenumber.h>

#include "typecombo.h"

#include "phoneeditwidget.h"

PhoneEditWidget::PhoneEditWidget( QWidget *parent, const char *name )
  : QWidget( parent, name ), mReadOnly(false)
{
  QGridLayout *layout = new QGridLayout( this, 5, 2 );
  layout->setSpacing( KDialog::spacingHint() );

  mPrefCombo = new PhoneTypeCombo( mPhoneList, this );
  mPrefEdit = new KLineEdit( this );
  mPrefEdit->setMinimumWidth( int(mPrefEdit->sizeHint().width() * 1.5) );
  mPrefCombo->setLineEdit( mPrefEdit );
  layout->addWidget( mPrefCombo, 0, 0 );
  layout->addWidget( mPrefEdit, 0, 1 );

  mSecondCombo = new PhoneTypeCombo( mPhoneList, this );
  mSecondEdit = new KLineEdit( this );
  mSecondCombo->setLineEdit( mSecondEdit );
  layout->addWidget( mSecondCombo, 1, 0 );
  layout->addWidget( mSecondEdit, 1, 1 );

  mThirdCombo = new PhoneTypeCombo( mPhoneList, this );
  mThirdEdit = new KLineEdit( this );
  mThirdCombo->setLineEdit( mThirdEdit );
  layout->addWidget( mThirdCombo, 2, 0 );
  layout->addWidget( mThirdEdit, 2, 1 );

  mFourthCombo = new PhoneTypeCombo( mPhoneList, this );
  mFourthEdit = new KLineEdit( this );
  mFourthCombo->setLineEdit( mFourthEdit );
  layout->addWidget( mFourthCombo, 3, 0 );
  layout->addWidget( mFourthEdit, 3, 1 );

  // Four numbers don't fit in the current dialog
  mFourthCombo->hide();
  mFourthEdit->hide();

  mEditButton = new QPushButton( i18n( "Edit Phone Numbers..." ), this );
  layout->addMultiCellWidget( mEditButton, 4, 4, 0, 1 );

  connect( mPrefEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotPrefEditChanged() ) );
  connect( mSecondEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotSecondEditChanged() ) );
  connect( mThirdEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotThirdEditChanged() ) );
  connect( mFourthEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotFourthEditChanged() ) );

  connect( mEditButton, SIGNAL( clicked() ), SLOT( edit() ) );

  connect( mPrefCombo, SIGNAL( activated( int ) ),
           SLOT( updatePrefEdit() ) );
  connect( mSecondCombo, SIGNAL( activated( int ) ),
           SLOT( updateSecondEdit() ) );
  connect( mThirdCombo, SIGNAL( activated( int ) ),
           SLOT( updateThirdEdit() ) );
  connect( mFourthCombo, SIGNAL( activated( int ) ),
           SLOT( updateFourthEdit() ) );
}

PhoneEditWidget::~PhoneEditWidget()
{
}

void PhoneEditWidget::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;

  mPrefEdit->setReadOnly( mReadOnly );
  mSecondEdit->setReadOnly( mReadOnly );
  mThirdEdit->setReadOnly( mReadOnly );
  mFourthEdit->setReadOnly( mReadOnly );
  mEditButton->setEnabled( !mReadOnly );
}

void PhoneEditWidget::setPhoneNumbers( const KABC::PhoneNumber::List &list )
{
  mPhoneList.clear();

  // Insert types for existing numbers.
  mPrefCombo->insertTypeList( list );

  QValueList<int> defaultTypes;
  defaultTypes << KABC::PhoneNumber::Home;
  defaultTypes << KABC::PhoneNumber::Work;
  defaultTypes << KABC::PhoneNumber::Cell;
  defaultTypes << ( KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
  defaultTypes << ( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );

  // Insert default types.
  // Doing this for mPrefCombo is enough because the list is shared by all
  // combos.
  QValueList<int>::ConstIterator it;
  for( it = defaultTypes.begin(); it != defaultTypes.end(); ++it ) {
    if ( !mPrefCombo->hasType( *it ) )
      mPrefCombo->insertType( list, *it, PhoneNumber( "", *it ) );
  }

  updateCombos();

  mPrefCombo->selectType( defaultTypes[ 0 ] );
  mSecondCombo->selectType( defaultTypes[ 1 ] );
  mThirdCombo->selectType( defaultTypes[ 2 ] );
  mFourthCombo->selectType( defaultTypes[ 3 ] );

  updateLineEdits();
}

void PhoneEditWidget::updateLineEdits()
{
  updatePrefEdit();
  updateSecondEdit();
  updateThirdEdit();
  updateFourthEdit();
}

void PhoneEditWidget::updateCombos()
{
  mPrefCombo->updateTypes();
  mSecondCombo->updateTypes();
  mThirdCombo->updateTypes();
  mFourthCombo->updateTypes();
}

KABC::PhoneNumber::List PhoneEditWidget::phoneNumbers()
{
  KABC::PhoneNumber::List retList;

  KABC::PhoneNumber::List::Iterator it;
  for ( it = mPhoneList.begin(); it != mPhoneList.end(); ++it )
    if ( !(*it).number().isEmpty() )
      retList.append( *it );

  return retList;
}

void PhoneEditWidget::edit()
{
  PhoneEditDialog dlg( mPhoneList, this );

  if ( dlg.exec() ) {
    if ( dlg.changed() ) {
      mPhoneList = dlg.phoneNumbers();
      updateCombos();
      emit modified();
    }
  }
}

void PhoneEditWidget::updatePrefEdit()
{
  updateEdit( mPrefCombo );
}

void PhoneEditWidget::updateSecondEdit()
{
  updateEdit( mSecondCombo );
}

void PhoneEditWidget::updateThirdEdit()
{
  updateEdit( mThirdCombo );
}

void PhoneEditWidget::updateFourthEdit()
{
  updateEdit( mFourthCombo );
}

void PhoneEditWidget::updateEdit( PhoneTypeCombo *combo )
{
  QLineEdit *edit = combo->lineEdit();
  if ( !edit )
    return;

#if 0
  if ( edit == mPrefEdit ) kdDebug(5720) << " prefEdit" << endl;
  if ( edit == mSecondEdit ) kdDebug(5720) << " secondEdit" << endl;
  if ( edit == mThirdEdit ) kdDebug(5720) << " thirdEdit" << endl;
  if ( edit == mFourthEdit ) kdDebug(5720) << " fourthEdit" << endl;
#endif

  PhoneNumber::List::Iterator it = combo->selectedElement();
  if ( it != mPhoneList.end() ) {
    int pos = edit->cursorPosition();
    edit->setText( (*it).number() );
    edit->setCursorPosition( pos );
  } else {
    kdDebug(5720) << "PhoneEditWidget::updateEdit(): no selected element" << endl;
  }
}

void PhoneEditWidget::slotPrefEditChanged()
{
  updatePhoneNumber( mPrefCombo );
}

void PhoneEditWidget::slotSecondEditChanged()
{
  updatePhoneNumber( mSecondCombo );
}

void PhoneEditWidget::slotThirdEditChanged()
{
  updatePhoneNumber( mThirdCombo );
}

void PhoneEditWidget::slotFourthEditChanged()
{
  updatePhoneNumber( mFourthCombo );
}

void PhoneEditWidget::updatePhoneNumber( PhoneTypeCombo *combo )
{
  QLineEdit *edit = combo->lineEdit();
  if ( !edit ) return;

  PhoneNumber::List::Iterator it = combo->selectedElement();
  if ( it != mPhoneList.end() ) {
    (*it).setNumber( edit->text() );
  } else {
    kdDebug(5720) << "PhoneEditWidget::updatePhoneNumber(): no selected element"
              << endl;
  }

  updateOtherEdit( combo, mPrefCombo );
  updateOtherEdit( combo, mSecondCombo );
  updateOtherEdit( combo, mThirdCombo );
  updateOtherEdit( combo, mFourthCombo );

  if ( !mReadOnly )
    emit modified();
}

void PhoneEditWidget::updateOtherEdit( PhoneTypeCombo *combo, PhoneTypeCombo *otherCombo )
{
  if ( combo == otherCombo ) return;

  if ( combo->currentItem() == otherCombo->currentItem() ) {
    updateEdit( otherCombo );
  }
}

///////////////////////////////////////////
// PhoneEditDialog

class PhoneViewItem : public QListViewItem
{
public:
  PhoneViewItem( QListView *parent, const KABC::PhoneNumber &number );

  void setPhoneNumber( const KABC::PhoneNumber &number )
  {
    mPhoneNumber = number;
    makeText();
  }

  QString key() { return mPhoneNumber.id(); }
  QString country() { return ""; }
  QString region() { return ""; }
  QString number() { return ""; }

  KABC::PhoneNumber phoneNumber() { return mPhoneNumber; }

private:
  void makeText();

  KABC::PhoneNumber mPhoneNumber;
};

PhoneViewItem::PhoneViewItem( QListView *parent, const KABC::PhoneNumber &number )
  : QListViewItem( parent ), mPhoneNumber( number )
{
  makeText();
}

void PhoneViewItem::makeText()
{
  /**
   * Will be used in future versions of kaddressbook/libkabc

    setText( 0, mPhoneNumber.country() );
    setText( 1, mPhoneNumber.region() );
    setText( 2, mPhoneNumber.number() );
    setText( 3, mPhoneNumber.typeLabel() );
   */

  setText( 0, mPhoneNumber.number() );
  setText( 1, mPhoneNumber.typeLabel() );
}

PhoneEditDialog::PhoneEditDialog( const KABC::PhoneNumber::List &list, QWidget *parent, const char *name )
  : KDialogBase( KDialogBase::Plain, i18n( "Edit Phone Numbers" ),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, true)
{
  mPhoneNumberList = list;

  QWidget *page = plainPage();

  QGridLayout *layout = new QGridLayout( page, 1, 2 );
  layout->setSpacing( spacingHint() );

  mListView = new KListView( page );
  mListView->addColumn( i18n( "Number" ) );
  mListView->addColumn( i18n( "Type" ) );
  mListView->setAllColumnsShowFocus( true );
  mListView->setFullWidth( true );

  KButtonBox *buttonBox = new KButtonBox( page, Vertical );

  buttonBox->addButton( i18n( "&Add..." ), this, SLOT( slotAddPhoneNumber() ) );
  mEditButton = buttonBox->addButton( i18n( "&Edit..." ), this, SLOT( slotEditPhoneNumber() ) );
  mEditButton->setEnabled( false );
  mRemoveButton = buttonBox->addButton( i18n( "&Remove" ), this, SLOT( slotRemovePhoneNumber() ) );
  mRemoveButton->setEnabled( false );
  buttonBox->layout();

  layout->addWidget( mListView, 0, 0 );
  layout->addWidget( buttonBox, 0, 1 );

  connect( mListView, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()) );
  connect( mListView, SIGNAL(doubleClicked( QListViewItem *, const QPoint &, int  )), this, SLOT( slotEditPhoneNumber()));

  KABC::PhoneNumber::List::Iterator it;
  for ( it = mPhoneNumberList.begin(); it != mPhoneNumberList.end(); ++it )
    new PhoneViewItem( mListView, *it );

  mChanged = false;
}

PhoneEditDialog::~PhoneEditDialog()
{
}

void PhoneEditDialog::slotAddPhoneNumber()
{
  KABC::PhoneNumber tmp( "", 0 );
  PhoneTypeDialog dlg( tmp, this );

  if ( dlg.exec() ) {
    KABC::PhoneNumber phoneNumber = dlg.phoneNumber();
    mPhoneNumberList.append( phoneNumber );
    new PhoneViewItem( mListView, phoneNumber );

    mChanged = true;
  }
}

void PhoneEditDialog::slotRemovePhoneNumber()
{
  PhoneViewItem *item = static_cast<PhoneViewItem*>( mListView->currentItem() );
  if ( !item )
    return;

  mPhoneNumberList.remove( item->phoneNumber() );
  QListViewItem *currItem = mListView->currentItem();
  mListView->takeItem( currItem );
  delete currItem;

  mChanged = true;
}

void PhoneEditDialog::slotEditPhoneNumber()
{
  PhoneViewItem *item = static_cast<PhoneViewItem*>( mListView->currentItem() );
  if ( !item )
    return;

  PhoneTypeDialog dlg( item->phoneNumber(), this );

  if ( dlg.exec() ) {
    slotRemovePhoneNumber();
    KABC::PhoneNumber phoneNumber = dlg.phoneNumber();
    mPhoneNumberList.append( phoneNumber );
    new PhoneViewItem( mListView, phoneNumber );

    mChanged = true;
  }
}

void PhoneEditDialog::slotSelectionChanged()
{
  bool state = ( mListView->currentItem() != 0 );

  mRemoveButton->setEnabled( state );
  mEditButton->setEnabled( state );
}

const KABC::PhoneNumber::List &PhoneEditDialog::phoneNumbers()
{
  return mPhoneNumberList;
}

bool PhoneEditDialog::changed() const
{
  return mChanged;
}

///////////////////////////////////////////
// PhoneTypeDialog
PhoneTypeDialog::PhoneTypeDialog( const KABC::PhoneNumber &phoneNumber,
                               QWidget *parent, const char *name)
  : KDialogBase( KDialogBase::Plain, i18n( "Edit Phone Number" ),
                KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                parent, name, true), mPhoneNumber( phoneNumber )
{
  QWidget *page = plainPage();
  QLabel *label = 0;
  QGridLayout *layout = new QGridLayout( page, 3, 2, marginHint(), spacingHint() );

  label = new QLabel( i18n( "Number:" ), page );
  layout->addWidget( label, 0, 0 );
  mNumber = new KLineEdit( page );
  layout->addWidget( mNumber, 0, 1 );

  mPreferredBox = new QCheckBox( i18n( "This is the preferred phone number" ), page );
  layout->addMultiCellWidget( mPreferredBox, 1, 1, 0, 1 );

  mGroup = new QButtonGroup( 2, Horizontal, i18n( "Types" ), page );
  layout->addMultiCellWidget( mGroup, 2, 2, 0, 1 );

  // fill widgets
  mNumber->setText( mPhoneNumber.number() );

  mTypeList = KABC::PhoneNumber::typeList();
  mTypeList.remove( KABC::PhoneNumber::Pref );

  KABC::PhoneNumber::TypeList::Iterator it;
  for ( it = mTypeList.begin(); it != mTypeList.end(); ++it )
    new QCheckBox( KABC::PhoneNumber::typeLabel( *it ), mGroup );

  for ( int i = 0; i < mGroup->count(); ++i ) {
    int type = mPhoneNumber.type();
    QCheckBox *box = (QCheckBox*)mGroup->find( i );
    box->setChecked( type & mTypeList[ i ] );
  }

  mPreferredBox->setChecked( mPhoneNumber.type() & KABC::PhoneNumber::Pref );
}

KABC::PhoneNumber PhoneTypeDialog::phoneNumber()
{
  mPhoneNumber.setNumber( mNumber->text() );

  int type = 0;
  for ( int i = 0; i < mGroup->count(); ++i ) {
    QCheckBox *box = (QCheckBox*)mGroup->find( i );
    if ( box->isChecked() )
      type += mTypeList[ i ];
  }

  if ( mPreferredBox->isChecked() )
    mPhoneNumber.setType( type | KABC::PhoneNumber::Pref );
  else
    mPhoneNumber.setType( type & ~KABC::PhoneNumber::Pref );

  return mPhoneNumber;
}


#include "phoneeditwidget.moc"
