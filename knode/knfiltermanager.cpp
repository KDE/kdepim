/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <stdlib.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>

#include "utilities.h"
#include "knglobals.h"
#include "knarticlefilter.h"
#include "knfilterdialog.h"
#include "knfiltermanager.h"
#include "knconfig.h"
#include "knconfigwidgets.h"


KNFilterSelectAction::KNFilterSelectAction( const QString& text, const QString& pix,
                                            QObject* parent, const char *name )
 : KActionMenu(text,pix,parent,name), currentItem(-42)
{
  popupMenu()->setCheckable(true);
  connect(popupMenu(),SIGNAL(activated(int)),this,SLOT(slotMenuActivated(int)));
  setDelayed(false);
}



KNFilterSelectAction::~KNFilterSelectAction()
{
}

void KNFilterSelectAction::setCurrentItem(int id)
{
  popupMenu()->setItemChecked(currentItem, false);
  popupMenu()->setItemChecked(id, true);
  currentItem = id;
}


void KNFilterSelectAction::slotMenuActivated(int id)
{
  setCurrentItem(id);
  emit(activated(id));
}


//==============================================================================

KNFilterManager::KNFilterManager(QObject * parent, const char * name)
 : QObject(parent,name), fset(0), currFilter(0), a_ctFilter(0)
{
  loadFilters();

  KConfig *conf=knGlobals.config();
  conf->setGroup("READNEWS");
  setFilter(conf->readNumEntry("lastFilterID", 1));
}



KNFilterManager::~KNFilterManager()
{
  for ( QValueList<KNArticleFilter*>::Iterator it = mFilterList.begin(); it != mFilterList.end(); ++it )
    delete (*it);
}



void KNFilterManager::readOptions()
{
}



void KNFilterManager::saveOptions()
{
}


void KNFilterManager::prepareShutdown()
{
  if (currFilter) {
    KConfig *conf=knGlobals.config();
    conf->setGroup("READNEWS");
    conf->writeEntry("lastFilterID", currFilter->id());
  }
}


void KNFilterManager::loadFilters()
{
  QString fname(locate("data","knode/filters/filters.rc") );

  if (!fname.isNull()) {
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
  QString dir(locateLocal("data","knode/")+"filters/");

  if (dir.isNull()) {
    KNHelper::displayInternalFileError();
    return;
  }
  KSimpleConfig conf(dir+"filters.rc");
  QValueList<int> activeFilters;
  for ( QValueList<KNArticleFilter*>::Iterator it = mFilterList.begin(); it != mFilterList.end(); ++it )
    activeFilters << (*it)->id();

  conf.writeEntry("Active",activeFilters);
  conf.writeEntry("Menu",menuOrder);
}



void KNFilterManager::startConfig(KNConfig::FilterListWidget *fs)
{
  fset=fs;
  commitNeeded = false;

  for ( QValueList<KNArticleFilter*>::Iterator it = mFilterList.begin(); it != mFilterList.end(); ++it )
    fset->addItem( (*it) );

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
  if ( f->id() == -1 ) {      // new filter, find suitable ID
    QValueList<int> activeFilters;
    // ok, this is a ugly hack: we want to reuse old id's, so we try to find the first unused id
    for ( QValueList<KNArticleFilter*>::Iterator it = mFilterList.begin(); it != mFilterList.end(); ++it )
      activeFilters << (*it)->id();
    int newId = 1;
    while ( activeFilters.contains( newId ) > 0 )
      newId++;
    f->setId( newId );
  }
  mFilterList.append( f );
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
  if ( KMessageBox::warningContinueCancel( fset ? fset : knGlobals.topWidget,
       i18n("Do you really want to delete this filter?"), QString::null, KGuiItem( i18n("&Delete"), "editdelete" ) )
       == KMessageBox::Continue ) {
    if ( mFilterList.remove( f ) ) { // does not delete surplus config files
      if ( fset ) {                 // we reuse ids to reduce the number of dead files
        fset->removeItem( f );
        fset->removeMenuItem( f );
      }
      if ( currFilter == f ) {
        currFilter = 0;
        emit filterChanged( currFilter );
      }
    }
  }
}


bool KNFilterManager::newNameIsOK(KNArticleFilter *f, const QString &newName)
{
  for ( QValueList<KNArticleFilter*>::Iterator it = mFilterList.begin(); it != mFilterList.end(); ++it )
    if ( (*it) != f && newName == (*it)->translatedName() )
      return false;

  return true;
}



KNArticleFilter* KNFilterManager::setFilter(const int id)
{
  KNArticleFilter *bak=currFilter;

  currFilter=byID(id);

  if(currFilter) {
    if(a_ctFilter)
      a_ctFilter->setCurrentItem(currFilter->id());
    emit(filterChanged(currFilter));
  } else
    currFilter=bak;

  return currFilter;
}



KNArticleFilter* KNFilterManager::byID(int id)
{
  for ( QValueList<KNArticleFilter*>::Iterator it = mFilterList.begin(); it != mFilterList.end(); ++it )
    if ( (*it)->id() == id )
      return (*it);

  return 0;
}



void KNFilterManager::updateMenu()
{
  if(!a_ctFilter)
    return;

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
    KMessageBox::error(knGlobals.topWidget, i18n("ERROR: no such filter."));
}


void KNFilterManager::slotShowFilterChooser()
{
  KNArticleFilter *f=0;
  QStringList items;
  QValueList<int> ids;

  QValueList<int>::Iterator it = menuOrder.begin();
  while (it != menuOrder.end()) {
    if ((*it)!=-1)
      if ((f=byID((*it)))) {
        items.append(f->translatedName());
        ids.append(*it);
      }
    ++it;
  }

  int currentItem=0;
  if (currFilter)
    currentItem=ids.findIndex(currFilter->id());
  if (currentItem==-1)
    currentItem=0;

  int newFilter = KNHelper::selectDialog(knGlobals.topWidget, i18n("Select Filter"), items, currentItem);
  if (newFilter != -1)
    setFilter(ids[newFilter]);
}


void KNFilterManager::setMenuAction(KNFilterSelectAction *a, KAction *keybA)
{
  if(a) {
    a_ctFilter = a;
    connect(a_ctFilter, SIGNAL(activated(int)), this,  SLOT(slotMenuActivated(int)));
  }
  if(keybA)
    connect(keybA, SIGNAL(activated()), this,  SLOT(slotShowFilterChooser()));

  updateMenu();
}

//--------------------------------

#include "knfiltermanager.moc"
