/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qapplication.h>

#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include "viewconfigurefieldspage.h"

class FieldItem : public QListBoxText
{
  public:
    FieldItem( QListBox *parent, KABC::Field *field )
      : QListBoxText( parent, field->label() ), mField( field ) {}

    FieldItem( QListBox *parent, KABC::Field *field, int index )
      : QListBoxText( parent, field->label(), parent->item( index ) ),
        mField( field ) {}

    KABC::Field *field() { return mField; }

  private:
    KABC::Field *mField;
};


ViewConfigureFieldsPage::ViewConfigureFieldsPage( KABC::AddressBook *ab,
                                                  QWidget *parent,
                                                  const char *name )
  : QWidget( parent, name ), mAddressBook( ab )
{
  initGUI();
}

void ViewConfigureFieldsPage::restoreSettings( KConfig *config )
{
  KABC::Field::List fields = KABC::Field::restoreFields( config, "KABCFields" );

  if ( fields.isEmpty() )
    fields = KABC::Field::defaultFields();

  KABC::Field::List::ConstIterator it;
  for ( it = fields.begin(); it != fields.end(); ++it )
    new FieldItem( mSelectedBox, *it );

  slotShowFields( mCategoryCombo->currentItem() );
}

void ViewConfigureFieldsPage::saveSettings( KConfig *config )
{
  KABC::Field::List fields;

  for ( uint i = 0; i < mSelectedBox->count(); ++i ) {
    FieldItem *fieldItem = static_cast<FieldItem *>( mSelectedBox->item( i ) );
    fields.append( fieldItem->field() );
  }

  KABC::Field::saveFields( config, "KABCFields", fields );
}

void ViewConfigureFieldsPage::slotShowFields( int index )
{
  int currentPos = mUnSelectedBox->currentItem();
  mUnSelectedBox->clear();

  int category;
  if ( index == 0 ) category = KABC::Field::All;
  else category = 1 << ( index - 1 );

  KABC::Field::List allFields = mAddressBook->fields( category );

  KABC::Field::List::ConstIterator it;
  for ( it = allFields.begin(); it != allFields.end(); ++it ) {
    QListBoxItem *item = mSelectedBox->firstItem();
    while( item ) {
      FieldItem *fieldItem = static_cast<FieldItem *>( item );
      if ( (*it)->equals( fieldItem->field() ) )
        break;
      item = item->next();
    }

    if ( !item )
      new FieldItem( mUnSelectedBox, *it );
  }

  mUnSelectedBox->sort();
  mUnSelectedBox->setCurrentItem( currentPos );
}

void ViewConfigureFieldsPage::slotSelect()
{
  // insert selected items in the unselected list to the selected list,
  // directoy under the current item if selected, or at the bottonm if
  // nothing is selected in the selected list
  int where = mSelectedBox->currentItem();
  if ( !(where > -1 && mSelectedBox->item( where )->isSelected()) )
    where = mSelectedBox->count() - 1;

  for ( uint i = 0; i < mUnSelectedBox->count(); ++i )
    if ( mUnSelectedBox->isSelected( mUnSelectedBox->item( i ) ) ) {
      FieldItem *fieldItem = static_cast<FieldItem *>( mUnSelectedBox->item( i ) );
      new FieldItem( mSelectedBox, fieldItem->field(), where );
      where++;
    }

  slotShowFields( mCategoryCombo->currentItem() );
}

void ViewConfigureFieldsPage::slotUnSelect()
{
  for ( uint i = 0; i < mSelectedBox->count(); ++i )
    if ( mSelectedBox->isSelected( mSelectedBox->item( i ) ) ) {
      mSelectedBox->removeItem( i );
      --i;
    }

  slotShowFields( mCategoryCombo->currentItem() );
}

void ViewConfigureFieldsPage::slotButtonsEnabled()
{
  bool state = false;
  // add button: enabled if any items are selected in the unselected list
  for ( uint i = 0; i < mUnSelectedBox->count(); ++i )
    if ( mUnSelectedBox->item( i )->isSelected() ) {
      state = true;
      break;
    }
  mAddButton->setEnabled( state );

  int j = mSelectedBox->currentItem();
  state = ( j > -1 && mSelectedBox->isSelected( j ) );

  // up button: enabled if there is a current item > 0 and that is selected
  mUpButton->setEnabled( ( j > 0 && state ) );

  // down button: enabled if there is a current item < count - 2 and that is selected
  mDownButton->setEnabled( ( j > -1 && j < (int)mSelectedBox->count() - 1 && state ) );

  // remove button: enabled if any items are selected in the selected list
  state = false;
  for ( uint i = 0; i < mSelectedBox->count(); ++i )
    if ( mSelectedBox->item( i )->isSelected() ) {
      state = true;
      break;
    }
  mRemoveButton->setEnabled( state );
}

void ViewConfigureFieldsPage::slotMoveUp()
{
  int i = mSelectedBox->currentItem();
  if ( i > 0 ) {
    QListBoxItem *item = mSelectedBox->item( i );
    mSelectedBox->takeItem( item );
    mSelectedBox->insertItem( item, i - 1 );
    mSelectedBox->setCurrentItem( item );
    mSelectedBox->setSelected( i - 1, true );
  }
}

