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
#include "knfiltersettings.h"
#include "knfiltermanager.h"


KNFilterSelectAction::KNFilterSelectAction( const QString& text, const QString& pix,
                                            int accel, QObject* parent, const char* name )
 : KAction(text,pix,accel,parent,name), currentItem(-42)
{
  p_opup = new KPopupMenu;
  p_opup->setCheckable(true);
  connect(p_opup,SIGNAL(activated(int)),this,SLOT(slotMenuActivated(int)));
}



KNFilterSelectAction::~KNFilterSelectAction()
{
  delete p_opup;
}



int KNFilterSelectAction::plug(QWidget* widget, int index)
{
  if ( widget->inherits("QPopupMenu") ) {
    QPopupMenu* menu = static_cast<QPopupMenu*>( widget );
    int id;
    if ( hasIconSet() )
      id = menu->insertItem( iconSet(), text(), p_opup, -1, index );
    else
      id = menu->insertItem( text(), p_opup, -1, index );

    menu->setItemEnabled( id, isEnabled() );
    menu->setWhatsThis( id, whatsThis() );

    addContainer( menu, id );
    connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
    return containerCount() - 1;
  }
  else if ( widget->inherits("KToolBar"))  {
    KToolBar *bar = static_cast<KToolBar *>( widget );

    int id_ = getToolButtonID();
    bar->insertButton( icon(), id_, isEnabled(), plainText(), index);

    KToolBarButton *btn = bar->getButton(id_);
    btn->setPopup(p_opup);

    QWhatsThis::add( btn, whatsThis() );
    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  kdDebug(5003) << "Can not plug KFilterSelectAction in " << widget->className() << endl;
  return -1;
}



void KNFilterSelectAction::setCurrentItem(int id)
{
  p_opup->setItemChecked(currentItem, false);
  p_opup->setItemChecked(id, true);
  currentItem = id;
}


void KNFilterSelectAction::setEnabled(bool b)
{
  p_opup->hide();
  KAction::setEnabled(b);
}


void KNFilterSelectAction::slotMenuActivated(int id)
{
  setCurrentItem(id);
  emit(activated(id));
}



//=========================================================================================



KNFilterManager::KNFilterManager( QObject * parent, const char * name)
 : QObject(parent,name), fset(0), currFilter(0), isAGroup(false)
{
  fList.setAutoDelete(true);

  actFilter = new KNFilterSelectAction(i18n("&Filter"), "filter", 0 , &actionCollection, "view_Filter");
  connect(actFilter, SIGNAL(activated(int)), this,  SLOT(slotMenuActivated(int)));
  actFilter->setEnabled(false);

  loadFilters();  
  readOptions();  
}



KNFilterManager::~KNFilterManager()
{
  actFilter->unplugAll();
}



void KNFilterManager::readOptions()
{
  KConfig *conf=KGlobal::config();    
  conf->setGroup("READNEWS");
  setFilter(conf->readNumEntry("lastFilterID", 1));
}



void KNFilterManager::saveOptions()
{
  if (currFilter) {
    KConfig *conf=KGlobal::config();    
    conf->setGroup("READNEWS");
    conf->writeEntry("lastFilterID", currFilter->id());
  }
}



// dis-/enable the filter menu
void KNFilterManager::setIsAGroup(bool b)
{
  isAGroup = b;
  actFilter->setEnabled(isAGroup && (menuOrder.count()));
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



void KNFilterManager::startConfig(KNFilterSettings *fs)
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
    actFilter->setCurrentItem(currFilter->id());
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
  actFilter->popupMenu()->clear();
  KNArticleFilter *f=0;

  QValueList<int>::Iterator it = menuOrder.begin();
  while (it != menuOrder.end()) {
    if ((*it)!=-1) {
      if ((f=byID((*it))))
        actFilter->popupMenu()->insertItem(f->translatedName(), f->id());
    }
    else actFilter->popupMenu()->insertSeparator();
    ++it;
  }
  
  if(currFilter)
    actFilter->setCurrentItem(currFilter->id());
  actFilter->setEnabled(isAGroup && (menuOrder.count()));
}



void KNFilterManager::slotMenuActivated(int id)
{
  KNArticleFilter *f=setFilter(id);

  if (!f)
    KMessageBox::error(knGlobals.topWidget, i18n("ERROR : no such filter!"));
}


//--------------------------------

#include "knfiltermanager.moc"
