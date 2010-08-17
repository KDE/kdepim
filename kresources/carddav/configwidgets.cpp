/*=========================================================================
| KABCDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Automatic Reload / Automatic Save configuration widgets.
| The code is mostly taken from resourcecachedconfig.h/cpp files from
| the kcal library and changed to meet our requirements.
| The original copyright is below.
 ========================================================================*/
/*
  This file is part of the kcal library.

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

#include "configwidgets.h"

#include <kabcresourcecached.h>

#include <klocale.h>
#include <kdebug.h>

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>
#include <tqspinbox.h>
#include <tqbuttongroup.h>
#include <tqgroupbox.h>
#include <tqhbox.h>

using namespace KABC;

//@cond PRIVATE
class CardDavConfigPrivate
{
  public:
    CardDavConfigPrivate()
      : mGroup( 0 ),
        mIntervalSpin( 0 ) {}

    TQButtonGroup *mGroup;
    TQSpinBox *mIntervalSpin;
};

class CardDavReloadConfig::Private
  : public CardDavConfigPrivate
{
};

class CardDavSaveConfig::Private
  : public CardDavConfigPrivate
{
};
//@endcond

CardDavReloadConfig::CardDavReloadConfig( TQWidget *parent )
  : TQWidget( parent ), d( new KABC::CardDavReloadConfig::Private() )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );

  //TQGroupBox *groupBox = new TQGroupBox( i18nc( "@title:group", "Automatic Reload" ), this );
  TQGroupBox *groupBox = new TQGroupBox( i18n( "Automatic Reload" ), this );
  topLayout->addWidget( groupBox );

  TQRadioButton *noAutomaticReload =
    new TQRadioButton(
      //i18nc( "@option:radio never reload the cache", "Never" ), groupBox );
      i18n( "Never" ), groupBox );
  TQRadioButton *automaticReloadOnStartup =
    new TQRadioButton(
      //i18nc( "@option:radio reload the cache on startup", "Only on startup" ), groupBox );
      i18n( "Only on startup" ), groupBox );
  TQRadioButton *intervalRadio =
    new TQRadioButton(
//       i18nc( "@option:radio reload the cache at regular intervals",
//              "Regular interval" ), groupBox );
      i18n( "Regular interval" ), groupBox );

  d->mGroup = new TQButtonGroup( this );
  d->mGroup->hide();
  d->mGroup->insert( intervalRadio, 2 );
  d->mGroup->insert( automaticReloadOnStartup, 1 );
  d->mGroup->insert( noAutomaticReload, 0 );

  connect( intervalRadio, TQT_SIGNAL( toggled( bool ) ),
           TQT_SLOT( slotIntervalToggled( bool ) ) );

  TQHBox *intervalBox = new TQHBox( groupBox );
  //new TQLabel( i18nc( "@label:spinbox", "Interval in minutes:" ), intervalBox );
  new TQLabel( i18n( "Interval in minutes:" ), intervalBox );
  d->mIntervalSpin = new TQSpinBox( intervalBox );
  d->mIntervalSpin->setRange( 1, 900 );
  d->mIntervalSpin->setEnabled( false );

  groupBox->setColumnLayout(1, Qt::Vertical);
  TQVBoxLayout *vbox = new TQVBoxLayout(groupBox->layout());
  vbox->addWidget(intervalRadio);
  vbox->addWidget(intervalBox);
  vbox->addWidget(automaticReloadOnStartup);
  vbox->addWidget(noAutomaticReload);
  vbox->addStretch(1);
}

CardDavReloadConfig::~CardDavReloadConfig()
{
   delete d;
}

void CardDavReloadConfig::loadSettings( ResourceCached *resource )
{
  d->mIntervalSpin->setValue( resource->reloadInterval() );
  d->mGroup->setButton( resource->reloadPolicy() );
}

void CardDavReloadConfig::saveSettings( ResourceCached *resource )
{
  resource->setReloadInterval( d->mIntervalSpin->value() );
  resource->setReloadPolicy( d->mGroup->selectedId() );
}

void CardDavReloadConfig::slotIntervalToggled( bool checked )
{
  if ( checked ) {
    d->mIntervalSpin->setEnabled( true );
  } else {
    d->mIntervalSpin->setEnabled( false );
  }
}

CardDavSaveConfig::CardDavSaveConfig( TQWidget *parent )
  : TQWidget( parent ), d( new KABC::CardDavSaveConfig::Private() )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );

  //TQGroupBox *groupBox = new TQGroupBox( i18nc( "@title:group", "Automatic Save" ), this );
  TQGroupBox *groupBox = new TQGroupBox( i18n( "Automatic Save" ), this );
  d->mGroup = new TQButtonGroup( this );
  d->mGroup->hide();
  topLayout->addWidget( groupBox );

  TQRadioButton *never =
    new TQRadioButton(
      //i18nc( "@option:radio never save the cache automatically", "Never" ), groupBox );
      i18n( "Never" ), groupBox );
  TQRadioButton *onExit =
    new TQRadioButton(
      //i18nc( "@option:radio save the cache on exit", "Only on exit" ), groupBox );
      i18n( "Only on exit" ), groupBox );

  TQRadioButton *intervalRadio =
    new TQRadioButton(
      //i18nc( "@option:radio save the cache at regular intervals", "Regular interval" ), groupBox );
      i18n( "Regular interval" ), groupBox );

  d->mGroup = new TQButtonGroup( this );
  d->mGroup->hide();
  d->mGroup->insert( never, 0 );
  d->mGroup->insert( onExit, 1 );
  d->mGroup->insert( intervalRadio, 2 );

  connect( intervalRadio, TQT_SIGNAL( toggled( bool ) ),
           TQT_SLOT( slotIntervalToggled( bool ) ) );

  TQHBox *intervalBox = new TQHBox( groupBox );
  //new TQLabel( i18nc( "@label:spinbox", "Interval in minutes:" ), intervalBox );
  new TQLabel( i18n( "Interval in minutes:" ), intervalBox );
  d->mIntervalSpin = new TQSpinBox( intervalBox );
  d->mIntervalSpin->setRange( 1, 900 );
  d->mIntervalSpin->setEnabled( false );

  TQRadioButton *delay =
    new TQRadioButton(
//       i18nc( "@option:radio save the cache after some delay",
//              "Delayed after changes" ), groupBox );
      i18n( "Delayed after changes" ), groupBox );
  TQRadioButton *every =
    new TQRadioButton(
//       i18nc( "@option:radio save the cache after every modification",
//              "Immediately after changes" ), groupBox );
      i18n( "Immediately after changes" ), groupBox );
  d->mGroup->insert( delay, 3 );
  d->mGroup->insert( every, 4 );

  // hide unwanted widgets. They may be useful in future, so don't delete them for now.
  intervalRadio->hide();
  intervalBox->hide();

  groupBox->setColumnLayout(1, Qt::Vertical);
  TQVBoxLayout *vbox = new TQVBoxLayout(groupBox->layout());
  vbox->addWidget(delay);
  vbox->addWidget(every);
  vbox->addWidget(intervalRadio);
  vbox->addWidget(intervalBox);
  vbox->addWidget(onExit);
  vbox->addWidget(never);
  vbox->addStretch(1);
}

CardDavSaveConfig::~CardDavSaveConfig()
{
  delete d;
}

void CardDavSaveConfig::loadSettings( ResourceCached *resource )
{
  d->mIntervalSpin->setValue( resource->saveInterval() );
  d->mGroup->setButton( resource->savePolicy() );
}

void CardDavSaveConfig::saveSettings( ResourceCached *resource )
{
  resource->setSaveInterval( d->mIntervalSpin->value() );
  resource->setSavePolicy( d->mGroup->selectedId() );
}

void CardDavSaveConfig::slotIntervalToggled( bool checked )
{
  if ( checked ) {
    d->mIntervalSpin->setEnabled( true );
  } else {
    d->mIntervalSpin->setEnabled( false );
  }
}

// EOF ========================================================================
