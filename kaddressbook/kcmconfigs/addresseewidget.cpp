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

#include <q3cstring.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3listbox.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QGridLayout>

#include <dcopclient.h>

#include <kbuttonbox.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <klineedit.h>

#include "addresseewidget.h"

NamePartWidget::NamePartWidget( const QString &title, const QString &label,
                                QWidget *parent, const char *name )
  : QWidget( parent, name ), mTitle( title ), mLabel( label )
{
  QHBoxLayout *layout = new QHBoxLayout( this );

  Q3GroupBox *group = new Q3GroupBox( 0, Qt::Vertical, title, this );
  QGridLayout *groupLayout = new QGridLayout( group->layout() );
  groupLayout->setSpacing( KDialog::spacingHint() );

  mBox = new Q3ListBox( group );
  connect( mBox, SIGNAL( selectionChanged( Q3ListBoxItem* ) ),
           SLOT( selectionChanged( Q3ListBoxItem* ) ) );
  groupLayout->addWidget( mBox, 0, 0 );

  KButtonBox *bbox = new KButtonBox( group, Qt::Vertical );
  mAddButton = bbox->addButton( i18n( "Add..." ), this,  SLOT( add() ) );
  mEditButton = bbox->addButton( i18n( "Edit..." ), this,  SLOT( edit() ) );
  mEditButton->setEnabled( false );
  mRemoveButton = bbox->addButton( i18n( "Remove" ), this,  SLOT( remove() ) );
  mRemoveButton->setEnabled( false );
  bbox->layout();
  groupLayout->addWidget( bbox, 0, 1 );

  layout->addWidget( group );
}

NamePartWidget::~NamePartWidget()
{
}

void NamePartWidget::setNameParts( const QStringList &list )
{
  mBox->clear();
  mBox->insertStringList( list );
}

QStringList NamePartWidget::nameParts() const
{
  QStringList parts;
  for ( uint i = 0; i < mBox->count(); ++i )
    parts.append( mBox->text( i ) );

  return parts;
}

void NamePartWidget::add()
{
  bool ok;

  QString namePart = KInputDialog::getText( i18n( "New" ), mLabel,
                                            QString(), &ok );
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

  QString namePart = KInputDialog::getText( i18n( "Edit" ), mLabel,
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

void NamePartWidget::selectionChanged( Q3ListBoxItem *item )
{
  mEditButton->setEnabled( item != 0 );
  mRemoveButton->setEnabled( item != 0 );
}



AddresseeWidget::AddresseeWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *layout = new QGridLayout( this );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  mPrefix = new NamePartWidget( i18n( "Prefixes"), i18n( "Enter prefix:" ), this );
  layout->addWidget( mPrefix, 0, 0 );

  mInclusion = new NamePartWidget( i18n( "Inclusions"), i18n( "Enter inclusion:" ), this );
  layout->addWidget( mInclusion, 0, 1 );

  mSuffix = new NamePartWidget( i18n( "Suffixes" ), i18n( "Enter suffix:" ), this );
  layout->addWidget( mSuffix, 0, 2 );

  QLabel *label = new QLabel( i18n( "Default formatted name:" ), this );
  layout->addWidget( label, 1, 0 );

  mFormattedNameCombo = new KComboBox( this );
  mFormattedNameCombo->addItem( i18n( "Empty" ) );
  mFormattedNameCombo->addItem( i18n( "Simple Name" ) );
  mFormattedNameCombo->addItem( i18n( "Full Name" ) );
  mFormattedNameCombo->addItem( i18n( "Reverse Name with Comma" ) );
  mFormattedNameCombo->addItem( i18n( "Reverse Name" ) );
  layout->addWidget( mFormattedNameCombo, 1, 1, 1, 2 );

  connect( mPrefix, SIGNAL( modified() ), SIGNAL( modified() ) );
  connect( mInclusion, SIGNAL( modified() ), SIGNAL( modified() ) );
  connect( mSuffix, SIGNAL( modified() ), SIGNAL( modified() ) );
  connect( mFormattedNameCombo, SIGNAL( activated( int ) ), SIGNAL( modified() ) );
}

AddresseeWidget::~AddresseeWidget()
{
}

void AddresseeWidget::restoreSettings()
{
  KConfig config( "kabcrc" );
  config.setGroup( "General" );

  mPrefix->setNameParts( config.readEntry( "Prefixes" , QStringList() ) );
  mInclusion->setNameParts( config.readEntry( "Inclusions" , QStringList() ) );
  mSuffix->setNameParts( config.readEntry( "Suffixes" , QStringList() ) );

  KConfig cfg( "kaddressbookrc" );
  cfg.setGroup( "General" );
  mFormattedNameCombo->setCurrentItem( cfg.readEntry( "FormattedNameType", 1 ) );
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
  cfg.writeEntry( "FormattedNameType", mFormattedNameCombo->currentIndex() );

  DCOPClient *client = DCOPClient::mainClient();
  if ( client )
      client->emitDCOPSignal( "KABC::AddressBookConfig", "changed()", QByteArray() );
}

#include "addresseewidget.moc"
