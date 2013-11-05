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

#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kicon.h>

#include "utilities.h"
#include "knglobals.h"
#include "knarticlefilter.h"
#include "knfilterdialog.h"
#include "knfiltermanager.h"
#include "knconfig.h"
#include "knconfigwidgets.h"


KNFilterSelectAction::KNFilterSelectAction( const QString& text, const QString& pix,
                                            KActionCollection* parent, const char *name )
  : KActionMenu(text, parent), currentItem(-42)
{
  setIcon(KIcon(pix));
  menu()->setCheckable(true);
  connect(menu(),SIGNAL(activated(int)),this,SLOT(slotMenuActivated(int)));
  setDelayed(false);
  parent->addAction(name, this);
}



KNFilterSelectAction::~KNFilterSelectAction()
{
}

void KNFilterSelectAction::setCurrentItem(int id)
{
  menu()->setItemChecked(currentItem, false);
  menu()->setItemChecked(id, true);
  currentItem = id;
}


void KNFilterSelectAction::slotMenuActivated(int id)
{
  setCurrentItem(id);
  emit(activated(id));
}


//==============================================================================

KNFilterManager::KNFilterManager( QObject * parent )
 : QObject( parent ), fset(0), currFilter(0), a_ctFilter(0)
{
  loadFilters();

  KConfigGroup conf(knGlobals.config(), "READNEWS");
  setFilter(conf.readEntry("lastFilterID", 1));
}



KNFilterManager::~KNFilterManager()
{
  qDeleteAll( mFilterList );
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
    KConfigGroup group = conf->group("READNEWS");
    group.writeEntry("lastFilterID", currFilter->id());
  }
}


void KNFilterManager::loadFilters()
{
  QString fname( KStandardDirs::locate( "data","knode/filters/filters.rc" ) );

  if (!fname.isNull()) {
    KConfig conf( fname, KConfig::SimpleConfig);
    KConfigGroup grp( &conf, QString());
    QList<int> activeFilters = grp.readEntry("Active",QList<int>());
    menuOrder = grp.readEntry("Menu",QList<int>());

    foreach ( int filterId, activeFilters ) {
      KNArticleFilter *f = new KNArticleFilter( filterId );
      if ( f->loadInfo() )
        addFilter( f );
      else
        delete f;
    }
  }
  updateMenu();
}



void KNFilterManager::saveFilterLists()
{
  QString dir( KStandardDirs::locateLocal( "data","knode/filters/" ) );

  if (dir.isNull()) {
    KNHelper::displayInternalFileError();
    return;
  }
  KConfig conf(dir+"filters.rc", KConfig::SimpleConfig);
  KConfigGroup grp(&conf, QString());
  QList<int> activeFilters;
  foreach ( KNArticleFilter *filter, mFilterList )
    activeFilters << filter->id();

  grp.writeEntry("Active",activeFilters);
  grp.writeEntry("Menu",menuOrder);
}



void KNFilterManager::startConfig(KNode::FilterListWidget *fs)
{
  fset=fs;
  commitNeeded = false;

  foreach ( KNArticleFilter* filter, mFilterList )
    fset->addItem( filter );

  foreach ( int id, menuOrder ) {
    if (id!=-1)
      fset->addMenuItem(byID(id));
    else
      fset->addMenuItem(0);
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
    QList<int> activeFilters;
    // ok, this is a ugly hack: we want to reuse old id's, so we try to find the first unused id
    foreach ( KNArticleFilter *filter, mFilterList )
      activeFilters << filter->id();
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
       i18n("Do you really want to delete this filter?"), QString(), KGuiItem( i18n("&Delete"), "edit-delete" ) )
       == KMessageBox::Continue ) {
    if ( mFilterList.removeAll( f ) ) { // does not delete surplus config files
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
  foreach ( KNArticleFilter *filter, mFilterList )
    if ( filter != f && newName == filter->translatedName() )
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
  foreach ( KNArticleFilter *filter, mFilterList )
    if ( filter->id() == id )
      return filter;

  return 0;
}



void KNFilterManager::updateMenu()
{
  if(!a_ctFilter)
    return;

  a_ctFilter->menu()->clear();
  KNArticleFilter *f=0;

  foreach ( int id, menuOrder ) {
    if ( id != -1 ) {
      if ( ( f = byID( id ) ) )
        a_ctFilter->menu()->insertItem( f->translatedName(), f->id() );
    } else
      a_ctFilter->menu()->addSeparator();
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
  QList<int> ids;

  foreach ( int id, menuOrder ) {
    if ( id != -1 )
      if ( ( f = byID( id ) ) ) {
        items.append( f->translatedName() );
        ids.append( id );
      }
  }

  int currentItem=0;
  if (currFilter)
    currentItem=ids.indexOf(currFilter->id());
  if (currentItem==-1)
    currentItem=0;

  int newFilter = KNHelper::selectDialog(knGlobals.topWidget, i18n("Select Filter"), items, currentItem);
  if (newFilter != -1)
    setFilter(ids[newFilter]);
}


void KNFilterManager::setMenuAction(KNFilterSelectAction *a, QAction *keybA)
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

