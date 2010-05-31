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

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qhbox.h>

using namespace KABC;

//@cond PRIVATE
class CardDavConfigPrivate
{
  public:
    CardDavConfigPrivate()
      : mGroup( 0 ),
        mIntervalSpin( 0 ) {}

    QButtonGroup *mGroup;
    QSpinBox *mIntervalSpin;
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

CardDavReloadConfig::CardDavReloadConfig( QWidget *parent )
  : QWidget( parent ), d( new KABC::CardDavReloadConfig::Private() )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  //QGroupBox *groupBox = new QGroupBox( i18nc( "@title:group", "Automatic Reload" ), this );
  QGroupBox *groupBox = new QGroupBox( i18n( "Automatic Reload" ), this );
  topLayout->addWidget( groupBox );

  QRadioButton *noAutomaticReload =
    new QRadioButton(
      //i18nc( "@option:radio never reload the cache", "Never" ), groupBox );
      i18n( "Never" ), groupBox );
  QRadioButton *automaticReloadOnStartup =
    new QRadioButton(
      //i18nc( "@option:radio reload the cache on startup", "Only on startup" ), groupBox );
      i18n( "Only on startup" ), groupBox );
  QRadioButton *intervalRadio =
    new QRadioButton(
//       i18nc( "@option:radio reload the cache at regular intervals",
//              "Regular interval" ), groupBox );
      i18n( "Regular interval" ), groupBox );

  d->mGroup = new QButtonGroup( this );
  d->mGroup->hide();
  d->mGroup->insert( intervalRadio, 2 );
  d->mGroup->insert( automaticReloadOnStartup, 1 );
  d->mGroup->insert( noAutomaticReload, 0 );

  connect( intervalRadio, SIGNAL( toggled( bool ) ),
           SLOT( slotIntervalToggled( bool ) ) );

  QHBox *intervalBox = new QHBox( groupBox );
  //new QLabel( i18nc( "@label:spinbox", "Interval in minutes:" ), intervalBox );
  new QLabel( i18n( "Interval in minutes:" ), intervalBox );
  d->mIntervalSpin = new QSpinBox( intervalBox );
  d->mIntervalSpin->setRange( 1, 900 );
  d->mIntervalSpin->setEnabled( false );

  groupBox->setColumnLayout(1, Qt::Vertical);
  QVBoxLayout *vbox = new QVBoxLayout(groupBox->layout());
  vbox->addWidget(intervalRadio);
  vbox->addWidget(intervalBox);
  vbox->addWidget(automaticReloadOnStartup);
  vbox->addWidget(noAutomaticReload);
  vbox->addStretch(1);

  // FIXME KABC
  groupBox->hide();
}

CardDavReloadConfig::~CardDavReloadConfig()
{
   delete d;
}

void CardDavReloadConfig::loadSettings( ResourceCached *resource )
{
  // FIXME KABC
  //d->mIntervalSpin->setValue( resource->reloadInterval() );
  //d->mGroup->setButton( resource->reloadPolicy() );
}

void CardDavReloadConfig::saveSettings( ResourceCached *resource )
{
  // FIXME KABC
  //resource->setReloadInterval( d->mIntervalSpin->value() );
  //resource->setReloadPolicy( d->mGroup->selectedId() );
}

void CardDavReloadConfig::slotIntervalToggled( bool checked )
{
  if ( checked ) {
    d->mIntervalSpin->setEnabled( true );
  } else {
    d->mIntervalSpin->setEnabled( false );
  }
}

CardDavSaveConfig::CardDavSaveConfig( QWidget *parent )
  : QWidget( parent ), d( new KABC::CardDavSaveConfig::Private() )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  //QGroupBox *groupBox = new QGroupBox( i18nc( "@title:group", "Automatic Save" ), this );
  QGroupBox *groupBox = new QGroupBox( i18n( "Automatic Save" ), this );
  d->mGroup = new QButtonGroup( this );
  d->mGroup->hide();
  topLayout->addWidget( groupBox );

  QRadioButton *never =
    new QRadioButton(
      //i18nc( "@option:radio never save the cache automatically", "Never" ), groupBox );
      i18n( "Never" ), groupBox );
  QRadioButton *onExit =
    new QRadioButton(
      //i18nc( "@option:radio save the cache on exit", "Only on exit" ), groupBox );
      i18n( "Only on exit" ), groupBox );

  QRadioButton *intervalRadio =
    new QRadioButton(
      //i18nc( "@option:radio save the cache at regular intervals", "Regular interval" ), groupBox );
      i18n( "Regular interval" ), groupBox );

  d->mGroup = new QButtonGroup( this );
  d->mGroup->hide();
  d->mGroup->insert( never, 0 );
  d->mGroup->insert( onExit, 1 );
  d->mGroup->insert( intervalRadio, 2 );

  connect( intervalRadio, SIGNAL( toggled( bool ) ),
           SLOT( slotIntervalToggled( bool ) ) );

  QHBox *intervalBox = new QHBox( groupBox );
  //new QLabel( i18nc( "@label:spinbox", "Interval in minutes:" ), intervalBox );
  new QLabel( i18n( "Interval in minutes:" ), intervalBox );
  d->mIntervalSpin = new QSpinBox( intervalBox );
  d->mIntervalSpin->setRange( 1, 900 );
  d->mIntervalSpin->setEnabled( false );

  QRadioButton *delay =
    new QRadioButton(
//       i18nc( "@option:radio save the cache after some delay",
//              "Delayed after changes" ), groupBox );
      i18n( "Delayed after changes" ), groupBox );
  QRadioButton *every =
    new QRadioButton(
//       i18nc( "@option:radio save the cache after every modification",
//              "Immediately after changes" ), groupBox );
      i18n( "Immediately after changes" ), groupBox );
  d->mGroup->insert( delay, 3 );
  d->mGroup->insert( every, 4 );

  // hide unwanted widgets. They may be useful in future, so don't delete them for now.
  intervalRadio->hide();
  intervalBox->hide();

  groupBox->setColumnLayout(1, Qt::Vertical);
  QVBoxLayout *vbox = new QVBoxLayout(groupBox->layout());
  vbox->addWidget(delay);
  vbox->addWidget(every);
  vbox->addWidget(intervalRadio);
  vbox->addWidget(intervalBox);
  vbox->addWidget(onExit);
  vbox->addWidget(never);
  vbox->addStretch(1);

  // FIXME KABC
  groupBox->hide();
}

CardDavSaveConfig::~CardDavSaveConfig()
{
  delete d;
}

void CardDavSaveConfig::loadSettings( ResourceCached *resource )
{
  // FIXME KABC
  //d->mIntervalSpin->setValue( resource->saveInterval() );
  //d->mGroup->setButton( resource->savePolicy() );
}

void CardDavSaveConfig::saveSettings( ResourceCached *resource )
{
  // FIXME KABC
  //resource->setSaveInterval( d->mIntervalSpin->value() );
  //resource->setSavePolicy( d->mGroup->selectedId() );
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
