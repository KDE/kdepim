/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "collectiongeneralpage.h"

#include "blockalarmsattribute.h"

#include <akonadi/collection.h>
#include <akonadi/entitydisplayattribute.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>

#include <QtGui/QCheckBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

using namespace Akonadi;
using namespace CalendarSupport;

CollectionGeneralPage::CollectionGeneralPage( QWidget *parent )
  : CollectionPropertiesPage( parent )
{
  setObjectName( QLatin1String( "CalendarSupport::CollectionGeneralPage" ) );
  setPageTitle( i18nc( "@title:tab General settings for a folder.", "General" ) );

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );

  QHBoxLayout *hbox = new QHBoxLayout();
  topLayout->addItem( hbox );
  hbox->setSpacing( KDialog::spacingHint() );
  hbox->setMargin( KDialog::marginHint() );

  QLabel *label = new QLabel( i18nc( "@label:textbox Name of the folder.", "&Name:" ), this );
  hbox->addWidget( label );

  mNameEdit = new KLineEdit( this );
  label->setBuddy( mNameEdit );
  hbox->addWidget( mNameEdit );

  // should replies to mails in this folder be kept in this same folder?
  hbox = new QHBoxLayout();
  topLayout->addItem( hbox );
  hbox->setSpacing( KDialog::spacingHint() );
  hbox->setMargin( KDialog::marginHint() );

  mBlockAlarmsCheckBox = new QCheckBox( i18n( "Block alarms locally" ), this );
  hbox->addWidget( mBlockAlarmsCheckBox );
  hbox->addStretch( 1 );

  topLayout->addStretch( 100 ); // eat all superfluous space
}

CollectionGeneralPage::~CollectionGeneralPage()
{
}

void CollectionGeneralPage::load( const Akonadi::Collection &collection )
{
  mNameEdit->setEnabled( collection.rights() & Collection::CanChangeCollection );

  const QString displayName = collection.hasAttribute<EntityDisplayAttribute>() ?
                                  collection.attribute<EntityDisplayAttribute>()->displayName() : collection.name();

  mNameEdit->setText( displayName );

  mBlockAlarmsCheckBox->setChecked( collection.hasAttribute<BlockAlarmsAttribute>() );
}

void CollectionGeneralPage::save( Collection &collection )
{
  if ( collection.hasAttribute<EntityDisplayAttribute>() &&
       !collection.attribute<EntityDisplayAttribute>()->displayName().isEmpty() )
    collection.attribute<EntityDisplayAttribute>()->setDisplayName( mNameEdit->text() );
  else
    collection.setName( mNameEdit->text() );

  if ( mBlockAlarmsCheckBox->isChecked() ) {
    if ( !collection.hasAttribute<BlockAlarmsAttribute>() )
      collection.attribute<BlockAlarmsAttribute>( Collection::AddIfMissing );
  } else {
    collection.removeAttribute<BlockAlarmsAttribute>();
  }
}

#include "collectiongeneralpage.moc"
