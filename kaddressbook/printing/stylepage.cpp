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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qradiobutton.h>

#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>

#include "stylepage.h"

StylePage::StylePage( KABC::AddressBook *ab, QWidget* parent,  const char* name )
  : QWidget( parent, name ), mAddressBook( ab )
{
  initGUI();

  initFieldCombo();

  mSortTypeCombo->insertItem( i18n( "Ascending" ) );
  mSortTypeCombo->insertItem( i18n( "Descending" ) );

  connect( mStyleCombo, SIGNAL( activated( int ) ), SIGNAL( styleChanged( int ) ) );
}

StylePage::~StylePage()
{
}

void StylePage::setPreview( const QPixmap &pixmap )
{
  if ( pixmap.isNull() )
    mPreview->setText( i18n( "(No preview available.)" ) );
  else
    mPreview->setPixmap( pixmap );
}

void StylePage::addStyleName( const QString &name )
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

  QGridLayout *topLayout = new QGridLayout( this, 2, 2, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "What should the print look like?\n"
                                    "KAddressBook has several printing styles, designed for different purposes.\n"
                                    "Choose the style that suits your needs below." ), this );
  topLayout->addMultiCellWidget( label, 0, 0, 0, 1 );

  QButtonGroup *group = new QButtonGroup( i18n( "Sorting" ), this );
  group->setColumnLayout( 0, Qt::Vertical );
  QGridLayout *sortLayout = new QGridLayout( group->layout(), 2, 2,
                                             KDialog::spacingHint() );
  sortLayout->setAlignment( Qt::AlignTop );

  label = new QLabel( i18n( "Criterion:" ), group );
  sortLayout->addWidget( label, 0, 0 );
  
  mFieldCombo = new KComboBox( false, group );
  sortLayout->addWidget( mFieldCombo, 0, 1 );

  label = new QLabel( i18n( "Order:" ), group );
  sortLayout->addWidget( label, 1, 0 );
  
  mSortTypeCombo = new KComboBox( false, group );
  sortLayout->addWidget( mSortTypeCombo, 1, 1 );

  topLayout->addWidget( group, 1, 0 );

  group = new QButtonGroup( i18n( "Print Style" ), this );
  group->setColumnLayout( 0, Qt::Vertical );
  QVBoxLayout *styleLayout = new QVBoxLayout( group->layout(),
                                              KDialog::spacingHint() );

  mStyleCombo = new KComboBox( false, group );
  styleLayout->addWidget( mStyleCombo );

  mPreview = new QLabel( group );
  QFont font( mPreview->font() );
  font.setPointSize( 20 );
  mPreview->setFont( font );
  mPreview->setScaledContents( true );
  mPreview->setAlignment( int( QLabel::WordBreak | QLabel::AlignCenter ) );
  styleLayout->addWidget( mPreview );

  topLayout->addWidget( group, 1, 1 );
}

#include "stylepage.moc"
