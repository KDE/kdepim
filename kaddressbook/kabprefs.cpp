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

KABPrefs *KABPrefs::sInstance = 0;
static KStaticDeleter<KABPrefs> staticDeleter;

KABPrefs::KABPrefs()
  : KPimPrefs("kaddressbookrc")
{
  KPrefs::setCurrentGroup( "Views" );
  addItemBool( "HonorSingleClick", mHonorSingleClick, false );

  KPrefs::setCurrentGroup( "General" );
  addItemBool( "AutomaticNameParsing", mAutomaticNameParsing, true );
  addItemInt( "CurrentIncSearchField", mCurrentIncSearchField, 0 );

  KPrefs::setCurrentGroup( "MainWindow" );
  addItemBool( "JumpButtonBarVisible", mJumpButtonBarVisible, false );
  addItemBool( "DetailsPageVisible", mDetailsPageVisible, true );
  addItemIntList( "ExtensionsSplitter", mExtensionsSplitter );
  addItemIntList( "DetailsSplitter", mDetailsSplitter );

  KPrefs::setCurrentGroup( "Extensions_General" );
  addItemInt( "CurrentExtension", mCurrentExtension, 0 );
  addItemStringList( "ActiveExtensions", mActiveExtensions );

  KPrefs::setCurrentGroup( "Views" );
  QString defaultView = i18n( "Default Table View" );
  addItemString( "CurrentView", mCurrentView, defaultView );
  addItemStringList( "ViewNames", mViewNames, defaultView );

  KPrefs::setCurrentGroup( "Filters" );
  addItemInt( "CurrentFilter", mCurrentFilter, 0 );
}

KABPrefs::~KABPrefs()
{
}

KABPrefs *KABPrefs::instance()
{
  if ( !sInstance ) {
    staticDeleter.setObject( sInstance, new KABPrefs() );
    sInstance->readConfig();
  }

  return sInstance;
}

void KABPrefs::setCategoryDefaults()
{
  mCustomCategories.clear();
  
  mCustomCategories << i18n( "Business" ) << i18n( "Family" ) << i18n( "School" )
                    << i18n( "Customer" ) << i18n( "Friend" );
}
