/***************************************************************************
                          knarticlemanager.cpp  -  description
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

#include <pthread.h>

#include <qheader.h>

#include <mimelib/string.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kio/netaccess.h>
#include <ktempfile.h>
#include <kuserprofile.h>
#include <kopenwith.h>
#include <klocale.h>
#include <kdebug.h>
#include <kwin.h>
#include <kcharsets.h>

#include "knode.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "utilities.h"
#include "knarticlemanager.h"
#include "knsearchdialog.h"
#include "knlistview.h"
#include "knfiltermanager.h"
#include "kngroup.h"
#include "knfolder.h"
#include "knarticlefilter.h"
#include "knhdrviewitem.h"
#include "knnetaccess.h"


QString KNSaveHelper::lastPath;

KNSaveHelper::KNSaveHelper(QString saveName)
  : s_aveName(saveName), file(0), tmpFile(0)
{
}


KNSaveHelper::~KNSaveHelper()
{
  if (file) {       // local filesystem, just close the file
    delete file;
  } else
    if (tmpFile) {      // network location, initiate transaction
      tmpFile->close();
      if (KIO::NetAccess::upload(tmpFile->name(),url) == false)
        displayRemoteFileError();
      tmpFile->unlink();   // delete temp file
      delete tmpFile;
    }
}


QFile* KNSaveHelper::getFile(QString dialogTitle)
{
  url = KFileDialog::getSaveURL(lastPath+s_aveName,QString::null,knGlobals.topWidget,dialogTitle);

  if (url.isEmpty())
    return 0;

  lastPath = url.url(-1);
  lastPath.truncate(lastPath.length()-url.fileName().length());

  if (url.isLocalFile()) {
    if (QFileInfo(url.path()).exists() &&
        (KMessageBox::warningContinueCancel(knGlobals.topWidget,
                                            i18n("A file named %1 already exists.\nDo you want to replace it?").arg(url.path()),
                                            dialogTitle, i18n("&Replace")) != KMessageBox::Continue)) {
      return 0;
    }

    file = new QFile(url.path());
    if(!file->open(IO_WriteOnly)) {
      displayExternalFileError();
      delete file;
      file = 0;
    }
    return file;
  } else {
    tmpFile = new KTempFile();
    if (tmpFile->status()!=0) {
      displayTempFileError();
      delete tmpFile;
      tmpFile = 0;
      return 0;
    }
    return tmpFile->file();
  }
}


//===============================================================================


KNArticleManager::KNArticleManager(KNListView *v, KNFilterManager *f) : QObject(0,0)
{
  v_iew=v;
  g_roup=0;
  f_older=0;
  f_ilter=f->currentFilter();
  f_ilterMgr=f;
  s_earchDlg=0;
  s_howThreads=knGlobals.cfgManager->readNewsGeneral()->showThreads();

	connect(v, SIGNAL(expanded(QListViewItem*)), this,
		SLOT(slotItemExpanded(QListViewItem*)));
  connect(f, SIGNAL(filterChanged(KNArticleFilter*)), this,
  	SLOT(slotFilterChanged(KNArticleFilter*)));
}


KNArticleManager::~KNArticleManager()
{
  delete s_earchDlg;
  deleteTempFiles();
}


void KNArticleManager::deleteTempFiles()
{
  KTempFile *file;

  while ((file = t_empFiles.first())) {
    file->unlink();                 // deletes file
    t_empFiles.removeFirst();
    delete file;
  }
}


void KNArticleManager::saveContentToFile(KNMimeContent *c)
{
	KNSaveHelper helper(c->contentType()->name());

  QFile *file = helper.getFile(i18n("Save Attachment"));

  if (file) {
		QByteArray data=c->decodedContent();
    file->writeBlock(data.data(), data.size());
  }
}


void KNArticleManager::saveArticleToFile(KNArticle *a)
{
  QString fName = a->subject()->asUnicodeString();
  fName.replace(QRegExp("[\\s/]"),"_");
  KNSaveHelper helper(fName);
  QFile *file = helper.getFile(i18n("Save Article"));
  KNMimeContent *text=0;

  if (file) {
    QCString tmp=a->head().copy();
    tmp+="\n";
    text=a->textContent();
    if(text) {
    	QString body;
    	a->decodedText(body);
      tmp+=body.local8Bit();
    }
    file->writeBlock(tmp.data(), tmp.size());
  }
}


QString KNArticleManager::saveContentToTemp(KNMimeContent *c)
{
	QString path;
  KNHeaders::Base *pathHdr=c->getHeaderByType("X-KNode-Tempfile");  // check for existing temp file

  if(pathHdr) {
    return pathHdr->asUnicodeString();
  }

  KTempFile* tmpFile=new KTempFile();
  if (tmpFile->status()!=0) {
    displayTempFileError();
    delete tmpFile;
    return QString::null;
  }

  t_empFiles.append(tmpFile);
  QFile *f=tmpFile->file();
	QByteArray data=c->decodedContent();
  f->writeBlock(data.data(), data.size());
  tmpFile->close();
  path=tmpFile->name();
  pathHdr=new KNHeaders::Generic("X-KNode-Tempfile", path, QFont::Unicode);
  c->setHeader(pathHdr);

  return path;
}


void KNArticleManager::openContent(KNMimeContent *c)
{
	QString path=saveContentToTemp(c);
  if(path.isNull()) return;

  KService::Ptr offer = KServiceTypeProfile::preferredService(c->contentType()->mimeType(), true);
  KURL::List lst;
  KURL url;
  url.setPath(path);
  lst.append(url);

  if (offer)
    KRun::run(*offer, lst);
  else {
    KFileOpenWithHandler *openhandler = new KFileOpenWithHandler();
    openhandler->displayOpenWithDialog(lst);
  }
}


void KNArticleManager::showHdrs(bool clear)
{
	if(!g_roup && !f_older) return;

  if(clear)
  	v_iew->clear();

  knGlobals.top->setCursorBusy(true);
  knGlobals.top->setStatusMsg(i18n(" Creating list ..."));
  knGlobals.top->secureProcessEvents();

  if(g_roup) {
    KNRemoteArticle *art; //, *ref;

    if (g_roup->isLocked()) {
      if (0!=pthread_mutex_lock(knGlobals.netAccess->nntpMutex())) {
        kdDebug(5003) << "failed to lock nntp mutex" << endl;
        knGlobals.top->setStatusMsg(QString::null);
        updateStatusString();
        knGlobals.top->setCursorBusy(false);
        return;
      }
    }

    if(f_ilter)
    	f_ilter->doFilter(g_roup);
    else
      for(int i=0; i<g_roup->length(); i++) {
      	art=g_roup->at(i);
        art->setFilterResult(true);
        art->setFiltered(true);
        if(art->idRef()>0)
        	g_roup->byId(art->idRef())->setVisibleFollowUps(true);
      }

    for(int i=0; i<g_roup->length(); i++) {
      art=g_roup->at(i);
      art->setThreadMode(s_howThreads);
      if (s_howThreads) {
        if( ( !art->listItem() && art->filterResult() ) &&
            ( art->idRef()==0 || !g_roup->byId(art->idRef())->filterResult() ) ) {
          art->setListItem(new KNHdrViewItem(v_iew));
          art->initListItem();
        }
        else if(art->listItem())
          art->updateListItem();
      }
      else {
        if(!art->listItem() && art->filterResult()) {
          art->setListItem(new KNHdrViewItem(v_iew));
          art->initListItem();
        }
        else if(art->listItem())
          art->updateListItem();
      }
    }


    if (g_roup->isLocked() && (0!=pthread_mutex_unlock(knGlobals.netAccess->nntpMutex())))
      kdDebug(5003) << "failed to unlock nntp mutex" << endl;
  }
  else { //folder
    KNLocalArticle *art;
    for(int idx=0; idx<f_older->length(); idx++) {
      art=f_older->at(idx);
      if(!art->listItem()) {
        art->setListItem( new KNHdrViewItem(v_iew, art) );
        art->updateListItem();
      }
    }
  }

  if(v_iew->firstChild())
    v_iew->setCurrentItem(v_iew->firstChild());

  knGlobals.top->setStatusMsg(QString::null);
  updateStatusString();
  knGlobals.top->setCursorBusy(false);
}


void KNArticleManager::updateViewForCollection(KNArticleCollection *c)
{
  if(g_roup==c || f_older==c)
    showHdrs(false);
}


void KNArticleManager::setAllThreadsOpen(bool b)
{
  if(g_roup) {
    knGlobals.top->setCursorBusy(true);
    for(int idx=0; idx<g_roup->length(); idx++)
      if(g_roup->at(idx)->listItem())
        g_roup->at(idx)->listItem()->QListViewItem::setOpen(b);
    knGlobals.top->setCursorBusy(false);
  }
}


void KNArticleManager::setViewFont()
{
  QFont fnt=knGlobals.cfgManager->appearance()->articleListFont();
  if(g_roup && g_roup->useCharset())
    KGlobal::charsets()->setQFont(fnt, g_roup->defaultCharset());
  v_iew->setFont(fnt);
}


void KNArticleManager::search()
{
  if(!g_roup) return;
  if(s_earchDlg) {
    s_earchDlg->show();
    KWin::setActiveWindow(s_earchDlg->winId());
  } else {
    s_earchDlg=new KNSearchDialog(KNSearchDialog::STgroupSearch, 0);
    connect(s_earchDlg, SIGNAL(doSearch(KNArticleFilter*)), this,
      SLOT(slotFilterChanged(KNArticleFilter*)));
    connect(s_earchDlg, SIGNAL(dialogDone()), this,
      SLOT(slotSearchDialogDone()));
    s_earchDlg->show();
  }
}


void KNArticleManager::setGroup(KNGroup *g)
{
  g_roup=g;

  if(g) {
    v_iew->header()->setLabel(1, i18n("From"));
    setViewFont();
  }
}


void KNArticleManager::setFolder(KNFolder *f)
{
  f_older=f;
  if(f) {
    v_iew->header()->setLabel(1, i18n("Newsgroups / To"));
    setViewFont();
  }
}


KNArticleCollection* KNArticleManager::collection()
{
  if(g_roup)
    return g_roup;
  if(f_older)
   return f_older;

  return 0;
}


void KNArticleManager::setAllRead(bool r)
{
  if(!g_roup)
    return;

  int new_count = 0;
  KNRemoteArticle *a;
  for(int i=0; i<g_roup->length(); i++) {
    a=g_roup->at(i);
    if(a->isRead()!=r) {
      a->setRead(r);
      a->setChanged(true);
      if(a->isNew())
        new_count++;
    }
  }

  g_roup->updateThreadInfo();
  if(r) {
    g_roup->setReadCount(g_roup->length());
    g_roup->setNewCount(0);
  } else {
    g_roup->setReadCount(0);
    g_roup->setNewCount(new_count);
  }

  g_roup->updateListItem();
  showHdrs(true);
}


void KNArticleManager::setRead(KNRemoteArticle::List *l, bool r)
{
  if(l->isEmpty())
    return;

  KNRemoteArticle *a=l->first(), *ref=0;
  KNGroup *g=static_cast<KNGroup*>(a->collection() );
  int changeCnt=0, idRef=0;


  for( ; a; a=l->next()) {

    if(a->isRead()!=r) {
      changeCnt++;
      a->setRead(r);
      a->setChanged(true);
      a->updateListItem();

      idRef=a->idRef();

      while(idRef!=0) {
        ref=g->byId(idRef);
        if(r) {
          ref->decUnreadFollowUps();
          if(a->isNew())
            ref->decNewFollowUps();
        }
        else {
          ref->incUnreadFollowUps();
          if(a->isNew())
            ref->incNewFollowUps();
        }

        if(ref->listItem() &&
           ((ref->unreadFollowUps()==0 || ref->unreadFollowUps()==1) ||
            (ref->newFollowUps()==0 || ref->newFollowUps()==1)))
          ref->updateListItem();

        idRef=ref->idRef();
      }

      if(r) {
        g->incReadCount();
        if(a->isNew())
          g->decNewCount();
      }
      else {
        g->decReadCount();
        if(a->isNew())
          g->incNewCount();
      }
    }
  }

  if(changeCnt>0) {
    g->updateListItem();
    if(g==g_roup)
      updateStatusString();
  }
}


void KNArticleManager::toggleWatched(KNRemoteArticle::List *l)
{
  for(KNRemoteArticle *a=l->first(); a; a=l->next()) {
    if(a->score()==100)
      a->setScore(50);
    else
      a->setScore(100);
    a->updateListItem();
    a->setChanged(true);
  }
}


void KNArticleManager::toggleIgnored(KNRemoteArticle::List *l)
{
  for(KNRemoteArticle *a=l->first(); a; a=l->next()) {
    if(a->score()==0)
      a->setScore(50);
    else
      a->setScore(0);
    a->updateListItem();
    a->setChanged(true);
  }
}


void KNArticleManager::setScore(KNRemoteArticle::List *l, int score)
{
  for(KNRemoteArticle *a=l->first(); a; a=l->next())
    if(a->score()!=score) {
      a->setScore(score);
      a->updateListItem();
      a->setChanged(true);
    }
}


void KNArticleManager::createHdrItem(KNRemoteArticle *a)
{
	a->setListItem(new KNHdrViewItem(v_iew));
  a->setThreadMode(s_howThreads);
  a->initListItem();
}


void KNArticleManager::createThread(KNRemoteArticle *a)
{
	KNRemoteArticle *ref=0;
  int idRef=a->idRef();
  bool found=false;

  while(idRef!=0 && !found) {
    ref=g_roup->byId(idRef);
    found=ref->filterResult();
    idRef=ref->idRef();
  }

  if(found) {
    if(!ref->listItem())
    	createThread(ref);
    a->setListItem(new KNHdrViewItem(ref->listItem()));
  }
  else
    a->setListItem(new KNHdrViewItem(v_iew));

  a->setThreadMode(s_howThreads);
  a->initListItem();
}


void KNArticleManager::updateStatusString()
{
  int displCnt=0;

  if(g_roup) {
    if(f_ilter)
      displCnt=f_ilter->count();
    else
      displCnt=g_roup->count();

    knGlobals.top->setStatusMsg(i18n(" %1 : %2 new , %3 displayed")
                        .arg(g_roup->name()).arg(g_roup->newCount()).arg(displCnt),SB_GROUP);

    if(f_ilter)
      knGlobals.top->setStatusMsg(i18n(" Filter: %1").arg(f_ilter->translatedName()), SB_FILTER);
    else
      knGlobals.top->setStatusMsg(QString::null, SB_FILTER);
  }
  else if(f_older) {
    knGlobals.top->setStatusMsg(i18n(" %1 : %2 displayed")
      .arg(f_older->name()).arg(f_older->count()), SB_GROUP);
    knGlobals.top->setStatusMsg(QString::null, SB_FILTER);
  }
}


void KNArticleManager::slotFilterChanged(KNArticleFilter *f)
{
	f_ilter=f;
  showHdrs();
}


void KNArticleManager::slotSearchDialogDone()
{
  s_earchDlg->hide();
  slotFilterChanged(f_ilterMgr->currentFilter());
}


void KNArticleManager::slotItemExpanded(QListViewItem *p)
{
	int idRef=0, topId=0;
  KNRemoteArticle *art, *ref;
  KNHdrViewItem *hdrItem;


  bool inThread=false;
  KNConfig::ReadNewsGeneral *rng=knGlobals.cfgManager->readNewsGeneral();

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
        art->setThreadMode(rng->showThreads());
        art->initListItem();
      }
      else if(rng->totalExpandThreads()) { //totalExpand
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

  if(rng->totalExpandThreads())
  	hdrItem->expandChildren();
}

//-----------------------------
#include "knarticlemanager.moc"
