/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <typeinfo>

#include <qlayout.h>

#include <klocale.h>
#include <kdebug.h>

#include "resourcekabc.h"
#include "resourcekabcconfig.h"

using namespace KCal;

ResourceKABCConfig::ResourceKABCConfig( QWidget* parent,  const char* name )
    : KRES::ResourceConfigWidget( parent, name )
{
  QGridLayout *topLayout = new QGridLayout(this, 2, 1);

  mAlarm = new QCheckBox(i18n("Set alarm"), this);
  topLayout->addWidget(mAlarm, 0, 0);
  QBoxLayout *alarmLayout = new QHBoxLayout(topLayout);
  topLayout->addLayout(alarmLayout, 1, 0);

  mALabel = new QLabel(i18n("Alarm before (in days):"), this);
  alarmLayout->addWidget(mALabel);
  mAlarmTimeEdit = new KRestrictedLine(this, "alarmTimeEdit", "1234567890");
  mAlarmTimeEdit->setText("0");
  alarmLayout->addWidget(mAlarmTimeEdit);

  mAlarmTimeEdit->setDisabled(true);
  mALabel->setDisabled(true);

  connect(mAlarm, SIGNAL(clicked()), SLOT(alarmClicked()));

  setReadOnly( true );
}

void ResourceKABCConfig::loadSettings( KRES::Resource *resource )
{
  ResourceKABC *res = static_cast<ResourceKABC *>( resource );
  if ( res ) {
    mAlarm->setChecked( res->alarm() );
    QString days;
    mAlarmTimeEdit->setText( days.setNum(res->alarmDays()) );

    mAlarmTimeEdit->setEnabled( res->alarm() );
    mALabel->setEnabled( res->alarm() );
  } else {
    kdDebug(5700) << "ERROR: ResourceKABCConfig::loadSettings(): no ResourceKABC, cast failed" << endl;
  }
}

void ResourceKABCConfig::saveSettings( KRES::Resource *resource )
{
  ResourceKABC *res = static_cast<ResourceKABC *>( resource );
  if ( res ) {
    res->setAlarm( mAlarm->isChecked() );
    res->setAlarmDays( mAlarmTimeEdit->text().toInt() );
    setReadOnly( true );
  } else {
    kdDebug(5700) << "ERROR: ResourceKABCConfig::saveSettings(): no ResourceKABC, cast failed" << endl;
  }
}

void ResourceKABCConfig::alarmClicked()
{
  mAlarmTimeEdit->setDisabled(!mAlarm->isChecked());
  mALabel->setDisabled(!mAlarm->isChecked());
}

#include "resourcekabcconfig.moc"
