/*
    This file is part of the KDE alarm daemon.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

// $Id$

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kdialog.h>

#include "alarmdaemoniface_stub.h"
#include "alarmdaemonctrl.moc"

extern "C" {
  KCModule *create_alarmdaemonctrl(QWidget *parent, const char *name) {
    KGlobal::locale()->insertCatalogue("alarmdaemonctrl");
    return new AlarmDaemonCtrl(parent, "alarmdaemonctrl");
  }
}


AlarmDaemonCtrl::AlarmDaemonCtrl(QWidget *parent, const char *name) :
  KCModule(parent, name)
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );

  mAutoStartCheck = new QCheckBox(
      i18n("Start alarm daemon automatically at login"), this );
  connect( mAutoStartCheck, SIGNAL( clicked() ), SLOT( changed() ) );
  QWhatsThis::add( mAutoStartCheck,
      i18n("Check to start the alarm daemon whenever you start a KDE session."));
  topLayout->addWidget( mAutoStartCheck );
  
  QBoxLayout *intervalLayout = new QHBoxLayout( topLayout );
 
  QLabel *intervalLabel = new QLabel( i18n("Check interval [minutes]"), this );
  intervalLayout->addWidget( intervalLabel );

  mIntervalSpin = new QSpinBox( this );
  mIntervalSpin->setMinValue( 1 );
  connect( mIntervalSpin, SIGNAL( valueChanged( int ) ), SLOT( changed() ) );
  QWhatsThis::add( mIntervalSpin,
      i18n("How often (in minutes) the alarm daemon should check for alarms becoming due."));
  intervalLayout->addWidget( mIntervalSpin );
  
  topLayout->addStretch();
  
  load();
}

AlarmDaemonCtrl::~AlarmDaemonCtrl()
{
}

void AlarmDaemonCtrl::defaults()
{
  mAutoStartCheck->setChecked( true );
  mIntervalSpin->setValue( 1 );

  emit changed(true);
}

QString AlarmDaemonCtrl::quickHelp() const
{
  return i18n( "<h1>Alarm Daemon</h1> This module allows you to configure"
               " the KDE alarm daemon.");
}

void AlarmDaemonCtrl::load()
{
  kdDebug() << "AlarmDaemonCtrl::load()" << endl;

  KConfig config( locate( "config", "kalarmdrc" ) );
  config.setGroup("General");
  mAutoStartCheck->setChecked( config.readBoolEntry( "Autostart", true ) );
  mIntervalSpin->setValue( config.readNumEntry( "CheckInterval", 1 ) );
  
  emit changed(false);
}

void AlarmDaemonCtrl::save()
{
  kdDebug() << "AlarmDaemonCtrl::save()" << endl;

  KConfig config( locate( "config", "kalarmdrc" ) );
  config.setGroup("General");
  config.writeEntry( "Autostart", mAutoStartCheck->isChecked() );
  config.writeEntry( "CheckInterval", mIntervalSpin->value() );
  config.sync();

  // Notify the alarm daemon that the settings have changed
  AlarmDaemonIface_stub s("kalarmd", "ad");
  s.readConfig();

  emit changed(false);
}

int AlarmDaemonCtrl::buttons()
{
  return KCModule::Help | KCModule::Default | KCModule::Apply;
}

void AlarmDaemonCtrl::changed()
{
  emit changed( true );
}
