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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <typeinfo>

#include <q3header.h>
#include <QLayout>
//Added by qt3to4:
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QBoxLayout>
#include <QCheckBox>
#include <QIntValidator>

#include <kabprefs.h>
#include <kdebug.h>
#include <klocale.h>
#include <krestrictedline.h>
#include <categoryselectdialog.h>

#include "resourcekabc.h"
#include "resourcekabcconfig.h"

using namespace KCal;

ResourceKABCConfig::ResourceKABCConfig( QWidget* parent )
    : KRES::ConfigWidget( parent )
{
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( 6 );
  topLayout->setMargin( 11 );

  mAlarm = new QCheckBox(i18n("Set reminder"), this);
  topLayout->addWidget(mAlarm, 0, 0, 1, 2);

  mALabel = new QLabel(i18n("Reminder before (in days):"),this);
  topLayout->addWidget(mALabel, 1, 0 );
  mAlarmTimeEdit = new KLineEdit(this);
  mAlarmTimeEdit->setValidator(new QIntValidator(mAlarmTimeEdit));
  mAlarmTimeEdit->setText("0");
  topLayout->addWidget(mAlarmTimeEdit, 1, 1 );

  QFrame *line = new QFrame( this );
  line->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  topLayout->addWidget( line, 2, 0, 1, 2 );

  mUseCategories = new QCheckBox( i18n( "Filter by categories" ), this );
  topLayout->addWidget( mUseCategories, 3, 0, 1, 2 );

  KABPrefs *prefs = KABPrefs::instance();
  mCategoryView = new KPIM::CategorySelectWidget(this,prefs);
  mCategoryView->setCategories(prefs->customCategories());
  mCategoryView->hideHeader();
  mCategoryView->hideButton();
  mCategoryView->setEnabled( false );
  topLayout->addWidget( mCategoryView, 4, 0, 1, 2 );

  connect( mUseCategories, SIGNAL( toggled( bool ) ),
           mCategoryView, SLOT( setEnabled( bool ) ) );

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

    const QStringList categories = res->categories();
    mCategoryView->setSelected(categories);
    mUseCategories->setChecked( res->useCategories() );
  } else {
    kDebug(5700) <<"ERROR: ResourceKABCConfig::loadSettings(): no ResourceKABC, cast failed";
  }
}

void ResourceKABCConfig::saveSettings( KRES::Resource *resource )
{
  ResourceKABC *res = static_cast<ResourceKABC *>( resource );
  if ( res ) {
    res->setAlarm( mAlarm->isChecked() );
    res->setAlarmDays( mAlarmTimeEdit->text().toInt() );
    setReadOnly( true );

    QStringList categories;
    QString categoriesStr;
    categories = mCategoryView->selectedCategories(categoriesStr);
    res->setCategories( categories );
    res->setUseCategories( mUseCategories->isChecked() );
  } else {
    kDebug(5700) <<"ERROR: ResourceKABCConfig::saveSettings(): no ResourceKABC, cast failed";
  }
}

void ResourceKABCConfig::alarmClicked()
{
  mAlarmTimeEdit->setDisabled(!mAlarm->isChecked());
  mALabel->setDisabled(!mAlarm->isChecked());
}

#include "resourcekabcconfig.moc"
