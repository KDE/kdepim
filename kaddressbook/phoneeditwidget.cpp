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

#include "phoneeditwidget.h"

class PhoneItem : public QListBoxText
{
public:
  PhoneItem( QListBox *parent, const KABC::PhoneNumber::List &list, const KABC::PhoneNumber &number )
    : QListBoxText( parent, QString::null ), mNumber( number )
  {
    KABC::PhoneNumber::List::ConstIterator it;
    int occurances = 0;
    for ( it = list.begin(); it != list.end(); ++it ) {
      if ( (*it).id() == mNumber.id() ) {
        QString text = mNumber.typeLabel();
        if ( occurances > 0 )
          text += " " + QString::number( occurances + 1 );
        setText( text );
        break;
      }

      if ( ( (*it).type() & ~KABC::PhoneNumber::Pref ) == ( mNumber.type() & ~KABC::PhoneNumber::Pref ) )
        occurances++;
    }
  }

  KABC::PhoneNumber phoneNumber() const { return mNumber; }


private:
  KABC::PhoneNumber mNumber;
};

PhoneEditWidget::PhoneEditWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *layout = new QGridLayout( this, 5, 2 );
  
  mPrefCombo = new KComboBox( this );
  mPrefEdit = new KLineEdit( this );
  layout->addWidget( mPrefCombo, 0, 0 );
  layout->addWidget( mPrefEdit, 0, 1 );
  mEditMap.insert( mPrefEdit, mPrefCombo );

  mSecondCombo = new KComboBox( this );
  mSecondEdit = new KLineEdit( this );
  layout->addWidget( mSecondCombo, 1, 0 );
  layout->addWidget( mSecondEdit, 1, 1 );
  mEditMap.insert( mSecondEdit, mSecondCombo );

  mThirdCombo = new KComboBox( this );
  mThirdEdit = new KLineEdit( this );
  layout->addWidget( mThirdCombo, 2, 0 );
  layout->addWidget( mThirdEdit, 2, 1 );
  mEditMap.insert( mThirdEdit, mThirdCombo );

  mFourthCombo = new KComboBox( this );
  mFourthEdit = new KLineEdit( this );
  layout->addWidget( mFourthCombo, 3, 0 );
  layout->addWidget( mFourthEdit, 3, 1 );
  mEditMap.insert( mFourthEdit, mFourthCombo );

  QPushButton *editButton = new QPushButton( i18n( "Edit Phone Numbers..." ), this );
  layout->addMultiCellWidget( editButton, 4, 4, 0, 1 );

  connect( mPrefEdit, SIGNAL( textChanged( const QString& ) ), SLOT( numberChanged( const QString& ) ) );
  connect( mSecondEdit, SIGNAL( textChanged( const QString& ) ), SLOT( numberChanged( const QString& ) ) );
  connect( mThirdEdit, SIGNAL( textChanged( const QString& ) ), SLOT( numberChanged( const QString& ) ) );
  connect( mFourthEdit, SIGNAL( textChanged( const QString& ) ), SLOT( numberChanged( const QString& ) ) );
  connect( editButton, SIGNAL( clicked() ), SLOT( edit() ) );

  connect( mPrefCombo, SIGNAL( highlighted( int ) ), SLOT( comboChanged( int ) ) );
  connect( mSecondCombo, SIGNAL( highlighted( int ) ), SLOT( comboChanged( int ) ) );
  connect( mThirdCombo, SIGNAL( highlighted( int ) ), SLOT( comboChanged( int ) ) );
  connect( mFourthCombo, SIGNAL( highlighted( int ) ), SLOT( comboChanged( int ) ) );
}
    
PhoneEditWidget::~PhoneEditWidget()
{
}
    
void PhoneEditWidget::setPhoneNumbers( const KABC::PhoneNumber::List &list )
{
  mPhoneList = list;

  mPrefCombo->clear();
  mSecondCombo->clear();
  mThirdCombo->clear();
  mFourthCombo->clear();

  updateAllTypeCombos();
}

const KABC::PhoneNumber::List &PhoneEditWidget::phoneNumbers()
{
  return mPhoneList;
}

void PhoneEditWidget::updateTypeCombo( const KABC::PhoneNumber::List &list, KComboBox *combo )
{
  int pos = combo->currentItem();
  combo->clear();

  QListBox *lb = combo->listBox();

  KABC::PhoneNumber::List::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    new PhoneItem( lb, list, *it );

  lb->sort();
  combo->setCurrentItem( pos );
}

void PhoneEditWidget::updateAllTypeCombos()
{
  if ( mPhoneList.count() > 0 ) {
    updateTypeCombo( mPhoneList, mPrefCombo );
    mPrefCombo->setCurrentItem( 0 );
  }

  if ( mPhoneList.count() > 1 ) {
    updateTypeCombo( mPhoneList, mSecondCombo );
    mSecondCombo->setCurrentItem( 1 );
  }

  if ( mPhoneList.count() > 2 ) {
    updateTypeCombo( mPhoneList, mThirdCombo );
    mThirdCombo->setCurrentItem( 2 );
  }

  if ( mPhoneList.count() > 3 ) {
    updateTypeCombo( mPhoneList, mFourthCombo );
    mFourthCombo->setCurrentItem( 3 );
  }

    comboChanged( mPrefCombo, mPrefCombo->currentItem() );
    comboChanged( mSecondCombo, mSecondCombo->currentItem() );
    comboChanged( mThirdCombo, mThirdCombo->currentItem() );
    comboChanged( mFourthCombo, mFourthCombo->currentItem() );
}

