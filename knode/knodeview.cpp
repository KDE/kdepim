/***************************************************************************
                     knodeview.cpp - description
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

#include <qheader.h>
#include <stdlib.h>

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kapp.h>

#include "knode.h"
#include "knappmanager.h"
#include "knfoldermanager.h"
#include "knarticlewidget.h"
#include "knarticlecollection.h"
#include "kncollectionviewitem.h"
#include "knhdrviewitem.h"
#include "knfocuswidget.h"
#include "knfetcharticle.h"
#include "knglobals.h"
#include "knodeview.h"


KNodeView::KNodeView(KActionCollection* actColl, QWidget *parent, const char * name)
  : QSplitter(parent,name), longView(true), actionCollection(actColl), notAFolder(true)
{
  setOpaqueResize(true);
  colFocus=new KNFocusWidget(this,"colFocus");
  collectionView=new KNListView(colFocus,"collectionView");
  colFocus->setWidget(collectionView);
  setResizeMode(colFocus, QSplitter::KeepSize);
  
  secSplitter=new QSplitter(QSplitter::Vertical,this,"secSplitter");
  secSplitter->setOpaqueResize(true);

  hdrFocus=new KNFocusWidget(secSplitter,"hdrFocus");
  hdrView=new KNListView(hdrFocus,"hdrViewFocus");
  hdrFocus->setWidget(hdrView);
  secSplitter->setResizeMode(hdrFocus, QSplitter::KeepSize);

  artFocus=new KNFocusWidget(secSplitter,"artFocus");
  artView=new KNArticleWidget(actionCollection, artFocus,"artView");
  artFocus->setWidget(artView);
  
  initCollectionView();
  initHdrView();
  
  setTabOrder(hdrView, artView);
  setTabOrder(artView, collectionView);

  actSortSelect = new KSelectAction(i18n("&Sort"), 0, actionCollection, "view_Sort");
  connect(actSortSelect, SIGNAL(activated(int)), this, SLOT(slotSortMenuSelect(int)));
  connect(hdrView, SIGNAL(sortingChanged(int)), this, SLOT(slotSortHdrSelect(int)));
  QStringList items;
  items += i18n("By &Subject");
  items += i18n("By S&ender");
  items += i18n("By S&core");
  items += i18n("By &Date");
  actSortSelect->setItems(items);
    
  actNextArt = new KAction(i18n("&Next article"), "next", Key_N , this, SLOT(slotNextArticle()),
                           actionCollection, "go_nextArticle");
  actPrevArt = new KAction(i18n("&Previous article"), "previous", Key_B , this, SLOT(slotPrevArticle()),
                           actionCollection, "go_prevArticle");
  actNextUnreadArt = new KAction(i18n("Next unread &article"), "1rightarrow", ALT+Key_Space , this, SLOT(slotNextUnreadArticle()),
                                 actionCollection, "go_nextUnreadArticle");
  actReadThrough = new KAction(i18n("Read &through articles"), Key_Space , this, SLOT(slotReadThrough()),
                               actionCollection, "go_readThrough");
  actNextUnreadThread =  new KAction(i18n("Next unread &thread"),"2rightarrow", CTRL+Key_Space , this, SLOT(slotNextUnreadThread()),
                                     actionCollection, "go_nextUnreadThread");
  actNextGroup = new KAction(i18n("Ne&xt group"), "down", Key_Plus , this, SLOT(slotNextGroup()),
                             actionCollection, "go_nextGroup");
  actPrevGroup = new KAction(i18n("Pre&vious group"), "up", Key_Minus , this, SLOT(slotPrevGroup()),
                             actionCollection, "go_prevGroup");
  actToggleThread = new KAction(i18n("&Toggle Subthread"), Key_T, this, SLOT(slotToggleThread()),
                                actionCollection, "thread_toggle");
  actToggleThread->setEnabled(false);

  readOptions();

  updateAppearance();
}



KNodeView::~KNodeView()
{
}



void KNodeView::readOptions()
{
  KConfig *conf=KGlobal::config();    
  conf->setGroup("APPEARANCE");

  QValueList<int> lst = conf->readIntListEntry("Vert_SepPos");
  if (lst.count()!=2)
    lst << 266 << 487;
  if (longView)
    setSizes(lst);
  else
    secSplitter->setSizes(lst);

  lst = conf->readIntListEntry("Horz_SepPos");
  if (lst.count()!=2)
    lst << 153 << 234;
  if (longView)
    secSplitter->setSizes(lst);
  else
    setSizes(lst);

  lst = conf->readIntListEntry("Hdrs_Size");
  if (lst.count()==7) {
    QValueList<int>::Iterator it = lst.begin();

    QHeader *h=collectionView->header();
    for (int i=0; i<3; i++) {
      h->resizeSection(i,(*it));
      ++it;
    }
  
    h=hdrView->header();
    for (int i=0; i<4; i++) {
      h->resizeSection(i,(*it));
      ++it;
    }
  }

  int sortCol = conf->readNumEntry("sortCol",3);
  bool sortAsc = conf->readBoolEntry("sortAscending", false);
  hdrView->setColAsc(sortCol, sortAsc); 
  hdrView->setSorting(sortCol, sortAsc);
  actSortSelect->setCurrentItem(sortCol);

  sortCol = conf->readNumEntry("account_sortCol", 0);
  sortAsc = conf->readBoolEntry("account_sortAscending", true);
  collectionView->setColAsc(sortCol, sortAsc);
  collectionView->setSorting(sortCol, sortAsc);
}



void KNodeView::saveOptions()
{
  KConfig *conf=KGlobal::config();    
  conf->setGroup("APPEARANCE");

  if (longView) {
    conf->writeEntry("Vert_SepPos",sizes());
    conf->writeEntry("Horz_SepPos",secSplitter->sizes());
  } else {
    conf->writeEntry("Vert_SepPos",secSplitter->sizes());
    conf->writeEntry("Horz_SepPos",sizes());
  }

  QValueList<int> lst;
  QHeader *h=collectionView->header();
  for (int i=0; i<3; i++)
    lst << h->sectionSize(i);

  h=hdrView->header();
  for (int i=0; i<4; i++)
    lst << h->sectionSize(i);
  conf->writeEntry("Hdrs_Size", lst);

  conf->writeEntry("sortCol", hdrView->sortColumn());
  conf->writeEntry("sortAscending", hdrView->ascending());
  conf->writeEntry("account_sortCol", collectionView->sortColumn());
  conf->writeEntry("account_sortAscending", collectionView->ascending());
}



// dis-/enable the next unread actions
void KNodeView::setNotAFolder(bool b)
{
  notAFolder = b;
  actNextUnreadArt->setEnabled(notAFolder);
  actNextUnreadThread->setEnabled(notAFolder);
}



// dis-/enable the toggle thread action
void KNodeView::setHeaderSelected(bool b)
{
  actToggleThread->setEnabled(b);
}



// switch between long & short group list, update fonts and colors
void KNodeView::updateAppearance()
{
  if (longView != knGlobals.appManager->longGroupList()) {
    longView = knGlobals.appManager->longGroupList();
    QValueList<int> size1 = sizes();
    QValueList<int> size2 = secSplitter->sizes();
    if (longView) {
      setOrientation(Qt::Horizontal);
      secSplitter->setOrientation(Qt::Vertical);
      artFocus->reparent(secSplitter,0,QPoint(0,0),true);
      colFocus->reparent(this,0,QPoint(0,0),true);
      moveToFirst(colFocus);
      moveToLast(secSplitter);
      setResizeMode(colFocus, QSplitter::KeepSize);
      setResizeMode(secSplitter, QSplitter::Stretch);
      secSplitter->moveToFirst(hdrFocus);
      secSplitter->moveToLast(artFocus);
      secSplitter->setResizeMode(hdrFocus, QSplitter::KeepSize);
      secSplitter->setResizeMode(artFocus, QSplitter::Stretch);
    } else {
      setOrientation(Qt::Vertical);
      secSplitter->setOrientation(Qt::Horizontal);
      artFocus->reparent(this,0,QPoint(0,0),true);
      colFocus->reparent(secSplitter,0,QPoint(0,0),true);
      moveToFirst(secSplitter);
      moveToLast(artFocus);
      setResizeMode(secSplitter, QSplitter::KeepSize);
      setResizeMode(artFocus, QSplitter::Stretch);
      secSplitter->moveToFirst(colFocus);
      secSplitter->moveToLast(hdrFocus);
      secSplitter->setResizeMode(colFocus, QSplitter::KeepSize);
      secSplitter->setResizeMode(hdrFocus, QSplitter::Stretch);
    }
    setSizes(size2);
    secSplitter->setSizes(size1);
  }

  KNLVItemBase::updateAppearance();
  if (knGlobals.appManager->useFonts()) {
    collectionView->setFont(knGlobals.appManager->font(KNAppManager::groupList));
    hdrView->setFont(knGlobals.appManager->font(KNAppManager::articleList));
  } else {
    collectionView->setFont(KGlobalSettings::generalFont());
    hdrView->setFont(KGlobalSettings::generalFont());
  }
  if (knGlobals.appManager->useColors()) {
    QPalette p = collectionView->palette();
    p.setColor(QColorGroup::Base,knGlobals.appManager->color(KNAppManager::background));
    collectionView->setPalette(p);
    hdrView->setPalette(p);
  } else {
    QPalette p = palette();
    collectionView->setPalette(p);
    hdrView->setPalette(p);
  }
}



void KNodeView::initCollectionView()
{
  collectionView->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  collectionView->setTreeStepSize(12);
  collectionView->setRootIsDecorated(true);
  collectionView->setShowSortIndicator(true);
  collectionView->addColumn(i18n("Name"),162);
  collectionView->addColumn(i18n("Total"),36);
  collectionView->addColumn(i18n("Unread"),48);
  collectionView->setColumnAlignment(1,AlignCenter);
  collectionView->setColumnAlignment(2,AlignCenter);
}



void KNodeView::initHdrView()
{
  hdrView->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  hdrView->setShowSortIndicator(true);
  hdrView->setRootIsDecorated(true);
  hdrView->addColumn(i18n("Subject"),207);
  hdrView->addColumn(i18n("From"),115);
  hdrView->addColumn(i18n("Score"),42);
  hdrView->addColumn(i18n("Date (Time)"),102);
  hdrView->setColumnAlignment(2, AlignCenter);
}



void KNodeView::paletteChange ( const QPalette & )
{
  updateAppearance();
  KNArticleWidget::readOptions();
  KNArticleWidget::updateInstances();
}



void KNodeView::fontChange ( const QFont & )
{
  updateAppearance();
  KNArticleWidget::readOptions();
  KNArticleWidget::updateInstances();

}



// select from KSelectAction
void KNodeView::slotSortMenuSelect(int newCol)
{
  hdrView->slotSortList(newCol);
}



// select from QListView header
void KNodeView::slotSortHdrSelect(int newCol)
{
  actSortSelect->setCurrentItem(newCol);
}



void KNodeView::slotNextArticle()
{
  QListViewItem *it=hdrView->currentItem();
  
  if(it) it=it->itemBelow();
  else it=hdrView->firstChild();
  
  if(it) {
    hdrView->setSelected(it, true);
    hdrView->setCurrentItem(it);
    hdrView->ensureItemVisible(it);
  }   
}



void KNodeView::slotPrevArticle()
{
  QListViewItem *it=hdrView->currentItem();
  
  if(it) it=it->itemAbove();
  else it=hdrView->firstChild();
  
  if(it) {
    hdrView->setSelected(it, true);
    hdrView->setCurrentItem(it);
    hdrView->ensureItemVisible(it);
  }
}
    


void KNodeView::slotNextUnreadArticle()
{
  KNHdrViewItem *next, *current;
  KNFetchArticle *art;
  
  if (!knGlobals.foManager->hasCurrentFolder()) {   // don't do anything if this is a folder !!!
  
    current=(KNHdrViewItem*)hdrView->currentItem();
    if(!current) current=(KNHdrViewItem*)hdrView->firstChild(); 
  
    if(!current) {               // no articles in the current group switch to next....
      slotNextGroup();
      return;
    }

    art=(KNFetchArticle*)current->art;
  
    if ((!current->isSelected())&&(!art->isRead()))   // take current article, if unread & not selected
      next = current;
    else {
      if(art->hasUnreadFollowUps()) hdrView->setOpen(current, true);
      next=(KNHdrViewItem*)current->itemBelow();
    }

    while(next) {
      art=(KNFetchArticle*)next->art;
    
      if(!art->isRead()) break; 
      else {
        if(art->hasUnreadFollowUps()>0) hdrView->setOpen(next, true);
        next=(KNHdrViewItem*)next->itemBelow();
      }
    }
    if(next) {
      hdrView->setSelected(next, true);
      hdrView->setCurrentItem(next);
      hdrView->ensureItemVisible(next);
    }
    else slotNextGroup();   
    
  }
}



void KNodeView::slotReadThrough()
{
  if (artView->scrollingDownPossible())
    artView->scrollDown();
  else if (notAFolder)
    slotNextUnreadArticle();
}



void KNodeView::slotNextUnreadThread()
{
  KNHdrViewItem *next, *current;
  KNFetchArticle *art;
  
  if (!knGlobals.foManager->hasCurrentFolder()) {   // don't do anything if this is a folder !!!
  
    current=(KNHdrViewItem*)hdrView->currentItem();
    if(!current) current=(KNHdrViewItem*)hdrView->firstChild(); 
    
    if(!current) {               // no articles in the current group switch to next....
      slotNextGroup();
      return;
    }
  
    art=(KNFetchArticle*)current->art;
    
    if ((current->depth()==0)&&((!current->isSelected())&&(!art->isRead() || art->hasUnreadFollowUps())))
      next=current;                           // take current article, if unread & not selected
    else  
      next=(KNHdrViewItem*)current->itemBelow();
  
    while(next) {
      art=(KNFetchArticle*)next->art;
      
      if(next->depth()==0) {
        if(!art->isRead() || art->hasUnreadFollowUps()) break;
      }
      next=(KNHdrViewItem*)next->itemBelow();
    }
  
    if(next) {  
      hdrView->setCurrentItem(next);
  
      if(art->isRead()) slotNextUnreadArticle();
      else {
        hdrView->setSelected(next, true);
        hdrView->ensureItemVisible(next);
      }
    } else slotNextGroup();
    
  }
}



void KNodeView::slotNextGroup()
{ 
  KNCollectionViewItem *current=static_cast<KNCollectionViewItem*>(collectionView->currentItem());
  KNCollectionViewItem *next=0;

  if(!current) current=(KNCollectionViewItem*)collectionView->firstChild(); 
  if(!current) return;

  next=current;
  while(next) {
    if(!next->isSelected())
      break;
    if(next->childCount()>0 && !next->isOpen()) {
      next->setOpen(true);
      knGlobals.top->secureProcessEvents();
      next=static_cast<KNCollectionViewItem*>(next->firstChild());
    }
    else next=static_cast<KNCollectionViewItem*>(next->itemBelow());
  }
    
  if(next) {
    collectionView->setCurrentItem(next);
    collectionView->ensureItemVisible(next);
    collectionView->setSelected(next, true);
  }
}



void KNodeView::slotPrevGroup()
{
  KNCollectionViewItem *current=static_cast<KNCollectionViewItem*>(collectionView->currentItem());
  KNCollectionViewItem *prev;

  if(!current) current=static_cast<KNCollectionViewItem*>(collectionView->firstChild());  
  if(!current) return;
    
  prev=current;
  while(prev) {
    if(!prev->isSelected())
      break;
    prev=static_cast<KNCollectionViewItem*>(prev->itemAbove());
  }   

  if(prev) {
    collectionView->setCurrentItem(prev);
    collectionView->ensureItemVisible(prev);
    collectionView->setSelected(prev, true);
  }
}



void KNodeView::slotToggleThread()
{
  KNHdrViewItem *it=(KNHdrViewItem*)hdrView->currentItem();
  
  if(it!=0) {
    if (it->isOpen())
      it->setOpen(false);
    else
      it->setOpen(true);
  }     
}




//--------------------------------

#include "knodeview.moc"










