/***************************************************************************
                          knfetcharticlemanager.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
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
#include <kfiledialog.h>
#include <qheader.h>
#include "knfetcharticlemanager.h"
#include "utilities.h"
#include "knarticlewindow.h"
#include "knhdrviewitem.h"
#include "knthread.h"
#include "knscoredialog.h"
#include "knglobals.h"
#include "kngroup.h"
#include "knarticlefilter.h"
#include "knjobdata.h"
#include "knsearchdialog.h"

KNFetchArticleManager::KNFetchArticleManager(KNListView *v) :
	QObject(0,0), KNArticleManager(v)
{
	g_roup=0;
	f_ilter=0;
	tOut=3000;
	sDlg=0;
	c_urrent=0;
	n_ext=0;
		
	timer=new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
	
	readConfig();
}



KNFetchArticleManager::~KNFetchArticleManager()
{
	delete timer;
	delete sDlg;
}



void KNFetchArticleManager::readConfig()
{
	KConfig *c=CONF();
	c->setGroup("READNEWS");
	tOut=1000*c->readNumEntry("markSecs", 3);
	KNLVItemBase::setTotalExpand(c->readBoolEntry("totalExpand", true));
	t_hreaded=c->readBoolEntry("showThreads", true);
	autoMark=c->readBoolEntry("autoMark", true);
}



void KNFetchArticleManager::setGroup(KNGroup *g)
{
	if(g!=0) {
		if(g_roup==0) view->header()->setLabel(1, i18n("From"));
		if(sDlg) {
			sDlg->hide();
			if(f_ilter==sDlg->filter()) slotFilterChanged(0);
		}				
	}
	
	g_roup=g;
	setCurrentArticle(0);
	timer->stop();		
}


void KNFetchArticleManager::showHdrs(bool clear)
{
	if(!g_roup) return;
	KNFetchArticle *art;
		
	xTop->setCursorBusy(true);
	xTop->setStatusMsg(i18n(" Creating list ..."));
	if(clear) {
		view->clear();	
 		setCurrentArticle(0);
 	}
 	if(f_ilter) f_ilter->doFilter(g_roup);
	else
		for (int i=0; i<g_roup->length(); i++)
			g_roup->at(i)->setFilterResult(true);
	
	for (int i=0; i<g_roup->length(); i++){
		art=g_roup->at(i);
		art->setFiltered(false);
		if(!art->listItem() && art->filterResult()) {
			if(t_hreaded) createThread(art);
			else createHdrItem(art);
			//if((++cnt>100) && (cnt%100==0)) kapp->processEvents();
		}		
	}
	
	if(view->firstChild())
	  view->setCurrentItem(view->firstChild());
	
	xTop->setStatusMsg("");	
	xTop->setCursorBusy(false);
	xTop->groupDisplayed(true);
	
	updateStatusString();
}



void KNFetchArticleManager::expandAllThreads(bool e)
{
	for(int idx=0; idx<g_roup->length(); idx++)
		if(g_roup->at(idx)->listItem())
			g_roup->at(idx)->listItem()->QListViewItem::setOpen(e);
}



void KNFetchArticleManager::setCurrentArticle(KNFetchArticle *a)
{
	n_ext=a;
	if(a) {
		qDebug("KNFetchArticleManager::setCurrentArticle() : messageID=%s", a->messageId().data());
		if(a->locked()) return;
		if(a->hasContent()) {
			c_urrent=a;
			mainArtWidget->setData(a, g_roup);
			showArticle(a);
		}
		else {
			KNJobData *job=new KNJobData(KNJobData::JTfetchArticle, g_roup->account(), a);
			xNet->addJob(job);
		}
	}
	else {
		c_urrent=0;
		xTop->fetchArticleDisplayed(false);
	}
	xTop->fetchArticleSelected((c_urrent!=0));
}



void KNFetchArticleManager::articleWindow(KNFetchArticle *a)
{
	if(!a) a=c_urrent;
	if(!a) return;
	KNArticleWindow *win=new KNArticleWindow(a, g_roup);
	win->show();
		
	if(a->hasContent()) win->artWidget()->createHtmlPage();
	else {
		KNJobData *job=new KNJobData(KNJobData::JTfetchArticle, g_roup->account(), a);
		xNet->addJob(job);
	}		
}



void KNFetchArticleManager::setArticleRead(KNFetchArticle *a, bool r, bool ugli)
{
	if(!a) a=c_urrent;
	if(!a) return;
	if(a->isRead()!=r) {
		KNFetchArticle *ref;
		int idRef;
		a->setRead(r);	
	 	a->setHasChanged(true);
	 	a->updateListItem();
	 	
	 	idRef=a->idRef();
	 	
	 	while(idRef!=0) {
			ref=g_roup->byId(idRef);
			if(r) {
				ref->decUnreadFollowUps();
				if(a->isNew()) ref->decNewFollowUps();
			}
			else {
				ref->incUnreadFollowUps();
				if(a->isNew()) ref->incNewFollowUps();
			}
			if(	ref->listItem() &&
					( (ref->unreadFollowUps()==0 || ref->unreadFollowUps()==1) ||
					  (ref->newFollowUps()==0 || ref->newFollowUps()==1)))    ref->updateListItem();
				
			idRef=ref->idRef();
		}	  	
	 	
	 	if(r) {
	 		g_roup->incReadCount();
	 		if(a->isNew()) g_roup->decNewCount();
	 	}
	 	else {
	 		g_roup->decReadCount();
	 		if(a->isNew()) g_roup->incNewCount();
	 	}
		if(ugli) g_roup->updateListItem();
	}	
  if(a->isNew()) updateStatusString();
}



void KNFetchArticleManager::setAllRead(KNGroup *g, bool r)
{
	if(!g) g=g_roup;
	if(!g) return;
	view->setUpdatesEnabled(false);
	for(int i=0; i<g->length(); i++)
		setArticleRead(g->at(i), r, false);
	g_roup->updateListItem();
	updateStatusString();
	view->setUpdatesEnabled(true);
}



void KNFetchArticleManager::setThreadRead(KNFetchArticle *a, bool r)
{
	if(!a) a=c_urrent;
	if(!a) return;
	if(a) {
		KNThread *thr=new KNThread(g_roup, a);
		int cnt, n_ew;
		cnt=thr->setRead(r, n_ew);
		
		if(cnt>0) {
			if(r) {
				g_roup->incReadCount(cnt);
				g_roup->decNewCount(n_ew);
			}
			else {
				g_roup->decReadCount(cnt);
				g_roup->incNewCount(n_ew);
			}
			g_roup->updateListItem();
		}
		delete thr;
	  if(n_ew>0) updateStatusString();
	}		
}



void KNFetchArticleManager::toggleWatched(KNFetchArticle *a)
{
	KNThread *thr;
	if(!a) a=c_urrent;
	if(a) {
		thr=new KNThread(g_roup, a);
		thr->toggleWatched();
		delete thr;
	}			
}



void KNFetchArticleManager::toggleIgnored(KNFetchArticle *a)
{
  KNThread *thr;
	if(!a) a=c_urrent;
	if(a) {
		thr=new KNThread(g_roup, a);
		thr->toggleIgnored();
		delete thr;
	}		
}



void KNFetchArticleManager::setArticleScore(KNFetchArticle *a)
{
}



void KNFetchArticleManager::setThreadScore(KNFetchArticle *a, int score)
{
	KNThread *thr;
	KNScoreDialog *sd;
	if(!a) a=c_urrent;
	if(a) {
		if(score==-1) {
			sd=new KNScoreDialog(a->score(), xTop);
			if(sd->exec()) score=sd->score();
			delete sd;
		}
		thr=new KNThread(g_roup, a);
		thr->setScore(score);
		delete thr;
	}		
}



void KNFetchArticleManager::search()
{
	if(!g_roup) return;
	if(sDlg) sDlg->show();
	else {
		sDlg=new KNSearchDialog();
		connect(sDlg, SIGNAL(doSearch(KNArticleFilter*)), this,
			SLOT(slotFilterChanged(KNArticleFilter*)));
		connect(sDlg, SIGNAL(dialogDone()), this,
			SLOT(slotSearchDialogDone()));
		sDlg->show();
	}
}



void KNFetchArticleManager::showArticle(KNArticle *a)
{
	KNArticleManager::showArticle(a);
	if(a==c_urrent) {
		if(!c_urrent->isRead() && autoMark) timer->start(tOut, true);
		xTop->fetchArticleDisplayed(true);
	}
}



void KNFetchArticleManager::showCancel(KNArticle *a)
{
//	KNArticleWidget *aw=KNArticleWidget::find(a);
	if(a==c_urrent) {
		timer->stop();
		xTop->fetchArticleDisplayed(false);
	}	
//	if(aw) aw->showCancelMessage();             // I think we don't need this, check later (CG)
}



void KNFetchArticleManager::showError(KNArticle *a, const QString &error)
{
	KNArticleManager::showError(a, error);
	if(a==c_urrent) {
		timer->stop();
		xTop->fetchArticleDisplayed(false);
	}
}



void KNFetchArticleManager::createHdrItem(KNFetchArticle *a)
{
	a->setListItem(new KNHdrViewItem(view));
	a->initListItem();
}



void KNFetchArticleManager::createThread(KNFetchArticle *a)
{
	KNFetchArticle *ref;	
  int idRef;
	bool found;

	if(a->idRef()==0) createHdrItem(a);
	else {
		idRef=a->idRef();
		found=false;
		while(idRef!=0 && !found) {
				ref=g_roup->byId(idRef);
				if(ref->filterResult()) found=true;
				else idRef=ref->idRef();
		}	
		
		if(found) {	
			if(!ref->listItem())  createThread(ref);
			a->setListItem(new KNHdrViewItem(ref->listItem()));
			a->initListItem();
		}
		else createHdrItem(a);
	}
}



void KNFetchArticleManager::jobDone(KNJobData *j)
{
	KNFetchArticle *art=(KNFetchArticle*)j->data();
	
	if(art==n_ext) {
		c_urrent=n_ext;
		n_ext=0;
		mainArtWidget->setData(c_urrent, g_roup);
	}
	if(j->canceled()) showCancel(art);
	else {
		if(j->success()) {
		  art->updateListItem();
			showArticle(art);
		}
		else showError(art, j->errorString());
	}
	delete j;	
}



void KNFetchArticleManager::referenceClicked(int refNr, KNArticleWidget *aw,int btn)
{
	KNGroup *grp;
	KNFetchArticle *art, *ref;
	KNArticleWidget *target;
	KNArticleWindow *win;
	
	grp=(KNGroup*)aw->collection();
	art=(KNFetchArticle*)aw->article();
	ref=grp->byMessageId(art->references().at(refNr));
	if(btn==4) {
		win=new KNArticleWindow();
		win->show();
		target=win->artWidget();
	}
	else target=aw;
	
	target->setData(ref, grp);
	if(ref) {
		if(ref->hasContent()) target->createHtmlPage();
		else {
			KNJobData *job=new KNJobData(KNJobData::JTfetchArticle, g_roup->account(), ref);
			xNet->addJob(job);
		}	
		if(target==mainArtWidget && ref->listItem())
			view->setSelected(ref->listItem(), true);			
	}
	else target->showErrorMessage(i18n("article not found"));	
}



void KNFetchArticleManager::slotFilterChanged(KNArticleFilter *f)
{
	timer->stop();
	f_ilter=f;
	showHdrs();
}



void KNFetchArticleManager::slotSearchDialogDone()
{
	sDlg->hide();
	slotFilterChanged(xTop->fiManager()->currentFilter());	
}



void KNFetchArticleManager::slotTimer()
{
	if(c_urrent) setArticleRead(c_urrent);
}



void KNFetchArticleManager::updateStatusString()
{
	int displCnt=0;
	
	if(g_roup) {				
		if (f_ilter)
		  displCnt=f_ilter->count();
		else
		  displCnt=g_roup->count();
		
		xTop->setStatusMsg(i18n(" %1 : %2 new , %3 displayed")
		                    .arg(g_roup->name()).arg(g_roup->newCount()).arg(displCnt),SB_GROUP);
		
		if (f_ilter)
  		xTop->setStatusMsg(i18n(" Filter: %1").arg(f_ilter->name()), SB_FILTER);
  	else
      xTop->setStatusMsg(QString::null, SB_FILTER);

    xTop->setCaption(g_roup->name());		
	}	
}


// -----------------------------------------------------------------------------

#include "knfetcharticlemanager.moc"
