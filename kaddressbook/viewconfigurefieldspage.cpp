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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QtGui/QApplication>
#include <QtGui/QBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>

#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include "viewconfigurefieldspage.h"

class FieldItem : public QListWidgetItem
{
  public:
    FieldItem( QListWidget *parent, KABC::Field *field )
      : QListWidgetItem( field->label(), parent ), mField( field ) {}

    KABC::Field *field() { return mField; }

  private:
    KABC::Field *mField;
};


ViewConfigureFieldsPage::ViewConfigureFieldsPage( KABC::AddressBook *ab,
                                                  QWidget *parent,
                                                  const char *name )
  : QWidget( parent ), mAddressBook( ab )
{
   setObjectName( name );
   initGUI();
}

void ViewConfigureFieldsPage::restoreSettings( const KConfigGroup &config )
{
  KABC::Field::List fields = KABC::Field::restoreFields( config, "KABCFields" );

  if ( fields.isEmpty() )
    fields = KABC::Field::defaultFields();

  KABC::Field::List::ConstIterator it;
  for ( it = fields.constBegin(); it != fields.constEnd(); ++it )
    new FieldItem( mSelectedBox, *it );

  slotShowFields( mCategoryCombo->currentIndex() );
}

void ViewConfigureFieldsPage::saveSettings( KConfigGroup &config )
{
  KABC::Field::List fields;

  for ( int i = 0; i < mSelectedBox->count(); ++i ) {
    FieldItem *fieldItem = static_cast<FieldItem *>( mSelectedBox->item( i ) );
    fields.append( fieldItem->field() );
  }

  KABC::Field::saveFields( config, "KABCFields", fields );
}

void ViewConfigureFieldsPage::slotShowFields( int index )
{
  int currentPos = mUnSelectedBox->currentRow();
  mUnSelectedBox->clear();

  int category;
  if ( index == 0 ) category = KABC::Field::All;
  else category = 1 << ( index - 1 );

  KABC::Field::List allFields = mAddressBook->fields( category );

  KABC::Field::List::ConstIterator it;
  for ( it = allFields.constBegin(); it != allFields.constEnd(); ++it ) {
    bool found = false;

    for ( int i = 0; i < mSelectedBox->count(); ++i ) {
      FieldItem *fieldItem = static_cast<FieldItem *>( mSelectedBox->item( i ) );
      if ( (*it)->equals( fieldItem->field() ) ) {
        found = true;
        break;
      }
    }

    if ( !found )
      new FieldItem( mUnSelectedBox, *it );
  }

  mUnSelectedBox->setCurrentRow( currentPos );
}

void ViewConfigureFieldsPage::slotSelect()
{
  // insert selected items in the unselected list to the selected list,
  // directory under the current item if selected, or at the bottonm if
  // nothing is selected in the selected list
  int where = mSelectedBox->currentRow();
  if ( !(where > -1 && mSelectedBox->item( where )->isSelected()) )
    where = mSelectedBox->count() - 1;

  for ( int i = 0; i < mUnSelectedBox->count(); ++i )
    if ( mUnSelectedBox->item( i )->isSelected() ) {
      FieldItem *fieldItem = static_cast<FieldItem *>( mUnSelectedBox->item( i ) );
      mUnSelectedBox->insertItem( where, new FieldItem( mSelectedBox, fieldItem->field() ) );
      where++;
    }

  slotShowFields( mCategoryCombo->currentIndex() );
}

void ViewConfigureFieldsPage::slotUnSelect()
{
  for ( int i = 0; i < mSelectedBox->count(); ++i )
    if ( mSelectedBox->item( i )->isSelected() ) {
      mSelectedBox->takeItem( i );
      --i;
    }

  slotShowFields( mCategoryCombo->currentIndex() );
}

void ViewConfigureFieldsPage::slotButtonsEnabled()
{
  bool state = false;
  // add button: enabled if any items are selected in the unselected list
  for ( int i = 0; i < mUnSelectedBox->count(); ++i )
    if ( mUnSelectedBox->item( i )->isSelected() ) {
      state = true;
      break;
    }
  mAddButton->setEnabled( state );

  QListWidgetItem *item = mSelectedBox->currentItem();
  int j = mSelectedBox->currentRow();
  state = ( item && item->isSelected() );

  // up button: enabled if there is a current item > 0 and that is selected
  mUpButton->setEnabled( j > 0 && state );

  // down button: enabled if there is a current item < count - 2 and that is selected
  mDownButton->setEnabled( j < (int)mSelectedBox->count() - 1 && state );

  // remove button: enabled if any items are selected in the selected list
  state = false;
  for ( int i = 0; i < mSelectedBox->count(); ++i )
    if ( mSelectedBox->item( i )->isSelected() ) {
      state = true;
      break;
    }
  mRemoveButton->setEnabled( state );
}

