/***************************************************************************
                          knfiltermanager.cpp  -  description
                             -------------------

    copyright            : (C) 1999 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <stdlib.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qbitarray.h>

#include <ktabctl.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kpopupmenu.h>
#include <ktoolbarbutton.h>
#include <kiconloader.h>
#include <ktoolbar.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include "utilities.h"
#include "knglobals.h"
#include "knstatusfilter.h"
#include "knrangefilter.h"
#include "knstringfilter.h"
#include "knarticlefilter.h"
#include "knfilterdialog.h"
#include "knfiltermanager.h"
#include "knodeview.h"
#include "knconfig.h"



KNFilterManager::KNFilterManager(KNFilterSelectAction *a, QObject * parent, const char * name)
 : QObject(parent,name), fset(0), currFilter(0), a_ctFilter(a)
{
  fList.setAutoDelete(true);

  connect(a_ctFilter, SIGNAL(activated(int)), this,  SLOT(slotMenuActivated(int)));
  loadFilters();

  KConfig *conf=KGlobal::config();
  conf->setGroup("READNEWS");
  setFilter(conf->readNumEntry("lastFilterID", 1));
}



KNFilterManager::~KNFilterManager()
{
  if (currFilter) {
    KConfig *conf=KGlobal::config();
    conf->setGroup("READNEWS");
    conf->writeEntry("lastFilterID", currFilter->id());
  }
}



void KNFilterManager::readOptions()
{

}



void KNFilterManager::saveOptions()
{

}



void KNFilterManager::loadFilters()
{
  QString fname(KGlobal::dirs()->findResource("appdata","filters/filters.rc"));
  if (fname != QString::null) {
    KSimpleConfig conf(fname,true);
  
    QValueList<int> activeFilters = conf.readIntListEntry("Active");
    menuOrder = conf.readIntListEntry("Menu");
  
    QValueList<int>::Iterator it = activeFilters.begin();
    while (it != activeFilters.end()) {
      KNArticleFilter *f=new KNArticleFilter((*it));
      if (f->loadInfo())
        addFilter(f);
      else
        delete f; 
      it++;
    } 
  }
  updateMenu(); 
}



void KNFilterManager::saveFilterLists()
{
  QString dir(KGlobal::dirs()->saveLocation("appdata","filters/"));
  if (dir==QString::null) {
    displayInternalFileError();
    return;
  }
  KSimpleConfig conf(dir+"filters.rc");
  QValueList<int> activeFilters;
  QListIterator<KNArticleFilter> it(fList);
  for ( ; it.current(); ++it )
    activeFilters << it.current()->id();

  conf.writeEntry("Active",activeFilters);
  conf.writeEntry("Menu",menuOrder);
}



void KNFilterManager::startConfig(KNConfig::FilterListWidget *fs)
{
  fset=fs;
  commitNeeded = false;

  for(KNArticleFilter *f=fList.first(); f; f=fList.next())
    fset->addItem(f); 
  
  QValueList<int>::Iterator it = menuOrder.begin();
  while (it != menuOrder.end()) {
    if ((*it)!=-1)
      fset->addMenuItem(byID((*it)));
    else
      fset->addMenuItem(0);
    ++it;
  }
}



void KNFilterManager::endConfig()
{
  fset=0;
}



void KNFilterManager::commitChanges()
{
  menuOrder = fset->menuOrder();
  saveFilterLists();

  if(currFilter)
    if(!currFilter->isEnabled()) currFilter=0;

  updateMenu();

  if (commitNeeded)
    emit filterChanged(currFilter);
}



void KNFilterManager::newFilter()
{
  KNArticleFilter *f=new KNArticleFilter();
  editFilter(f);
}



void KNFilterManager::addFilter(KNArticleFilter *f)
{
  if(f->id()==-1) {      // new filter, find suitable ID
    QValueList<int> activeFilters;
    QListIterator<KNArticleFilter> it(fList);  // ok, this is a ugly hack:
    for ( ; it.current(); ++it )               // we want to reuse old id's, so we
      activeFilters << it.current()->id();     // try to find the first unused id.
    int newId = 1;
    while (activeFilters.contains(newId)>0)
      newId++;
    f->setId(newId);
  }
  fList.append(f);
}



void KNFilterManager::editFilter(KNArticleFilter *f)
{
  if (!f->loaded() && f->id()!=-1)
    f->load();

  KNFilterDialog *fdlg=new KNFilterDialog(f,(fset)? fset:knGlobals.topWidget);
  
  if (fdlg->exec()) {
    commitNeeded = true;
    if(f->id()==-1) {  // new filter
      addFilter(f);
      f->setLoaded(true);
      if(fset) {           // updating settings tab
        fset->addItem(f);
        if(f->isEnabled())
          fset->addMenuItem(f);
      }
    } else {
      if(fset) {          // updating settings tab
        if(f->isEnabled())
          fset->addMenuItem(f);
        else
          fset->removeMenuItem(f);
        fset->updateItem(f);
      }
    }
    f->save();      
  } else {
    if(f->id()==-1)   // new filter
      delete f;
  }
    
  delete fdlg;
}


void KNFilterManager::copyFilter(KNArticleFilter *f)
{
  if (!f->loaded())
    f->load();
  KNArticleFilter *newf=new KNArticleFilter(*f);
  editFilter(newf);
}


void KNFilterManager::deleteFilter(KNArticleFilter *f)
{
  if(KMessageBox::questionYesNo((fset)? fset:knGlobals.topWidget,i18n("Do you really want to delete this filter?"))==KMessageBox::Yes) {
    if(fList.removeRef(f)) {     // does not delete surplus config files
      if(fset) {                  // we reuse ids to reduce the number of dead files
        fset->removeItem(f);
        fset->removeMenuItem(f);
      }
      if(currFilter==f) {
        currFilter=0;
        emit filterChanged(currFilter);
      }
    }
  }
}


bool KNFilterManager::newNameIsOK(KNArticleFilter *f, const QString &newName)
{
  KNArticleFilter *var=fList.first();
  bool found=false;
  
  while(var && !found) {
    if(var!=f) found=(newName==var->translatedName());
    var=fList.next();
  }

  return (!found);
}



KNArticleFilter* KNFilterManager::setFilter(const int id)
{
  KNArticleFilter *bak=currFilter;
  
  currFilter=byID(id);  
  
  if(currFilter) {
    a_ctFilter->setCurrentItem(currFilter->id());
    emit(filterChanged(currFilter));
  } else
    currFilter=bak;
  
  return currFilter;    
}



KNArticleFilter* KNFilterManager::byID(int id)
{
  KNArticleFilter *ret=0;
  
  for(ret=fList.first(); ret; ret=fList.next())
    if(ret->id()==id) break;
    
  return ret; 
}



void KNFilterManager::updateMenu()
{
  a_ctFilter->popupMenu()->clear();
  KNArticleFilter *f=0;

  QValueList<int>::Iterator it = menuOrder.begin();
  while (it != menuOrder.end()) {
    if ((*it)!=-1) {
      if ((f=byID((*it))))
        a_ctFilter->popupMenu()->insertItem(f->translatedName(), f->id());
    }
    else a_ctFilter->popupMenu()->insertSeparator();
    ++it;
  }
  
  if(currFilter)
    a_ctFilter->setCurrentItem(currFilter->id());
}



void KNFilterManager::slotMenuActivated(int id)
{
  KNArticleFilter *f=setFilter(id);

  if (!f)
    KMessageBox::error(knGlobals.topWidget, i18n("ERROR : no such filter!"));
}


//--------------------------------

#include "knfiltermanager.moc"
