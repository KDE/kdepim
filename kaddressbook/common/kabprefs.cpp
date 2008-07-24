/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kconfig.h>
#include <klocale.h>
#include <kstaticdeleter.h>

#include "kabprefs.h"

KABPrefs *KABPrefs::mInstance = 0;
static KStaticDeleter<KABPrefs> staticDeleter;

KABPrefs::KABPrefs()
  : KABPrefsBase()
{
  KConfigSkeleton::setCurrentGroup( "General" );

  QStringList defaultMap;
  defaultMap << "http://maps.google.com/maps?f=q&hl=%1&q=%n,%l,%s";
  addItemString( "LocationMapURL", mLocationMapURL, defaultMap[ 0 ] );
  addItemStringList( "LocationMapURLs", mLocationMapURLs, defaultMap );
}

KABPrefs::~KABPrefs()
{
}

KABPrefs *KABPrefs::instance()
{
  if ( !mInstance ) {
    staticDeleter.setObject( mInstance, new KABPrefs() );
    mInstance->readConfig();
  }

  return mInstance;
}

void KABPrefs::setCategoryDefaults()
{
  mCustomCategories.clear();
  mCustomCategories << i18n( "Business" ) << i18n( "Family" ) << i18n( "School" )
                    << i18n( "Customer" ) << i18n( "Friend" );
}

void KABPrefs::usrReadConfig()
{
  config()->setGroup( "General" );
  mCustomCategories = config()->readListEntry( "Custom Categories" );
  if ( mCustomCategories.isEmpty() )
    setCategoryDefaults();

  KPimPrefs::usrReadConfig();
}


void KABPrefs::usrWriteConfig()
{
  config()->setGroup( "General" );
  config()->writeEntry( "Custom Categories", mCustomCategories );

  KPimPrefs::usrWriteConfig();
}
