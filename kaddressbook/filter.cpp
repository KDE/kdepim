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
#include <kdebug.h>

#include "filter.h"

Filter::Filter()
{
  mName = QString::null;
  mEnabled = true;
  mMatchRule = Matching;
}

Filter::Filter(const QString &name)
{
  mName = name;
  mEnabled = true;
  mMatchRule = Matching;
}

Filter::Filter(const Filter &copyFrom)
{
  (*this) = copyFrom;
}

Filter::~Filter()
{
}

Filter &Filter::operator=(const Filter &copyFrom)
{
  if (this == &copyFrom)
    return *this;
    
  mName = copyFrom.mName;
  mEnabled = copyFrom.mEnabled;
  mMatchRule = copyFrom.mMatchRule;
  mCategoryList = copyFrom.mCategoryList;
  
  return *this;
}

void Filter::apply(KABC::Addressee::List &addresseeList)
{
  KABC::Addressee::List::Iterator iter;
  for (iter = addresseeList.begin(); iter != addresseeList.end(); )
  {
    if (filterAddressee(*iter))
      ++iter;
    else
      iter = addresseeList.erase(iter);
  }
}

bool Filter::filterAddressee(const KABC::Addressee &a)
{
  bool matches = true;
  
  QStringList::Iterator iter;
  for (iter = mCategoryList.begin(); (iter != mCategoryList.end()) && matches; 
       ++iter)
  {
    matches = a.hasCategory(*iter);
  }
  
  if (mMatchRule == Matching)
    return matches;
    
  return !matches;
}

void Filter::save(KConfig *config)
{
  config->writeEntry("Name", mName);
  config->writeEntry("Enabled", mEnabled);
  config->writeEntry("Categories", mCategoryList);
  config->writeEntry("MatchRule", (int)mMatchRule);
}

void Filter::restore(KConfig *config)
{
  mName = config->readEntry("Name", "<internal error>");
  mEnabled = config->readBoolEntry("Enabled", true);
  mCategoryList = config->readListEntry("Categories");
  mMatchRule = (MatchRule)config->readNumEntry("MatchRule", Matching);
}

void Filter::save(KConfig *config, QString baseGroup, 
                     Filter::List &list)
{
  {
    KConfigGroupSaver s(config, baseGroup);
    config->writeEntry("Count", list.count());
  }
  
  int index = 0;
  Filter::List::Iterator iter;
  for (iter = list.begin(); iter != list.end(); ++iter)
  {
    {
      KConfigGroupSaver s(config, QString("%1_%2").arg(baseGroup).arg(index));
      (*iter).save(config);
    }
    index++;
  }
}
                     
Filter::List Filter::restore(KConfig *config, QString baseGroup)
{
  Filter::List list;
  int count = 0;
  Filter f;
  
  {
    KConfigGroupSaver s(config, baseGroup);
    count = config->readNumEntry("Count", 0);
  }
  
  for (int i = 0; i < count; i++)
  {
    {
      KConfigGroupSaver s(config, QString("%1_%2").arg(baseGroup).arg(i));
      f.restore(config);
    }
    list.append(f);
  }
  
  return list;
}
