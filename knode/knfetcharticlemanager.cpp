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

#include <qheader.h>

#include <klocale.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kstdaccel.h>
#include <kwin.h>
#include <kdebug.h>

#include "knnntpaccount.h"
#include "knarticlewidget.h"
#include "knfetcharticle.h"
#include "knode.h"
#include "knstatusfilter.h"
#include "knrangefilter.h"
#include "knstringfilter.h"
#include "knfetcharticlemanager.h"
#include "knsavedarticlemanager.h"
#include "knarticlewindow.h"
#include "knhdrviewitem.h"
#include "knthread.h"
#include "knscoredialog.h"
#include "kngroup.h"
#include "knarticlefilter.h"
#include "knfiltermanager.h"
#include "knjobdata.h"
#include "knsearchdialog.h"
#include "knnetaccess.h"
#include "knglobals.h"


KNFetchArticleManager::KNFetchArticleManager(KNListView *v, KNFilterManager* fiManager, QObject * parent, const char * name)
  : QObject(parent, name), KNArticleManager(v), g_roup(0), c_urrent(0), n_ext(0), tOut(3000), sDlg(0)
{
  connect(fiManager, SIGNAL(filterChanged(KNArticleFilter*)), this, SLOT(slotFilterChanged(KNArticleFilter*)));
  f_ilter = fiManager->currentFilter();

  timer=new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
  connect(view, SIGNAL(expanded(QListViewItem*)), this, SLOT(slotItemExpanded(QListViewItem*)));

  readOptions();

  actShowThreads = new KToggleAction(i18n("Show T&hreads"), 0 , this, SLOT(slotToggleShowThreads()),
                                     &actionCollection, "view_showThreads");
  actShowThreads->setChecked(t_hreaded);

  actExpandAll = new KAction(i18n("&Expand all threads"), 0 , this, SLOT(slotThreadsExpand()),
                             &actionCollection, "view_ExpandAll");
  actCollapseAll = new KAction(i18n("&Collapse all threads"), 0 , this, SLOT(slotThreadsCollapse()),
                               &actionCollection, "view_CollapseAll");
  actRefresh = new KAction(i18n("&Refresh List"),"reload", KStdAccel::key(KStdAccel::Reload), this, SLOT(slotRefresh()),
                           &actionCollection, "view_Refresh");
  actAllRead = new KAction(i18n("Mark all as &read"), 0, this, SLOT(slotAllRead()),
                           &actionCollection, "group_allRead");
  actAllUnread = new KAction(i18n("Mark all as u&nread"), 0, this, SLOT(slotAllUnread()),
                             &actionCollection, "group_allUnread");
  actPostReply = new KAction(i18n("Post &reply"),"reply", Key_R , this, SLOT(slotReply()),
                             &actionCollection, "article_postReply");
  actMailReply = new KAction(i18n("&Mail reply"),"remail", Key_A , this, SLOT(slotRemail()),
                             &actionCollection, "article_mailReply");
  actForward = new KAction(i18n("&Forward"),"fwd", Key_F , this, SLOT(slotForward()),
                           &actionCollection, "article_forward");
  actMarkRead = new KAction(i18n("M&ark as read"), Key_D , this, SLOT(slotMarkRead()),
                            &actionCollection, "article_read");
  actMarkUnread = new KAction(i18n("Mar&k as unread"), Key_U , this, SLOT(slotMarkUnread()),
                              &actionCollection, "article_unread");
  actThreadRead = new KAction(i18n("Mark thread as r&ead"), CTRL+Key_D , this, SLOT(slotThreadRead()),
                              &actionCollection, "thread_read");
  actThreadUnread = new KAction(i18n("Mark thread as u&nread"), CTRL+Key_U , this, SLOT(slotThreadUnread()),
                                &actionCollection, "thread_unread");
  actThreadSetScore = new KAction(i18n("Set &Score..."), Key_S , this, SLOT(slotThreadScore()),
                                  &actionCollection, "thread_setScore");
  actThreadWatch = new KAction(i18n("&Watch"), Key_W , this, SLOT(slotThreadWatch()),
                               &actionCollection, "thread_watch");
  actThreadIgnore = new KAction(i18n("&Ignore"), Key_I , this, SLOT(slotThreadIgnore()),
                                &actionCollection, "thread_ignore");
  actOwnWindow = new KAction(i18n("&Open in own window"), Key_O , this, SLOT(slotOwnWindow()),
                             &actionCollection, "article_ownWindow");
  actSearch = new KAction(i18n("&Search..."),"find" , Key_F4 , this, SLOT(slotSearch()),
                          &actionCollection, "article_search");
}



