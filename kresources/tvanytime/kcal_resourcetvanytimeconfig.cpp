/*
    This file is part of kdepim.

    Copyright (c) 2005 Will Stephenson <wstephenson@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <typeinfo>

#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

#include <kconfigskeleton.h>
#include <klocale.h>
#include <kdebug.h>
//#include <kdialogbase.h>
//#include <kstandarddirs.h>
#include <klineedit.h>

#include <libkcal/resourcecachedconfig.h>

#include "kcal_resourcetvanytime.h"
#include "kcal_tvanytimeprefs.h"
#include "kcal_resourcetvanytimeconfig.h"

using namespace KCal;

ResourceTVAnytimeConfig::ResourceTVAnytimeConfig( QWidget* parent,  const char* name )
    : KRES::ConfigWidget( parent, name )
{
  resize( 245, 115 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );

  QLabel *label = new QLabel( i18n( "Schedule tarball URL:" ), this );
  mainLayout->addWidget( label, 1, 0 );
  mUrl = new KLineEdit( this );
  mainLayout->addWidget( mUrl, 1, 1 );
  label = new QLabel( i18n( "Retrieve how many days?" ), this );
  mainLayout->addWidget( label, 2, 0 );
  mDays = new QSpinBox( this );
  mainLayout->addWidget( mDays, 2, 1 );
  mReloadConfig = new KCal::ResourceCachedReloadConfig( this );
  mainLayout->addMultiCellWidget( mReloadConfig, 3, 3, 0, 1 );

}

void ResourceTVAnytimeConfig::loadSettings( KRES::Resource *resource )
{
  kdDebug() << "KCal::ResourceTVAnytimeConfig::loadSettings()" << endl;
  ResourceTVAnytime *res = static_cast<ResourceTVAnytime *>( resource );
  mResource = res;
  
  if ( res ) {
    if ( !res->prefs() ) {
      kdError() << "No PREF" << endl;
      return;
    }
    KConfigSkeleton::ItemInt * daysItem = res->prefs()->daysItem();
    mDays->setMinValue( daysItem->minValue().toInt() );
    mDays->setMaxValue( daysItem->maxValue().toInt() );
    QWhatsThis::add( mDays, daysItem->whatsThis() );
    mUrl->setText( res->prefs()->url() );
    mDays->setValue( res->prefs()->days() );
    mReloadConfig->loadSettings( res );
  } else {
    kdError(5700) << "KCalResourceTVAnytimeConfig::loadSettings(): no KCalResourceTVAnytime, cast failed" << endl;
  }
}

void ResourceTVAnytimeConfig::saveSettings( KRES::Resource *resource )
{
  ResourceTVAnytime *res = static_cast<ResourceTVAnytime*>( resource );
  if ( res ) {
    res->prefs()->setUrl( mUrl->text() );
    res->prefs()->setDays( mDays->value() );
    mReloadConfig->saveSettings( res );
  } else {
    kdError(5700) << "KCalResourceTVAnytimeConfig::saveSettings(): no KCalResourceTVAnytime, cast failed" << endl;
  }
}

#include "kcal_resourcetvanytimeconfig.moc"
