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

#include <kconfig.h>
#include <klocale.h>
#include <kresources/manager.h>
#include <libkcal/resourcecalendar.h>

#include <qlabel.h>
#include <qlayout.h>

#include "kcalkonnector.h"

#include "kcalkonnectorconfig.h"

using namespace KSync;

KCalKonnectorConfig::KCalKonnectorConfig( QWidget *parent )
  : KRES::ConfigWidget( parent, 0 )
{
  initGUI();

  KRES::Manager<KCal::ResourceCalendar> manager( "calendar" );
  manager.readConfig();
  KRES::Manager<KCal::ResourceCalendar>::ActiveIterator it;
  for ( it = manager.activeBegin(); it != manager.activeEnd(); ++it ) {
    mResourceIdentifiers.append( (*it)->identifier() );
    mResourceBox->insertItem( (*it)->resourceName() );
  }
}

KCalKonnectorConfig::~KCalKonnectorConfig()
{
}

void KCalKonnectorConfig::loadSettings( KRES::Resource *resource )
{
  KCalKonnector *konnector = dynamic_cast<KCalKonnector *>( resource );
  if ( konnector ) {
    int pos = mResourceIdentifiers.findIndex( konnector->currentResource() );
    mResourceBox->setCurrentItem( pos );
  }
}

void KCalKonnectorConfig::saveSettings( KRES::Resource *resource )
{
  KCalKonnector *konnector = dynamic_cast<KCalKonnector *>( resource );
  if ( konnector ) {
    int pos = mResourceBox->currentItem();
    konnector->setCurrentResource( mResourceIdentifiers[ pos ] );
  }
}

void KCalKonnectorConfig::initGUI()
{
  QBoxLayout *layout = new QVBoxLayout( this );

  QLabel *label = new QLabel( i18n( "Select the calendar you want to sync with." ), this );
  layout->addWidget( label );

  mResourceBox = new QComboBox( this );
  layout->addWidget( mResourceBox );
}

#include "kcalkonnectorconfig.moc"