KNFetchArticleManager::~KNFetchArticleManager()
{
  delete timer;
  delete sDlg;
}



void KNFetchArticleManager::readOptions()
{
  KConfig *c=KGlobal::config();
  c->setGroup("READNEWS");
  tOut=1000*c->readNumEntry("markSecs", 3);
  totalExpand=c->readBoolEntry("totalExpand", true);
  t_hreaded=c->readBoolEntry("showThreads", true);
  autoMark=c->readBoolEntry("autoMark", true);
}



void KNFetchArticleManager::saveOptions()
{
  KConfig *c=KGlobal::config();
  c->setGroup("READNEWS");

  c->writeEntry("showThreads", t_hreaded);  // the remaining options are save in the settings-module
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

  knGlobals.top->setCursorBusy(true);
  knGlobals.top->setStatusMsg(i18n(" Creating list ..."));
  knGlobals.top->secureProcessEvents();

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
    art->setThreadMode(t_hreaded);
    if (t_hreaded) {
      if( ( !art->listItem() && art->filterResult() ) &&
          ( art->idRef()==0 || !g_roup->byId(art->idRef())->filterResult() ) ) {
        art->setListItem(new KNHdrViewItem(view));
        art->initListItem();
      }
      else if(art->listItem())
        art->updateListItem();
    } else {
      if(!art->listItem() && art->filterResult()) {
        art->setListItem(new KNHdrViewItem(view));
        art->initListItem();
      }
      else if(art->listItem())
        art->updateListItem();
    }
  }
  if(view->firstChild())
    view->setCurrentItem(view->firstChild());

  knGlobals.top->setStatusMsg("");
  updateStatusString();
  knGlobals.top->setCursorBusy(false);
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
    kdDebug(5003) << "KNFetchArticleManager::setCurrentArticle() : messageID=" << a->messageId() << endl;
    if(a->locked()) return;
    if(a->hasContent()) {
      c_urrent=a;
      mainArtWidget->setData(a, g_roup);
      showArticle(a);
    }
    else {
      KNJobData *job=new KNJobData(KNJobData::JTfetchArticle, g_roup->account(), a);
      knGlobals.netAccess->addJob(job);
    }
  }
  else {
    c_urrent=0;
//    knGlobals.top->fetchArticleDisplayed(false);
  }
