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

#include <klocale.h>

#include <qheader.h>
#include <stdlib.h>
#include "knodeview.h"
#include "knarticlecollection.h"
#include "kncollectionviewitem.h"
#include "knhdrviewitem.h"
#include "knfocuswidget.h"
#include "knfetcharticle.h"


KNodeView::KNodeView(QWidget *parent, const char * name)
	: QSplitter(parent,name)
{
  KNFocusWidget *colFocus=new KNFocusWidget(this);
	collectionView=new KNListView(colFocus);
	colFocus->setWidget(collectionView);
  setResizeMode(colFocus, QSplitter::KeepSize);
	
	PanHorz=new QSplitter(QSplitter::Vertical,this);
	
	KNFocusWidget *hdrFocus=new KNFocusWidget(PanHorz);
	hdrView=new KNListView(hdrFocus);
	hdrFocus->setWidget(hdrView);
	PanHorz->setResizeMode(hdrFocus, QSplitter::KeepSize);
	
	KNFocusWidget *artFocus=new KNFocusWidget(PanHorz);	
	artView=new KNArticleWidget(artFocus);
	artFocus->setWidget(artView);
	
	initCollectionView();
	initHdrView();
	
	setTabOrder(hdrView, artView);
	setTabOrder(artView, collectionView);
	
	is_Zoomed=false;	
}



KNodeView::~KNodeView()
{
}



void KNodeView::initCollectionView()
{
 	collectionView->setFrameStyle(QFrame::Panel | QFrame::Sunken);
 	collectionView->setTreeStepSize(12);
 	collectionView->setRootIsDecorated(true);
 	collectionView->addColumn(i18n("Name"),150);
	collectionView->addColumn(i18n("Total"),50);
	collectionView->addColumn(i18n("Unread"),50);
	collectionView->setColumnAlignment(1,AlignCenter);
	collectionView->setColumnAlignment(2,AlignCenter);
	collectionView->setSorting(0);
	collectionView->header()->setClickEnabled(false);
	collectionView->setColumnWidthMode(0,QListView::Maximum);
}



void KNodeView::initHdrView()
{
	hdrView->setFrameStyle(QFrame::Panel | QFrame::Sunken);
 	hdrView->addColumn(i18n("Subject"),500);
	hdrView->addColumn(i18n("From"),300);
	hdrView->addColumn(i18n("Score"),50);
	hdrView->addColumn(i18n("Date (Time)"),300);
	hdrView->setRootIsDecorated(true);
  hdrView->setShowSortIndicator(true);
	hdrView->setColumnAlignment(2, AlignCenter);
}



void KNodeView::toggleZoom()
{
		
}


void KNodeView::sepPos(QValueList<int> &vert, QValueList<int> &horz)
{
	vert = sizes();
	horz = PanHorz->sizes();
} 		
 		
 		
 		
void KNodeView::setSepPos(const QValueList<int> &vert, const QValueList<int> &horz)
{
	if (vert.count()==2)
		setSizes(vert);
	if (horz.count()==2)			
		PanHorz->setSizes(horz);
}
		


void KNodeView::headersSize(QStrList *lst)
{
	QHeader *h;
	char str[10];
	
	lst->clear();
	
	h=collectionView->header();
	for(int i=0; i < 3; i++) {
		sprintf(str, "%d", h->cellSize(i));
		lst->append(str);
	}
	
	h=hdrView->header();
	for(int i=0; i < 4; i++) {
		sprintf(str, "%d", h->cellSize(i));
		lst->append(str);
	}
			
}


void KNodeView::setHeadersSize(QStrList *lst)
{
	QHeader *h;
  char *var=lst->first();
	
	h=collectionView->header();
	for(int i=0; i<3; i++) {
		h->setCellSize(i, atoi(var));
		var=lst->next();
	}	
	
	h=hdrView->header();
	for(int i=0; i<4; i++) {
		h->setCellSize(i, atoi(var));
		var=lst->next();
		if(!var) break;
	}		
}



void KNodeView::nextArticle()
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



void KNodeView::prevArticle()
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
 		


void KNodeView::nextUnreadArticle()
{
	KNHdrViewItem *next, *current;
	KNFetchArticle *art;
	
	current=(KNHdrViewItem*)hdrView->currentItem();
	if(!current) current=(KNHdrViewItem*)hdrView->firstChild();	
	
	if(!current) {               // no articles in the current group switch to next....
		nextGroup();
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
	else nextGroup();		
}



void KNodeView::readThrough()
{
	if (artView->scrollingDownPossible())
		artView->scrollDown();
	else
		nextUnreadArticle();
}



void KNodeView::nextUnreadThread()
{
	KNHdrViewItem *next, *current;
	KNFetchArticle *art;
	
	current=(KNHdrViewItem*)hdrView->currentItem();
	if(!current) current=(KNHdrViewItem*)hdrView->firstChild();	
	
	if(!current) {               // no articles in the current group switch to next....
		nextGroup();
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
	
	  if(art->isRead()) nextUnreadArticle();
	  else {
			hdrView->setSelected(next, true);
		  hdrView->ensureItemVisible(next);
		}
	}
	else nextGroup();
}



void KNodeView::nextGroup()
{	
	KNCollectionViewItem *current=static_cast<KNCollectionViewItem*>(collectionView->currentItem());
	KNCollectionViewItem *next=0;

	if(!current) current=(KNCollectionViewItem*)collectionView->firstChild();	
	if(!current) return;

	//if(!current->isSelected() && current->coll->type()==KNArticleCollection::CTgroup)
	next=current;
		
 	while(next) {
 		if(!next->isSelected() && next->coll->type()==KNArticleCollection::CTgroup)
 			break;
 		if(next->childCount()>0 && !next->isOpen()) {
 			next->setOpen(true);
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



void KNodeView::prevGroup()
{
	KNCollectionViewItem *current=static_cast<KNCollectionViewItem*>(collectionView->currentItem());
	KNCollectionViewItem *prev;

	if(!current) current=static_cast<KNCollectionViewItem*>(collectionView->firstChild());	
	if(!current) return;
  	
  prev=current;
  while(prev) {
  	if(!prev->isSelected() && prev->coll->type()==KNArticleCollection::CTgroup)
 			break;
 		if(prev->childCount()>0 && !prev->isOpen()) {
 			prev->setOpen(true);
 			kapp->processEvents();
 			current=prev;
 			prev=static_cast<KNCollectionViewItem*>(current->firstChild());
 			while(prev->itemBelow()->parent()==current)
 				prev=static_cast<KNCollectionViewItem*>(prev->itemBelow());
 		}
 		else prev=static_cast<KNCollectionViewItem*>(prev->itemAbove());
 	}		

	if(prev) {
  	collectionView->setCurrentItem(prev);
		collectionView->ensureItemVisible(prev);
		collectionView->setSelected(prev, true);
	}
}



void KNodeView::toggleThread()
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










