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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
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
  defaultMap << "http://www.maporama.com/share/map.asp?COUNTRYCODE=%c&_XgoGCAddress=%s&Zip=%z&State=%r&_XgoGCTownName=%l";
  defaultMap << "http://link2.map24.com/?lid=9cc343ae&maptype=CGI&lang=%1&street0=%s&zip0=%z&city0=%l&country0=%c";
  defaultMap << "http://www.mapquest.com/main.adp?searchtab=address&searchtype=address&country=%c&address=%s&state=%r&zipcode=%z&city=%l&search=1";
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

  qDebug( "categories set" );

  KPimPrefs::usrReadConfig();
}


void KABPrefs::usrWriteConfig()
{
  config()->setGroup( "General" );
  config()->writeEntry( "Custom Categories", mCustomCategories );

  KPimPrefs::usrWriteConfig();
}
