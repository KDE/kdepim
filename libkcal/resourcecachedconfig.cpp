/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qbuttongroup.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qhbox.h>
#include <qlabel.h>

#include <klocale.h>
#include <kdebug.h>

#include "resourcecached.h"

#include "resourcecachedconfig.h"

using namespace KCal;

ResourceCachedReloadConfig::ResourceCachedReloadConfig( QWidget *parent,
                                                        const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  mGroup = new QButtonGroup( 1, Horizontal, i18n("Automatic Reload"), this );
  topLayout->addWidget( mGroup );
  new QRadioButton( i18n("Never"), mGroup );
  new QRadioButton( i18n("On startup"), mGroup );

  QRadioButton *intervalRadio = new QRadioButton( i18n("Regular interval"),
                                                  mGroup );
  connect( intervalRadio, SIGNAL( stateChanged( int ) ),
           SLOT( slotIntervalStateChanged( int ) ) );
  QHBox *intervalBox = new QHBox( mGroup );
  new QLabel( i18n("Interval in minutes"), intervalBox );
  mIntervalSpin = new QSpinBox( 1,900, 1,intervalBox );
  mIntervalSpin->setEnabled( false );
}

void ResourceCachedReloadConfig::loadSettings( ResourceCached *resource )
{
  mGroup->setButton( resource->reloadPolicy() );
  mIntervalSpin->setValue( resource->reloadInterval() );
}

void ResourceCachedReloadConfig::saveSettings( ResourceCached *resource )
{
  resource->setReloadPolicy( mGroup->selectedId() );
  resource->setReloadInterval( mIntervalSpin->value() );
}

void ResourceCachedReloadConfig::slotIntervalStateChanged( int state )
{
  if ( state == QButton::On ) mIntervalSpin->setEnabled( true );
  else mIntervalSpin->setEnabled( false );
}


ResourceCachedSaveConfig::ResourceCachedSaveConfig( QWidget *parent,
                                                        const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  mGroup = new QButtonGroup( 1, Horizontal, i18n("Automatic Save"), this );
  topLayout->addWidget( mGroup );
  new QRadioButton( i18n("Never"), mGroup );
  new QRadioButton( i18n("On exit"), mGroup );

  QRadioButton *intervalRadio = new QRadioButton( i18n("Regular interval"),
                                                  mGroup );
  connect( intervalRadio, SIGNAL( stateChanged( int ) ),
           SLOT( slotIntervalStateChanged( int ) ) );
  QHBox *intervalBox = new QHBox( mGroup );
  new QLabel( i18n("Interval in minutes"), intervalBox );
  mIntervalSpin = new QSpinBox( 1,900, 1,intervalBox );
  mIntervalSpin->setEnabled( false );

  new QRadioButton( i18n("Delayed after changes"), mGroup );
  new QRadioButton( i18n("On every change"), mGroup );
}

void ResourceCachedSaveConfig::loadSettings( ResourceCached *resource )
{
  mGroup->setButton( resource->savePolicy() );
  mIntervalSpin->setValue( resource->saveInterval() );
}

void ResourceCachedSaveConfig::saveSettings( ResourceCached *resource )
{
  resource->setSavePolicy( mGroup->selectedId() );
  resource->setSaveInterval( mIntervalSpin->value() );
}

void ResourceCachedSaveConfig::slotIntervalStateChanged( int state )
{
  if ( state == QButton::On ) mIntervalSpin->setEnabled( true );
  else mIntervalSpin->setEnabled( false );
}

#include "resourcecachedconfig.moc"
