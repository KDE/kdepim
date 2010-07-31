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

#include <tqbuttongroup.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqradiobutton.h>

#include <kconfig.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>

#include "viewconfigurefilterpage.h"
#include "filter.h"

ViewConfigureFilterPage::ViewConfigureFilterPage( TQWidget *parent,
                                                  const char *name )
  : TQWidget( parent, name )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this, 0, KDialog::spacingHint() );

  mFilterGroup = new TQButtonGroup();
  connect( mFilterGroup, TQT_SIGNAL( clicked( int ) ), TQT_SLOT( buttonClicked( int ) ) );

  TQLabel *label = new TQLabel( i18n( "The default filter will be activated whenever"
  " this view is displayed. This feature allows you to configure views that only"
  " interact with certain types of information based on the filter. Once the view"
  " is activated, the filter can be changed at anytime." ), this );
  label->setAlignment( Qt::AlignLeft | Qt::AlignTop | Qt::WordBreak );
  topLayout->addWidget( label );

  TQWidget *spacer = new TQWidget( this );
  spacer->setMinimumHeight( 5 );
  topLayout->addWidget( spacer );

  TQRadioButton *button = new TQRadioButton( i18n( "No default filter" ), this );
  mFilterGroup->insert( button );
  topLayout->addWidget( button );

  button = new TQRadioButton( i18n( "Use last active filter" ), this );
  mFilterGroup->insert( button );
  topLayout->addWidget( button );

  TQBoxLayout *comboLayout = new TQHBoxLayout();
  topLayout->addLayout( comboLayout );
  button = new TQRadioButton( i18n( "Use filter:" ), this );
  mFilterGroup->insert( button );
  comboLayout->addWidget( button );

  mFilterCombo = new KComboBox( this );
  comboLayout->addWidget( mFilterCombo );

  topLayout->addStretch( 100 );
}

ViewConfigureFilterPage::~ViewConfigureFilterPage()
{
  delete mFilterGroup;
}

void ViewConfigureFilterPage::restoreSettings( KConfig *config )
{
  mFilterCombo->clear();

  // Load the filter combo
  const Filter::List list = Filter::restore( config, "Filter" );
  Filter::List::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    mFilterCombo->insertItem( (*it).name() );

  int id = config->readNumEntry( "DefaultFilterType", 1 );
  mFilterGroup->setButton( id );
  buttonClicked( id );

  if ( id == 2 ) // has default filter
    mFilterCombo->setCurrentText( config->readEntry( "DefaultFilterName" ) );
}

void ViewConfigureFilterPage::saveSettings( KConfig *config )
{
  config->writeEntry( "DefaultFilterName", mFilterCombo->currentText() );
  config->writeEntry( "DefaultFilterType", mFilterGroup->id( mFilterGroup->selected() ) );
}

void ViewConfigureFilterPage::buttonClicked( int id )
{
  mFilterCombo->setEnabled( id == 2 );
}

#include "viewconfigurefilterpage.moc"
