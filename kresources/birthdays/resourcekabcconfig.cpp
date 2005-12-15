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

#include <qheader.h>
#include <qlayout.h>

#include <kabprefs.h>
#include <kdebug.h>
#include <klocale.h>

#include "resourcekabc.h"
#include "resourcekabcconfig.h"

using namespace KCal;

ResourceKABCConfig::ResourceKABCConfig( QWidget* parent,  const char* name )
    : KRES::ConfigWidget( parent, name )
{
  QGridLayout *topLayout = new QGridLayout( this, 5, 1, 11, 6 );

  mAlarm = new QCheckBox(i18n("Set reminder"), this);
  topLayout->addWidget(mAlarm, 0, 0);
  QBoxLayout *alarmLayout = new QHBoxLayout(topLayout);

  mALabel = new QLabel(i18n("Reminder before (in days):"), this);
  alarmLayout->addWidget(mALabel);
  mAlarmTimeEdit = new KRestrictedLine(this, "alarmTimeEdit", "1234567890");
  mAlarmTimeEdit->setText("0");
  alarmLayout->addWidget(mAlarmTimeEdit);

  QFrame *line = new QFrame( this );
  line->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  topLayout->addMultiCellWidget( line, 2, 2, 0, 1 );

  mUseCategories = new QCheckBox( i18n( "Filter by categories" ), this );
  topLayout->addMultiCellWidget( mUseCategories, 3, 3, 0, 1 );

  mCategoryView = new KListView( this );
  mCategoryView->addColumn( "" );
  mCategoryView->header()->hide();
  mCategoryView->setEnabled( false );
  topLayout->addMultiCellWidget( mCategoryView, 4, 4, 0, 1 );

  connect( mUseCategories, SIGNAL( toggled( bool ) ),
           mCategoryView, SLOT( setEnabled( bool ) ) );

  mAlarmTimeEdit->setDisabled(true);
  mALabel->setDisabled(true);

  connect(mAlarm, SIGNAL(clicked()), SLOT(alarmClicked()));

  setReadOnly( true );

  KABPrefs *prefs = KABPrefs::instance();
  const QStringList categories = prefs->customCategories();
  QStringList::ConstIterator it;
  for ( it = categories.begin(); it != categories.end(); ++it )
    new QCheckListItem( mCategoryView, *it, QCheckListItem::CheckBox );
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
    QListViewItemIterator it( mCategoryView );
    while ( it.current() ) {
      if ( categories.contains( it.current()->text( 0 ) ) ) {
        QCheckListItem *item = static_cast<QCheckListItem*>( it.current() );
        item->setOn( true );
      }
      ++it;
    }

    mUseCategories->setChecked( res->useCategories() );
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

    QStringList categories;
    QListViewItemIterator it( mCategoryView, QListViewItemIterator::Checked );
    while ( it.current() ) {
      categories.append( it.current()->text( 0 ) );
      ++it;
    }
    res->setCategories( categories );
    res->setUseCategories( mUseCategories->isChecked() );
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
