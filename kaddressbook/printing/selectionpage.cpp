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

#include "selectionpage.h"

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QVBoxLayout>

#include <kdialog.h>
#include <klocale.h>

SelectionPage::SelectionPage( QWidget* parent )
    : QWidget( parent )
{
  setWindowTitle( i18n( "Choose Which Contacts to Print" ) );

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( KDialog::marginHint() );

  QLabel *label = new QLabel( i18n( "Which contacts do you want to print?" ), this );
  topLayout->addWidget( label );

  mButtonGroup = new QGroupBox( this );

  QVBoxLayout *groupLayout = new QVBoxLayout();
  mButtonGroup->setLayout( groupLayout );
  mButtonGroup->layout()->setSpacing( KDialog::spacingHint() );
  mButtonGroup->layout()->setMargin( KDialog::marginHint() );
  groupLayout->setAlignment( Qt::AlignTop );

  mUseWholeBook = new QRadioButton( i18n( "&All contacts" ), mButtonGroup );
  mUseWholeBook->setChecked( true );
  mUseWholeBook->setWhatsThis( i18n( "Print the entire address book" ) );
  groupLayout->addWidget( mUseWholeBook );

  mUseSelection = new QRadioButton( i18n( "&Selected contacts" ), mButtonGroup );
  mUseSelection->setWhatsThis( i18n( "Only print contacts selected in KAddressBook.\n"
                                     "This option is disabled if no contacts are selected." ) );
  groupLayout->addWidget( mUseSelection );


  topLayout->addWidget( mButtonGroup );
  topLayout->addStretch( 1 );
}

SelectionPage::~SelectionPage()
{
}

void SelectionPage::setUseSelection( bool value )
{
  mUseSelection->setEnabled( value );
}

bool SelectionPage::useSelection() const
{
  return mUseSelection->isChecked();
}

#include "selectionpage.moc"
