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

#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QGridLayout>

#include <KDialogButtonBox>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <QDBusMessage>
#include <QDBusConnection>
#include "addresseewidget.h"

NamePartWidget::NamePartWidget( const QString &title, const QString &label,
                                QWidget *parent, const char *name )
  : QWidget( parent ), mTitle( title ), mLabel( label )
{
  setObjectName( name );
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( 0 );

  QGroupBox *group = new QGroupBox( title, this );
  QGridLayout *groupLayout = new QGridLayout();
  groupLayout->setSpacing( KDialog::spacingHint() );
  groupLayout->setMargin( KDialog::marginHint() );
  group->setLayout( groupLayout );

  mBox = new QListWidget( group );
  connect( mBox, 
           SIGNAL( currentItemChanged( QListWidgetItem *, QListWidgetItem * ) ),
           SLOT( selectionChanged( QListWidgetItem * ) ) );
  groupLayout->addWidget( mBox, 0, 0 );

  KDialogButtonBox *bbox = new KDialogButtonBox( group, Qt::Vertical );
  mAddButton = bbox->addButton( i18n( "Add..." ), QDialogButtonBox::ActionRole, this,  SLOT( add() ) );
  mEditButton = bbox->addButton( i18n( "Edit..." ), QDialogButtonBox::ActionRole, this,  SLOT( edit() ) );
  mEditButton->setEnabled( false );
  mRemoveButton = bbox->addButton( i18n( "Remove" ), QDialogButtonBox::ActionRole, this,  SLOT( remove() ) );
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
  mBox->addItems( list );
}

QStringList NamePartWidget::nameParts() const
{
  QStringList parts;
  for ( int i = 0; i < mBox->count(); ++i )
    parts.append( mBox->item( i )->text() );

  return parts;
}

void NamePartWidget::add()
{
  bool ok;

  QString namePart = KInputDialog::getText( i18n( "New" ), mLabel,
                                            QString(), &ok );
  if ( ok && !namePart.isEmpty() ) {
    mBox->addItem( namePart );
    emit modified();
  }
}

void NamePartWidget::edit()
{
  bool ok;

  QListWidgetItem *item = mBox->currentItem();
  if ( !item )
    return;

  QString namePart = KInputDialog::getText( i18n( "Edit" ), mLabel,
                                            item->text(), &ok );
  if ( ok && !namePart.isEmpty() ) {
    item->setText( namePart );
    emit modified();
  }
}

void NamePartWidget::remove()
{
  mBox->takeItem( mBox->currentRow() );
  if ( mBox->count() == 0 )
    selectionChanged( 0 );

  emit modified();
}

void NamePartWidget::selectionChanged( QListWidgetItem *item )
{
  mEditButton->setEnabled( item );
  mRemoveButton->setEnabled( item );
}



AddresseeWidget::AddresseeWidget( QWidget *parent, const char *name )
  : QWidget( parent )
{
  setObjectName( name );
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
  KConfig _config( "kabcrc" );
  KConfigGroup config(&_config, "General" );

  mPrefix->setNameParts( config.readEntry( "Prefixes" , QStringList() ) );
  mInclusion->setNameParts( config.readEntry( "Inclusions" , QStringList() ) );
  mSuffix->setNameParts( config.readEntry( "Suffixes" , QStringList() ) );

  KConfig _cfg( "kaddressbookrc" );
  KConfigGroup cfg(&_cfg, "General" );
  mFormattedNameCombo->setCurrentIndex( cfg.readEntry( "FormattedNameType", 1 ) );
}

void AddresseeWidget::saveSettings()
{
  KConfig _config( "kabcrc" );
  KConfigGroup config(&_config, "General" );

  config.writeEntry( "Prefixes", mPrefix->nameParts() );
  config.writeEntry( "Inclusions", mInclusion->nameParts() );
  config.writeEntry( "Suffixes", mSuffix->nameParts() );

  KConfig _cfg( "kaddressbookrc" );
  KConfigGroup cfg(&_cfg, "General" );
  cfg.writeEntry( "FormattedNameType", mFormattedNameCombo->currentIndex() );

   QDBusMessage message =
       QDBusMessage::createSignal(QString(), "org.kde.kabc.AddressBookConfig", "changed");
   QDBusConnection::sessionBus().send(message);

}

#include "addresseewidget.moc"