void ViewConfigureFieldsPage::slotMoveDown()
{
  int i = mSelectedBox->currentItem();
  if ( i > -1 && i < (int)mSelectedBox->count() - 1 ) {
    QListBoxItem *item = mSelectedBox->item( i );
    mSelectedBox->takeItem( item );
    mSelectedBox->insertItem( item, i + 1 );
    mSelectedBox->setCurrentItem( item );
    mSelectedBox->setSelected( i + 1, true );
  }
}

void ViewConfigureFieldsPage::initGUI()
{
  setCaption( i18n("Select Fields to Display") );

  QGridLayout *gl = new QGridLayout( this , 6, 4, 0, KDialog::spacingHint() );

  mCategoryCombo = new KComboBox( false, this );
  mCategoryCombo->insertItem( KABC::Field::categoryLabel( KABC::Field::All ) );
  mCategoryCombo->insertItem( KABC::Field::categoryLabel( KABC::Field::Frequent ) );
  mCategoryCombo->insertItem( KABC::Field::categoryLabel( KABC::Field::Address ) );
  mCategoryCombo->insertItem( KABC::Field::categoryLabel( KABC::Field::Email ) );
  mCategoryCombo->insertItem( KABC::Field::categoryLabel( KABC::Field::Personal ) );
  mCategoryCombo->insertItem( KABC::Field::categoryLabel( KABC::Field::Organization ) );
  mCategoryCombo->insertItem( KABC::Field::categoryLabel( KABC::Field::CustomCategory ) );
  connect( mCategoryCombo, SIGNAL( activated(int) ), SLOT( slotShowFields(int) ) );
  gl->addWidget( mCategoryCombo, 0, 0 );

  QLabel *label = new QLabel( i18n( "&Selected fields:" ), this );
  gl->addWidget( label, 0, 2 );

  mUnSelectedBox = new QListBox( this );
  mUnSelectedBox->setSelectionMode( QListBox::Extended );
  mUnSelectedBox->setMinimumHeight( 100 );
  gl->addWidget( mUnSelectedBox, 1, 0 );

  mSelectedBox = new QListBox( this );
  mSelectedBox->setSelectionMode( QListBox::Extended );
  label->setBuddy( mSelectedBox );
  gl->addWidget( mSelectedBox, 1, 2 );

  QBoxLayout *vb1 = new QBoxLayout( QBoxLayout::TopToBottom, KDialog::spacingHint() );
  vb1->addStretch();

  mAddButton = new QToolButton( this );
  mAddButton->setIconSet( QApplication::reverseLayout() ? SmallIconSet( "1leftarrow" ) : SmallIconSet( "1rightarrow" ) );
  connect( mAddButton, SIGNAL( clicked() ), SLOT( slotSelect() ) );
  vb1->addWidget( mAddButton );

  mRemoveButton = new QToolButton( this );
  mRemoveButton->setIconSet( QApplication::reverseLayout() ? SmallIconSet( "1rightarrow" ) : SmallIconSet( "1leftarrow" ) );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( slotUnSelect() ) );
  vb1->addWidget( mRemoveButton );

  vb1->addStretch();
  gl->addLayout( vb1, 1, 1 );

  QBoxLayout *vb2 = new QBoxLayout( QBoxLayout::TopToBottom, KDialog::spacingHint() );
  vb2->addStretch();

  mUpButton = new QToolButton( this );
  mUpButton->setIconSet( SmallIconSet( "1uparrow" ) );
  connect( mUpButton, SIGNAL( clicked() ), SLOT( slotMoveUp() ) );
  vb2->addWidget( mUpButton );

  mDownButton = new QToolButton( this );
  mDownButton->setIconSet( SmallIconSet( "1downarrow" ) );
  connect( mDownButton, SIGNAL( clicked() ), SLOT( slotMoveDown() ) );
  vb2->addWidget( mDownButton );

  vb2->addStretch();
  gl->addLayout( vb2, 1, 3 );

  QSize sizeHint = mUnSelectedBox->sizeHint();

  // make sure we fill the list with all items, so that we can
  // get the maxItemWidth we need to not truncate the view
  slotShowFields( 0 );

  sizeHint = sizeHint.expandedTo( mSelectedBox->sizeHint() );
  sizeHint.setWidth( mUnSelectedBox->maxItemWidth() );
  mUnSelectedBox->setMinimumSize( sizeHint );
  mSelectedBox->setMinimumSize( sizeHint );

  gl->activate();

  connect( mUnSelectedBox, SIGNAL( selectionChanged() ), SLOT( slotButtonsEnabled() ) );
  connect( mSelectedBox, SIGNAL( selectionChanged() ), SLOT( slotButtonsEnabled() ) );
  connect( mSelectedBox, SIGNAL( currentChanged( QListBoxItem * ) ), SLOT( slotButtonsEnabled() ) );

  slotButtonsEnabled();
}

#include "viewconfigurefieldspage.moc"