KABC::PhoneNumber PhoneEditWidget::currentPhoneNumber( KComboBox *combo, int index )
{
  if ( index < 0 ) index = 0;
  if ( index >= combo->count() ) index = combo->count() - 1;

  QListBox *lb = combo->listBox();
  PhoneItem *item = dynamic_cast<PhoneItem*>( lb->item( index ) );
  if ( item )
    return item->phoneNumber();
  else
    return KABC::PhoneNumber();
}

void PhoneEditWidget::numberChanged( const QString &text )
{
  KLineEdit *edit = (KLineEdit*)sender();
  if ( !edit )
    return;

  KComboBox *combo = mEditMap[ edit ];
  if ( !combo )
    return;

  KABC::PhoneNumber number = currentPhoneNumber( combo, combo->currentItem() );

  KABC::PhoneNumber::List::Iterator it;
  for ( it = mPhoneList.begin(); it != mPhoneList.end(); ++it ) {
    if ( (*it).id() == number.id() ) {
      (*it).setNumber( text );
      bool blocked = combo->signalsBlocked();
      combo->blockSignals( true );
      updateTypeCombo( mPhoneList, combo );
      combo->blockSignals( blocked );
      return;
    }
  }
}

void PhoneEditWidget::comboChanged( int index )
{
  KComboBox *combo = (KComboBox*)sender();
  if ( !combo )
    return;

  comboChanged( combo, index );
}

void PhoneEditWidget::comboChanged( KComboBox *combo, int index )
{
  KABC::PhoneNumber number = currentPhoneNumber( combo, index );

  KABC::PhoneNumber::List::Iterator iter;
  for ( iter = mPhoneList.begin(); iter != mPhoneList.end(); ++iter ) {
    if ( (*iter).id() == number.id() )
      number = *iter;
  }

  QMap<KLineEdit*,KComboBox*>::Iterator it;
  for ( it = mEditMap.begin(); it != mEditMap.end(); ++it ) {
    if ( it.data() == combo ) {
      KLineEdit *edit = it.key();
      bool blocked = edit->signalsBlocked();
      edit->blockSignals( true );
      edit->setText( number.number() );
      edit->blockSignals( blocked );
      return;
    }
  }
}

void PhoneEditWidget::edit()
{
  PhoneEditDialog dlg( mPhoneList, this );
  
  if ( dlg.exec() ) {
    mPhoneList = dlg.phoneNumbers();
    updateAllTypeCombos();
    emit modified();
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
  mListView->setAllColumnsShowFocus( true );
  mListView->addColumn( i18n( "Number" ) );
  mListView->addColumn( i18n( "Type" ) );
  
  KButtonBox *buttonBox = new KButtonBox( page, Vertical );

  buttonBox->addButton( i18n( "&Add..." ), this, SLOT( slotAddPhoneNumber() ) );
  mRemoveButton = buttonBox->addButton( i18n( "&Remove" ), this, SLOT( slotRemovePhoneNumber() ) );
  mRemoveButton->setEnabled( false );
  mEditButton = buttonBox->addButton( i18n( "&Edit..." ), this, SLOT( slotEditPhoneNumber() ) );
  mEditButton->setEnabled( false );
  buttonBox->layout();

  layout->addWidget( mListView, 0, 0 );
  layout->addWidget( buttonBox, 0, 1 );

  connect( mListView, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()) );

  KABC::PhoneNumber::List::Iterator it;
  for ( it = mPhoneNumberList.begin(); it != mPhoneNumberList.end(); ++it )
    new PhoneViewItem( mListView, *it );
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
  }
}

void PhoneEditDialog::slotRemovePhoneNumber()
{
  PhoneViewItem *item = dynamic_cast<PhoneViewItem*>( mListView->currentItem() );
  if ( !item )
    return;

  mPhoneNumberList.remove( item->phoneNumber() );
  QListViewItem *currItem = mListView->currentItem();
  mListView->takeItem( currItem );
  delete currItem;
}

void PhoneEditDialog::slotEditPhoneNumber()
{
  PhoneViewItem *item = dynamic_cast<PhoneViewItem*>( mListView->currentItem() );
  if ( !item )
    return;

  PhoneTypeDialog dlg( item->phoneNumber(), this );
  
  if ( dlg.exec() ) {
    slotRemovePhoneNumber();
    KABC::PhoneNumber phoneNumber = dlg.phoneNumber();
    mPhoneNumberList.append( phoneNumber );
    new PhoneViewItem( mListView, phoneNumber );
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
