/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kabc/resource.h>
#include <kconfig.h>
#include <klocale.h>
#include <kresources/manager.h>

#include <qlabel.h>
#include <qlayout.h>

#include "kabckonnector.h"

#include "kabckonnectorconfig.h"

using namespace KSync;

KABCKonnectorConfig::KABCKonnectorConfig( QWidget *parent )
  : KRES::ConfigWidget( parent, 0 )
{
  initGUI();

  KRES::Manager<KABC::Resource> manager( "contact" );
  manager.readConfig();
  KRES::Manager<KABC::Resource>::ActiveIterator it;
  for ( it = manager.activeBegin(); it != manager.activeEnd(); ++it ) {
    mResourceIdentifiers.append( (*it)->identifier() );
    mResourceBox->insertItem( (*it)->resourceName() );
  }
}

KABCKonnectorConfig::~KABCKonnectorConfig()
{
}

void KABCKonnectorConfig::loadSettings( KRES::Resource *resource )
{
  KABCKonnector *konnector = dynamic_cast<KABCKonnector *>( resource );
  if ( konnector ) {
    int pos = mResourceIdentifiers.findIndex( konnector->currentResource() );
    mResourceBox->setCurrentItem( pos );
  }
}

void KABCKonnectorConfig::saveSettings( KRES::Resource *resource )
{
  KABCKonnector *konnector = dynamic_cast<KABCKonnector *>( resource );
  if ( konnector ) {
    int pos = mResourceBox->currentItem();
    konnector->setCurrentResource( mResourceIdentifiers[ pos ] );
  }
}

void KABCKonnectorConfig::initGUI()
{
  QBoxLayout *layout = new QVBoxLayout( this );

  QLabel *label = new QLabel( i18n( "Select the address book you want to sync with." ), this );
  layout->addWidget( label );

  mResourceBox = new QComboBox( this );
  layout->addWidget( mResourceBox );
}

#include "kabckonnectorconfig.moc"
