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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qpushbutton.h>

#include <kbuttonbox.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>

#include "addresseewidget.h"

NamePartWidget::NamePartWidget( const QString &title, QWidget *parent,
                                const char *name )
  : QWidget( parent, name )
{
  QHBoxLayout *layout = new QHBoxLayout( this );

  QGroupBox *group = new QGroupBox( 0, Qt::Vertical, title, this );
  QGridLayout *groupLayout = new QGridLayout( group->layout(), 2, 2,
                                              KDialog::spacingHint() );

  mBox = new QListBox( group );
  connect( mBox, SIGNAL( selectionChanged( QListBoxItem* ) ),
           SLOT( selectionChanged( QListBoxItem* ) ) );
  groupLayout->addWidget( mBox, 0, 0 );

  KButtonBox *bbox = new KButtonBox( group, Qt::Vertical );
  mAddButton = bbox->addButton( i18n( "Add" ), this,  SLOT( add() ) );
  mAddButton->setEnabled( false );
  mRemoveButton = bbox->addButton( i18n( "Remove" ), this,  SLOT( remove() ) );
  mRemoveButton->setEnabled( false );
  bbox->layout();
  groupLayout->addWidget( bbox, 0, 1 );

  mEdit = new KLineEdit( group );
  connect( mEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  connect( mEdit, SIGNAL( returnPressed() ), SLOT( add() ) );
  groupLayout->addMultiCellWidget( mEdit, 1, 1, 0, 1 );
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
  if ( !mEdit->text().isEmpty() ) {
    mBox->insertItem( mEdit->text() );
    emit modified();
  }

  mEdit->setText( "" );
}

void NamePartWidget::remove()
{
  mBox->removeItem( mBox->currentItem() );
  if ( mBox->count() == 0 )
    selectionChanged( 0 );

  emit modified();
}

void NamePartWidget::selectionChanged( QListBoxItem *item )
{
  mRemoveButton->setEnabled( item != 0 );
}

void NamePartWidget::textChanged( const QString& text )
{
  mAddButton->setEnabled( !text.isEmpty() );
}


AddresseeWidget::AddresseeWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *layout = new QGridLayout( this, 2, 3, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mPrefix = new NamePartWidget( i18n( "Prefixes" ), this );
  layout->addWidget( mPrefix, 0, 0 );
  
  mInclusion = new NamePartWidget( i18n( "Inclusions" ), this );
  layout->addWidget( mInclusion, 0, 1 );

  mSuffix = new NamePartWidget( i18n( "Suffixes" ), this );
  layout->addWidget( mSuffix, 0, 2 );

  QLabel *label = new QLabel( i18n( "Default Formatted Name" ), this );
  layout->addWidget( label, 1, 0 );

  mFormattedNameCombo = new KComboBox( this );
  mFormattedNameCombo->insertItem( i18n( "Empty" ) );
  mFormattedNameCombo->insertItem( i18n( "Simple Name" ) );
  mFormattedNameCombo->insertItem( i18n( "Full Name" ) );
  mFormattedNameCombo->insertItem( i18n( "Reverse Name" ) );
  layout->addMultiCellWidget( mFormattedNameCombo, 1, 1, 1, 2 );

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
}

#include "addresseewidget.moc"