//  knGlobals.top->fetchArticleSelected((c_urrent!=0));
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
    knGlobals.netAccess->addJob(job);
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
      if( ref->listItem() &&
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

  int new_count = 0;
  KNFetchArticle *a;
  for(int i=0; i<g->length(); i++) {
    a = g->at(i);
    if(a->isRead()!=r) {
      a->setRead(r);
      a->setHasChanged(true);
      if(a->isNew()) new_count++;
    }
  }

  g->updateThreadInfo();
  if(r) {
    g->setReadCount(g->length());
    g->setNewCount(0);
  } else {
    g->setReadCount(0);
    g->setNewCount(new_count);
  }

  g->updateListItem();
  showHdrs(true);
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



void KNFetchArticleManager::setArticleScore(KNFetchArticle *)
{
}



void KNFetchArticleManager::setThreadScore(KNFetchArticle *a, int score)
{
  KNThread *thr;
  KNScoreDialog *sd;
  if(!a) a=c_urrent;
  if(a) {
    if(score==-1) {
      sd=new KNScoreDialog(a->score(), knGlobals.topWidget);
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
  if (sDlg) {
    sDlg->show();
    KWin::setActiveWindow(sDlg->winId());
  } else {
    sDlg=new KNSearchDialog(KNSearchDialog::STfolderSearch, 0);
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
//    knGlobals.top->fetchArticleDisplayed(true);
  }
}



void KNFetchArticleManager::showCancel(KNArticle *a)
{
  if(a==c_urrent) {
    timer->stop();
//    knGlobals.top->fetchArticleDisplayed(false);
  }
}



void KNFetchArticleManager::showError(KNArticle *a, const QString &error)
{
  KNArticleManager::showError(a, error);
  if(a==c_urrent) {
    timer->stop();
//    knGlobals.top->fetchArticleDisplayed(false);
  }
}




void KNFetchArticleManager::createHdrItem(KNFetchArticle *a)
{
  a->setListItem(new KNHdrViewItem(view));
  a->setThreadMode(t_hreaded);
  a->initListItem();
}



void KNFetchArticleManager::createThread(KNFetchArticle *a)
{
  KNFetchArticle *ref=0;
  int idRef=a->idRef();
  bool found=false;

  while(idRef!=0 && !found) {
    ref=g_roup->byId(idRef);
    found=ref->filterResult();
    idRef=ref->idRef();
  }

  if(found) {
    if(!ref->listItem())  createThread(ref);
    a->setListItem(new KNHdrViewItem(ref->listItem()));
  } else
    a->setListItem(new KNHdrViewItem(view));

  a->setThreadMode(t_hreaded);
  a->initListItem();
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
      knGlobals.netAccess->addJob(job);
    }
    if(target==mainArtWidget && ref->listItem())
      view->setSelected(ref->listItem(), true);
  }
  else target->showErrorMessage(i18n("article not found"));
}



void KNFetchArticleManager::slotItemExpanded(QListViewItem *p)
{
  int idRef=0, topId=0;
  KNFetchArticle *art, *ref;
  KNHdrViewItem *hdrItem;

  bool inThread=false;

  if(p->childCount() > 0) {
    //kdDebug(5003) << "KNFetchArticleManager::slotItemExpanded() : childCount = " << p->childCount() << " => returning" << endl;
    return;
  }
  hdrItem=static_cast<KNHdrViewItem*>(p);
  topId=hdrItem->art->id();


  for(int i=0; i<g_roup->count(); i++) {
    art=g_roup->at(i);
    if(art->filterResult() && !art->listItem()) {

      if(art->idRef()==topId) {
        art->setListItem(new KNHdrViewItem(hdrItem));
        art->setThreadMode(t_hreaded);
        art->initListItem();
      }
      else if(totalExpand) {
        idRef=art->idRef();
        inThread=false;
        while(idRef>0 && !inThread) {
          ref=g_roup->byId(idRef);
          inThread=(ref->id()==topId);
          idRef=ref->idRef();
        }
        if(inThread)
          createThread(art);
      }
    }
  }
  if(totalExpand) hdrItem->expandChildren();
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
  slotFilterChanged(knGlobals.fiManager->currentFilter());
}



void KNFetchArticleManager::slotTimer()
{
  if(c_urrent) setArticleRead(c_urrent);
}



void KNFetchArticleManager::slotReply()
{
  knGlobals.sArtManager->reply(c_urrent,g_roup);
}



void KNFetchArticleManager::slotRemail()
{
  knGlobals.sArtManager->reply(c_urrent,0);
}



void KNFetchArticleManager::slotForward()
{
  knGlobals.sArtManager->forward(c_urrent);
}



void KNFetchArticleManager::updateStatusString()
{
  int displCnt=0;

  if(g_roup) {
    if (f_ilter)
      displCnt=f_ilter->count();
    else
      displCnt=g_roup->count();

    knGlobals.top->setStatusMsg(i18n(" %1 : %2 new , %3 displayed")
                        .arg(g_roup->name()).arg(g_roup->newCount()).arg(displCnt),SB_GROUP);

    if (f_ilter)
      knGlobals.top->setStatusMsg(i18n(" Filter: %1").arg(f_ilter->name()), SB_FILTER);
    else
      knGlobals.top->setStatusMsg(QString::null, SB_FILTER);
  }
}



// -----------------------------------------------------------------------------

#include "knfetcharticlemanager.moc"
