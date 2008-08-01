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

#include <QtGui/QBoxLayout>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QVBoxLayout>

#include <kconfig.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>

#include "viewconfigurefilterpage.h"
#include "filter.h"

ViewConfigureFilterPage::ViewConfigureFilterPage( QWidget *parent,
                                                  const char *name )
  : QWidget( parent )
{
  setObjectName( name );
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );

  mFilterGroup = new QButtonGroup();
  connect( mFilterGroup, SIGNAL( buttonClicked( int ) ), SLOT( buttonClicked( int ) ) );

  QLabel *label = new QLabel( i18n( "The default filter will be activated whenever"
  " this view is displayed. This feature allows you to configure views that only"
  " interact with certain types of information based on the filter. Once the view"
  " is activated, the filter can be changed at anytime." ), this );
  label->setAlignment( Qt::AlignLeft | Qt::AlignTop );
  label->setWordWrap( true );
  topLayout->addWidget( label );

  QWidget *spacer = new QWidget( this );
  spacer->setMinimumHeight( 5 );
  topLayout->addWidget( spacer );

  QRadioButton *button = new QRadioButton( i18n( "No default filter" ), this );
  mFilterGroup->addButton( button,0 );
  topLayout->addWidget( button );

  button = new QRadioButton( i18n( "Use last active filter" ), this );
  mFilterGroup->addButton( button,1 );
  topLayout->addWidget( button );

  QBoxLayout *comboLayout = new QHBoxLayout();
  topLayout->addLayout( comboLayout );
  button = new QRadioButton( i18n( "Use filter:" ), this );
  mFilterGroup->addButton( button,2 );
  comboLayout->addWidget( button );

  mFilterCombo = new KComboBox( this );
  comboLayout->addWidget( mFilterCombo );

  topLayout->addStretch( 100 );
}

ViewConfigureFilterPage::~ViewConfigureFilterPage()
{
}

void ViewConfigureFilterPage::restoreSettings( const KConfigGroup &config )
{
  mFilterCombo->clear();

  // Load the filter combo
  const Filter::List list = Filter::restore( config.config(), "Filter" );
  Filter::List::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    mFilterCombo->addItem( (*it).name() );

  int id = config.readEntry( "DefaultFilterType", 1 );
  mFilterGroup->button ( id )->setChecked(true);
  buttonClicked( id );

  if ( id == 2 ) // has default filter
    mFilterCombo->setItemText( mFilterCombo->currentIndex(), config.readEntry( "DefaultFilterName" ) );
}

void ViewConfigureFilterPage::saveSettings( KConfigGroup &config )
{
  config.writeEntry( "DefaultFilterName", mFilterCombo->currentText() );
  config.writeEntry( "DefaultFilterType", mFilterGroup->id( mFilterGroup->checkedButton() ) );
}

void ViewConfigureFilterPage::buttonClicked( int id )
{
  mFilterCombo->setEnabled( id == 2 );
}

#include "viewconfigurefilterpage.moc"
