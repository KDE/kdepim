/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
                       Tobias Koenig <tokoe@kde.org>

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

#include <tqbuttongroup.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpixmap.h>
#include <tqradiobutton.h>

#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>

#include "stylepage.h"

StylePage::StylePage( KABC::AddressBook *ab, TQWidget* parent,  const char* name )
  : TQWidget( parent, name ), mAddressBook( ab )
{
  initGUI();

  initFieldCombo();

  mSortTypeCombo->insertItem( i18n( "Ascending" ) );
  mSortTypeCombo->insertItem( i18n( "Descending" ) );

  connect( mStyleCombo, TQT_SIGNAL( activated( int ) ), TQT_SIGNAL( styleChanged( int ) ) );
}

StylePage::~StylePage()
{
}

void StylePage::setPreview( const TQPixmap &pixmap )
{
  if ( pixmap.isNull() )
    mPreview->setText( i18n( "(No preview available.)" ) );
  else
    mPreview->setPixmap( pixmap );
}

void StylePage::addStyleName( const TQString &name )
{
  mStyleCombo->insertItem( name );
}

void StylePage::clearStyleNames()
{
  mStyleCombo->clear();
}

void StylePage::setSortField( KABC::Field *field )
{
  mFieldCombo->setCurrentText( field->label() );
}

void StylePage::setSortAscending( bool value )
{
  if ( value )
    mSortTypeCombo->setCurrentItem( 0 );
  else
    mSortTypeCombo->setCurrentItem( 1 );
}

KABC::Field* StylePage::sortField()
{
  if ( mFieldCombo->currentItem() == -1 )
    return mFields[ 0 ];

  return mFields[ mFieldCombo->currentItem() ];
}

bool StylePage::sortAscending()
{
  return ( mSortTypeCombo->currentItem() == 0 );
}

void StylePage::initFieldCombo()
{
  if ( !mAddressBook )
    return;

  mFieldCombo->clear();

  mFields = mAddressBook->fields( KABC::Field::All );
  KABC::Field::List::ConstIterator it;
  for ( it = mFields.begin(); it != mFields.end(); ++it )
    mFieldCombo->insertItem( (*it)->label() );
}

void StylePage::initGUI()
{
  setCaption( i18n( "Choose Printing Style" ) );

  TQGridLayout *topLayout = new TQGridLayout( this, 2, 2, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  TQLabel *label = new TQLabel( i18n( "What should the print look like?\n"
                                    "KAddressBook has several printing styles, designed for different purposes.\n"
                                    "Choose the style that suits your needs below." ), this );
  topLayout->addMultiCellWidget( label, 0, 0, 0, 1 );

  TQButtonGroup *group = new TQButtonGroup( i18n( "Sorting" ), this );
  group->setColumnLayout( 0, Qt::Vertical );
  TQGridLayout *sortLayout = new TQGridLayout( group->layout(), 2, 2,
                                             KDialog::spacingHint() );
  sortLayout->setAlignment( Qt::AlignTop );

  label = new TQLabel( i18n( "Criterion:" ), group );
  sortLayout->addWidget( label, 0, 0 );

  mFieldCombo = new KComboBox( false, group );
  sortLayout->addWidget( mFieldCombo, 0, 1 );

  label = new TQLabel( i18n( "Order:" ), group );
  sortLayout->addWidget( label, 1, 0 );

  mSortTypeCombo = new KComboBox( false, group );
  sortLayout->addWidget( mSortTypeCombo, 1, 1 );

  topLayout->addWidget( group, 1, 0 );

  group = new TQButtonGroup( i18n( "Print Style" ), this );
  group->setColumnLayout( 0, Qt::Vertical );
  TQVBoxLayout *styleLayout = new TQVBoxLayout( group->layout(),
                                              KDialog::spacingHint() );

  mStyleCombo = new KComboBox( false, group );
  styleLayout->addWidget( mStyleCombo );

  mPreview = new TQLabel( group );
  TQFont font( mPreview->font() );
  font.setPointSize( 20 );
  mPreview->setFont( font );
  mPreview->setScaledContents( true );
  mPreview->setAlignment( int( TQLabel::WordBreak | TQLabel::AlignCenter ) );
  styleLayout->addWidget( mPreview );

  topLayout->addWidget( group, 1, 1 );
}

#include "stylepage.moc"
