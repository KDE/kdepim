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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <kconfig.h>
#include <kapplication.h>
#include <klocale.h>

#include "kabprefs.h"

KABPrefs *KABPrefs::sInstance = 0;

KABPrefs::KABPrefs()
{
  mConfig = kapp->config();
  
  load();
}

KABPrefs::~KABPrefs()
{
}

KABPrefs *KABPrefs::instance()
{
  if (sInstance == 0)
    sInstance = new KABPrefs();

  return sInstance;
}

void KABPrefs::writeConfig()
{
  save();
}

void KABPrefs::load()
{
  {
    KConfigGroupSaver s(mConfig, "General");
    mCategoryList = mConfig->readListEntry("Categories");
    if (mCategoryList.count() == 0)
      mCategoryList << i18n("Business") << i18n("Family") << i18n("School")
                    << i18n("Customer") << i18n("Friend");
  }
  
  {
    KConfigGroupSaver s(mConfig, "Views");
    mHonorSingleClick = mConfig->readBoolEntry("HonorSingleClick", false);
  }
}

void KABPrefs::save()
{
  {
    KConfigGroupSaver(mConfig, "General");
    mConfig->writeEntry("Categories", mCategoryList);
  }
  {
    KConfigGroupSaver(mConfig, "Views");
    mConfig->writeEntry("HonorSingleClick", mHonorSingleClick);
  }
}

