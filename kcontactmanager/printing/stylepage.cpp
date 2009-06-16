/*
    This file is part of KContactManager.
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

#include "stylepage.h"

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QPixmap>
#include <QtGui/QVBoxLayout>

#include <KComboBox>
#include <KDialog>
#include <KLocale>

StylePage::StylePage( QWidget* parent,  const char* name )
  : QWidget( parent )
{
  setObjectName( name );
  initGUI();

  initFieldCombo();

  mSortTypeCombo->addItem( i18n( "Ascending" ) );
  mSortTypeCombo->addItem( i18n( "Descending" ) );

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
  mStyleCombo->addItem( name );
}

void StylePage::clearStyleNames()
{
  mStyleCombo->clear();
}

void StylePage::setSortField( ContactFields::Field field )
{
  mFieldCombo->setItemText( mFieldCombo->currentIndex(), ContactFields::label( field) );
}

void StylePage::setSortAscending( bool value )
{
  if ( value )
    mSortTypeCombo->setCurrentIndex( 0 );
  else
    mSortTypeCombo->setCurrentIndex( 1 );
}

ContactFields::Field StylePage::sortField()
{
  if ( mFieldCombo->currentIndex() == -1 )
    return mFields[ 0 ];

  return mFields[ mFieldCombo->currentIndex() ];
}

bool StylePage::sortAscending()
{
  return ( mSortTypeCombo->currentIndex() == 0 );
}

void StylePage::initFieldCombo()
{
  mFieldCombo->clear();

  mFields = ContactFields::allFields();
  ContactFields::Fields::ConstIterator it;
  for ( it = mFields.constBegin(); it != mFields.constEnd(); ++it )
    mFieldCombo->addItem( ContactFields::label(*it) );
}

void StylePage::initGUI()
{
  setWindowTitle( i18n( "Choose Printing Style" ) );

  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( KDialog::marginHint() );

  QLabel *label = new QLabel( i18n( "What should the print look like?\n"
                                    "KAddressBook has several printing styles, designed for different purposes.\n"
                                    "Choose the style that suits your needs below." ), this );
  topLayout->addWidget( label, 0, 0, 1, 2 );

  QGroupBox *group = new QGroupBox( i18n( "Sorting" ), this );
  QGridLayout *sortLayout = new QGridLayout();
  group->setLayout( sortLayout );
  sortLayout->setSpacing( KDialog::spacingHint() );
  sortLayout->setMargin( KDialog::marginHint() );
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

  group = new QGroupBox( i18n( "Print Style" ), this );
  QVBoxLayout *styleLayout = new QVBoxLayout();
  group->setLayout( styleLayout );
  styleLayout->setSpacing( KDialog::spacingHint() );
  styleLayout->setMargin( KDialog::marginHint() );

  mStyleCombo = new KComboBox( false, group );
  styleLayout->addWidget( mStyleCombo );

  mPreview = new QLabel( group );
  QFont font( mPreview->font() );
  font.setPointSize( 20 );
  mPreview->setFont( font );
  mPreview->setScaledContents( true );
  mPreview->setAlignment( Qt::AlignCenter );
  mPreview->setWordWrap( true );
  styleLayout->addWidget( mPreview );

  topLayout->addWidget( group, 1, 1 );
}

#include "stylepage.moc"
