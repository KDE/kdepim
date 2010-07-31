/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <tqcstring.h>
#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlistbox.h>
#include <tqpushbutton.h>

#include <dcopclient.h>

#include <kbuttonbox.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <klineedit.h>

#include "addresseewidget.h"

NamePartWidget::NamePartWidget( const TQString &title, const TQString &label,
                                TQWidget *parent, const char *name )
  : TQWidget( parent, name ), mTitle( title ), mLabel( label )
{
  TQHBoxLayout *layout = new TQHBoxLayout( this );

  TQGroupBox *group = new TQGroupBox( 0, Qt::Vertical, title, this );
  TQGridLayout *groupLayout = new TQGridLayout( group->layout(), 2, 2,
                                              KDialog::spacingHint() );

  mBox = new TQListBox( group );
  connect( mBox, TQT_SIGNAL( selectionChanged( TQListBoxItem* ) ),
           TQT_SLOT( selectionChanged( TQListBoxItem* ) ) );
  groupLayout->addWidget( mBox, 0, 0 );

  KButtonBox *bbox = new KButtonBox( group, Qt::Vertical );
  mAddButton = bbox->addButton( i18n( "Add..." ), this,  TQT_SLOT( add() ) );
  mEditButton = bbox->addButton( i18n( "Edit..." ), this,  TQT_SLOT( edit() ) );
  mEditButton->setEnabled( false );
  mRemoveButton = bbox->addButton( i18n( "Remove" ), this,  TQT_SLOT( remove() ) );
  mRemoveButton->setEnabled( false );
  bbox->layout();
  groupLayout->addWidget( bbox, 0, 1 );

  layout->addWidget( group );
}

NamePartWidget::~NamePartWidget()
{
}

void NamePartWidget::setNameParts( const TQStringList &list )
{
  mBox->clear();
  mBox->insertStringList( list );
}

TQStringList NamePartWidget::nameParts() const
{
  TQStringList parts;
  for ( uint i = 0; i < mBox->count(); ++i )
    parts.append( mBox->text( i ) );

  return parts;
}

void NamePartWidget::add()
{
  bool ok;

  TQString namePart = KInputDialog::getText( i18n( "New" ), mLabel,
                                            TQString::null, &ok );
  if ( ok && !namePart.isEmpty() ) {
    mBox->insertItem( namePart );
    emit modified();
  }
}

void NamePartWidget::edit()
{
  bool ok;

  int index = mBox->currentItem();
  if ( index == -1 )
    return;

  TQString namePart = KInputDialog::getText( i18n( "Edit" ), mLabel,
                                            mBox->text( index ), &ok );
  if ( ok && !namePart.isEmpty() ) {
    mBox->changeItem( namePart, index );
    emit modified();
  }
}

void NamePartWidget::remove()
{
  mBox->removeItem( mBox->currentItem() );
  if ( mBox->count() == 0 )
    selectionChanged( 0 );

  emit modified();
}

void NamePartWidget::selectionChanged( TQListBoxItem *item )
{
  mEditButton->setEnabled( item != 0 );
  mRemoveButton->setEnabled( item != 0 );
}



AddresseeWidget::AddresseeWidget( TQWidget *parent, const char *name )
  : TQWidget( parent, name )
{
  TQGridLayout *layout = new TQGridLayout( this, 2, 3, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mPrefix = new NamePartWidget( i18n( "Prefixes"), i18n( "Enter prefix:" ), this );
  layout->addWidget( mPrefix, 0, 0 );

  mInclusion = new NamePartWidget( i18n( "Inclusions"), i18n( "Enter inclusion:" ), this );
  layout->addWidget( mInclusion, 0, 1 );

  mSuffix = new NamePartWidget( i18n( "Suffixes" ), i18n( "Enter suffix:" ), this );
  layout->addWidget( mSuffix, 0, 2 );

  TQLabel *label = new TQLabel( i18n( "Default formatted name:" ), this );
  layout->addWidget( label, 1, 0 );

  mFormattedNameCombo = new KComboBox( this );
  mFormattedNameCombo->insertItem( i18n( "Empty" ) );
  mFormattedNameCombo->insertItem( i18n( "Simple Name" ) );
  mFormattedNameCombo->insertItem( i18n( "Full Name" ) );
  mFormattedNameCombo->insertItem( i18n( "Reverse Name with Comma" ) );
  mFormattedNameCombo->insertItem( i18n( "Reverse Name" ) );
  layout->addMultiCellWidget( mFormattedNameCombo, 1, 1, 1, 2 );

  connect( mPrefix, TQT_SIGNAL( modified() ), TQT_SIGNAL( modified() ) );
  connect( mInclusion, TQT_SIGNAL( modified() ), TQT_SIGNAL( modified() ) );
  connect( mSuffix, TQT_SIGNAL( modified() ), TQT_SIGNAL( modified() ) );
  connect( mFormattedNameCombo, TQT_SIGNAL( activated( int ) ), TQT_SIGNAL( modified() ) );
}

AddresseeWidget::~AddresseeWidget()
{
}

void AddresseeWidget::restoreSettings()
{
  KConfig config( "kabcrc" );
  config.setGroup( "General" );

  mPrefix->setNameParts( config.readListEntry( "Prefixes" ) );
  mInclusion->setNameParts( config.readListEntry( "Inclusions" ) );
  mSuffix->setNameParts( config.readListEntry( "Suffixes" ) );

  KConfig cfg( "kaddressbookrc" );
  cfg.setGroup( "General" );
  mFormattedNameCombo->setCurrentItem( cfg.readNumEntry( "FormattedNameType", 1 ) );
}

void AddresseeWidget::saveSettings()
{
  KConfig config( "kabcrc" );
  config.setGroup( "General" );

  config.writeEntry( "Prefixes", mPrefix->nameParts() );
  config.writeEntry( "Inclusions", mInclusion->nameParts() );
  config.writeEntry( "Suffixes", mSuffix->nameParts() );

  KConfig cfg( "kaddressbookrc" );
  cfg.setGroup( "General" );
  cfg.writeEntry( "FormattedNameType", mFormattedNameCombo->currentItem() );

  DCOPClient *client = DCOPClient::mainClient();
  if ( client )
      client->emitDCOPSignal( "KABC::AddressBookConfig", "changed()", TQByteArray() );
}

#include "addresseewidget.moc"