void ViewConfigureFieldsPage::slotMoveUp()
{
  int i = mSelectedBox->currentRow();
  if ( i > 0 ) {
    QListWidgetItem *item = mSelectedBox->item( i );
    mSelectedBox->takeItem( i );
    mSelectedBox->insertItem( i - 1, item );
    mSelectedBox->setCurrentItem( item );
    mSelectedBox->item( i - 1 )->setSelected( true );
  }
}

void ViewConfigureFieldsPage::slotMoveDown()
{
  int i = mSelectedBox->currentRow();
  if ( i > -1 && i < (int)mSelectedBox->count() - 1 ) {
    QListWidgetItem *item = mSelectedBox->item( i );
    mSelectedBox->takeItem( i );
    mSelectedBox->insertItem( i + 1, item );
    mSelectedBox->setCurrentItem( item );
    mSelectedBox->item( i + 1 )->setSelected( true );
  }
}

void ViewConfigureFieldsPage::initGUI()
{
  setWindowTitle( i18n( "Select Fields to Display" ) );

  QGridLayout *gl = new QGridLayout( this  );
  gl->setSpacing( KDialog::spacingHint() );
  gl->setMargin( 0 );

  mCategoryCombo = new KComboBox( false, this );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::All ) );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::Frequent ) );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::Address ) );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::Email ) );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::Personal ) );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::Organization ) );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::CustomCategory ) );
  connect( mCategoryCombo, SIGNAL( activated(int) ), SLOT( slotShowFields(int) ) );
  gl->addWidget( mCategoryCombo, 0, 0 );

  QLabel *label = new QLabel( i18n( "&Selected fields:" ), this );
  gl->addWidget( label, 0, 2 );

  mUnSelectedBox = new QListWidget( this );
  mUnSelectedBox->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mUnSelectedBox->setSortingEnabled( true );
  mUnSelectedBox->setMinimumHeight( 100 );
  gl->addWidget( mUnSelectedBox, 1, 0 );

  mSelectedBox = new QListWidget( this );
  mSelectedBox->setSelectionMode( QAbstractItemView::ExtendedSelection );
  label->setBuddy( mSelectedBox );
  gl->addWidget( mSelectedBox, 1, 2 );

  QBoxLayout *vb1 = new QVBoxLayout();
  vb1->setSpacing( KDialog::spacingHint() );
  vb1->addStretch();

  mAddButton = new QToolButton( this );
  mAddButton->setIcon( QApplication::isRightToLeft() ? KIcon( "arrow-left" ) : KIcon( "arrow-right" ) );
  connect( mAddButton, SIGNAL( clicked() ), SLOT( slotSelect() ) );
  vb1->addWidget( mAddButton );

  mRemoveButton = new QToolButton( this );
  mRemoveButton->setIcon( QApplication::isRightToLeft() ? KIcon( "arrow-right" ) : KIcon( "arrow-left" ) );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( slotUnSelect() ) );
  vb1->addWidget( mRemoveButton );

  vb1->addStretch();
  gl->addLayout( vb1, 1, 1 );

  QBoxLayout *vb2 = new QVBoxLayout();
  vb2->setSpacing( KDialog::spacingHint() );
  vb2->addStretch();

  mUpButton = new QToolButton( this );
  mUpButton->setIcon( KIcon( "arrow-up" ) );
  connect( mUpButton, SIGNAL( clicked() ), SLOT( slotMoveUp() ) );
  vb2->addWidget( mUpButton );

  mDownButton = new QToolButton( this );
  mDownButton->setIcon( KIcon( "arrow-down" ) );
  connect( mDownButton, SIGNAL( clicked() ), SLOT( slotMoveDown() ) );
  vb2->addWidget( mDownButton );

  vb2->addStretch();
  gl->addLayout( vb2, 1, 3 );

  QSize sizeHint = mUnSelectedBox->sizeHint();

  // make sure we fill the list with all items, so that we can
  // get the maxItemWidth we need to not truncate the view
  slotShowFields( 0 );

  sizeHint = sizeHint.expandedTo( mSelectedBox->sizeHint() );
  sizeHint.setWidth( mUnSelectedBox->sizeHintForColumn( 0 ) );
  mUnSelectedBox->setMinimumSize( sizeHint );
  mSelectedBox->setMinimumSize( sizeHint );

  gl->activate();

  connect( mUnSelectedBox, SIGNAL( itemSelectionChanged() ), SLOT( slotButtonsEnabled() ) );
  connect( mSelectedBox, SIGNAL( itemSelectionChanged() ), SLOT( slotButtonsEnabled() ) );
  connect( mSelectedBox, 
           SIGNAL( currentItemChanged( QListWidgetItem *, QListWidgetItem * ) ), 
           SLOT( slotButtonsEnabled() ) );

  slotButtonsEnabled();
}

#include "viewconfigurefieldspage.moc"
