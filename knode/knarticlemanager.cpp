/*
    knarticlemanager.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <kmessagebox.h>
#include <kuserprofile.h>
#include <kopenwith.h>
#include <klocale.h>
#include <kdebug.h>
#include <kwin.h>
#include <ktempfile.h>

#include "knmainwidget.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "utilities.h"
#include "knarticlemanager.h"
#include "kngroupmanager.h"
#include "knarticlewidget.h"
#include "knsearchdialog.h"
#include "knfiltermanager.h"
#include "knfolder.h"
#include "knarticlefilter.h"
#include "knhdrviewitem.h"
#include "knnetaccess.h"
#include "knnntpaccount.h"
#include "knscoring.h"
#include "knmemorymanager.h"
#include "knarticlefactory.h"
#include "knarticlewindow.h"
#include "knfoldermanager.h"
#include "headerview.h"


KNArticleManager::KNArticleManager() : QObject(0,0)
{
  g_roup=0;
  f_older=0;
  f_ilterMgr = knGlobals.filterManager();
  f_ilter = f_ilterMgr->currentFilter();
  s_earchDlg=0;
  d_isableExpander=false;

  connect(f_ilterMgr, SIGNAL(filterChanged(KNArticleFilter*)), this,
    SLOT(slotFilterChanged(KNArticleFilter*)));
}


KNArticleManager::~KNArticleManager()
{
  delete s_earchDlg;
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


void KNArticleManager::saveContentToFile(KMime::Content *c, QWidget *parent)
{
  KNSaveHelper helper(c->contentType()->name(),parent);

  QFile *file = helper.getFile(i18n("Save Attachment"));

  if (file) {
    QByteArray data=c->decodedContent();
    file->writeBlock(data.data(), data.size());
  }
}


void KNArticleManager::saveArticleToFile(KNArticle *a, QWidget *parent)
{
  QString fName = a->subject()->asUnicodeString();
  QString s = "";

  for (unsigned int i=0; i<fName.length(); i++)
    if (fName[i].isLetterOrNumber())
      s.append(fName[i]);
    else
      s.append(' ');
  fName = s.simplifyWhiteSpace();
  fName.replace(QRegExp("[\\s]"),"_");

  KNSaveHelper helper(fName,parent);
  QFile *file = helper.getFile(i18n("Save Article"));

  if (file) {
    QCString tmp=a->encodedContent(false);
    file->writeBlock(tmp.data(), tmp.size());
  }
}


QString KNArticleManager::saveContentToTemp(KMime::Content *c)
{
  QString path;
  KTempFile* tmpFile;
  KMime::Headers::Base *pathHdr=c->getHeaderByType("X-KNode-Tempfile");  // check for existing temp file

  if(pathHdr) {
    path = pathHdr->asUnicodeString();
    bool found=false;

    // lets see if the tempfile-path is still valid...
    for (tmpFile=t_empFiles.first(); tmpFile; tmpFile=t_empFiles.next())
      if (tmpFile->name()==path) {
        found = true;
        break;
      }

    if (found)
      return path;
    else
      c->removeHeader("X-KNode-Tempfile");
  }

  tmpFile=new KTempFile();
  if (tmpFile->status()!=0) {
    KNHelper::displayTempFileError();
    delete tmpFile;
    return QString::null;
  }

  t_empFiles.append(tmpFile);
  QFile *f=tmpFile->file();
  QByteArray data=c->decodedContent();
  f->writeBlock(data.data(), data.size());
  tmpFile->close();
  path=tmpFile->name();
  pathHdr=new KMime::Headers::Generic("X-KNode-Tempfile", c, path, "UTF-8");
  c->setHeader(pathHdr);

  return path;
}


void KNArticleManager::openContent(KMime::Content *c)
{
  QString path=saveContentToTemp(c);
  if(path.isNull()) return;

  KService::Ptr offer = KServiceTypeProfile::preferredService(c->contentType()->mimeType(), "Application");
  KURL::List lst;
  KURL url;
  url.setPath(path);
  lst.append(url);

  if (offer)
    KRun::run(*offer, lst);
  else
    KRun::displayOpenWithDialog(lst);
}


void KNArticleManager::showHdrs(bool clear)
{
  if(!g_roup && !f_older) return;

  bool setFirstChild=true;
  bool showThreads=knGlobals.configManager()->readNewsGeneral()->showThreads();
  bool expandThreads=knGlobals.configManager()->readNewsGeneral()->defaultToExpandedThreads();

  if(clear)
    v_iew->clear();

  knGlobals.top->setCursorBusy(true);
  knGlobals.setStatusMsg(i18n(" Creating list..."));
  knGlobals.top->secureProcessEvents();

  if(g_roup) {
    KNRemoteArticle *art, *ref, *current;

    current=static_cast<KNRemoteArticle*>(knGlobals.top->articleView()->article());

    if(current && (current->collection() != g_roup)) {
      current=0;
      knGlobals.top->articleView()->setArticle(0);
    }

    if(g_roup->isLocked())
      knGlobals.netAccess()->nntpMutex().lock();

    if(f_ilter)
      f_ilter->doFilter(g_roup);
    else
      for(int i=0; i<g_roup->length(); i++) {
        art=g_roup->at(i);
        art->setFilterResult(true);
        art->setFiltered(true);
        ref=(art->idRef()!=0) ? g_roup->byId(art->idRef()) : 0;
        art->setDisplayedReference(ref);
        if(ref)
          ref->setVisibleFollowUps(true);
      }

    d_isableExpander=true;

    for(int i=0; i<g_roup->length(); i++) {

      art=g_roup->at(i);
      art->setThreadMode(showThreads);

      if(showThreads) {
        art->propagateThreadChangedDate();

        if( !art->listItem() && art->filterResult() ) {

          // ### disable delayed header view item creation for now, it breaks
          // the quick search
          // since it doesn't seem to improve performance at all, it probably
          // could be removed entirely (see also slotItemExpanded(), etc.)
          /*if (!expandThreads) {

            if( (ref=art->displayedReference()) ) {

              if( ref->listItem() && ( ref->listItem()->isOpen() || ref->listItem()->childCount()>0 ) ) {
                art->setListItem(new KNHdrViewItem(ref->listItem()));
                art->initListItem();
              }

            }
            else {
              art->setListItem(new KNHdrViewItem(v_iew));
              art->initListItem();
            }

        } else {  // expandThreads == true */
            createThread(art);
            if ( expandThreads )
              art->listItem()->setOpen(true);
//           }

        }
        else if(art->listItem()) {
          art->updateListItem();
          if (expandThreads)
            art->listItem()->setOpen(true);
        }

      }
      else {

        if(!art->listItem() && art->filterResult()) {
          art->setListItem(new KNHdrViewItem(v_iew));
          art->initListItem();
        } else if(art->listItem())
          art->updateListItem();

      }

    }

    if (current && !current->filterResult()) {   // try to find a parent that is visible
      int idRef;
      while (current && !current->filterResult()) {
        idRef=current->idRef();
        if (idRef == -1)
          break;
        current = g_roup->byId(idRef);
      }
    }

    if(current && current->filterResult()) {
      if(!current->listItem())
        createCompleteThread(current);
      v_iew->setActive( current->listItem() );
      setFirstChild=false;
    }

    d_isableExpander=false;

    if (g_roup->isLocked())
      knGlobals.netAccess()->nntpMutex().unlock();
  }

  else { //folder

    KNLocalArticle *art;
    if(f_ilter) {
      f_ilter->doFilter(f_older);
    } else {
      for(int i=0; i<f_older->length(); i++) {
        art=f_older->at(i);
        art->setFilterResult(true);
      }
    }

    for(int idx=0; idx<f_older->length(); idx++) {
      art=f_older->at(idx);

      if(!art->listItem() &&  art->filterResult()) {
        art->setListItem( new KNHdrViewItem(v_iew, art) );
        art->updateListItem();
      } else if(art->listItem())
        art->updateListItem();
    }

  }

  if(setFirstChild && v_iew->firstChild()) {
    v_iew->setCurrentItem(v_iew->firstChild());
    knGlobals.top->articleView()->setArticle(0);
  }

  knGlobals.setStatusMsg(QString::null);
  updateStatusString();
  knGlobals.top->setCursorBusy(false);
}


void KNArticleManager::updateViewForCollection(KNArticleCollection *c)
{
  if(g_roup==c || f_older==c)
    showHdrs(false);
}


void KNArticleManager::updateListViewItems()
{
  if(!g_roup && !f_older) return;

  if(g_roup) {
    KNRemoteArticle *art;

    for(int i=0; i<g_roup->length(); i++) {
      art=g_roup->at(i);
      if(art->listItem())
        art->updateListItem();
    }
  } else { //folder
    KNLocalArticle *art;

    for(int idx=0; idx<f_older->length(); idx++) {
      art=f_older->at(idx);
      if(art->listItem())
        art->updateListItem();
    }
  }
}


void KNArticleManager::setAllThreadsOpen(bool b)
{
  KNRemoteArticle *art;
  if(g_roup) {
    knGlobals.top->setCursorBusy(true);
    d_isableExpander = true;
    for(int idx=0; idx<g_roup->length(); idx++) {
      art = g_roup->at(idx);
      if (art->listItem())
        art->listItem()->setOpen(b);
      else
        if (b && art->filterResult()) {
          createThread(art);
          art->listItem()->setOpen(true);
        }
    }
    d_isableExpander = false;
    knGlobals.top->setCursorBusy(false);
  }
}


void KNArticleManager::search()
{
  if(s_earchDlg) {
    s_earchDlg->show();
    KWin::activateWindow(s_earchDlg->winId());
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
  g_roup = g;
  if ( g )
    emit aboutToShowGroup();
}


void KNArticleManager::setFolder(KNFolder *f)
{
  f_older = f;
  if ( f )
    emit aboutToShowFolder();
}


KNArticleCollection* KNArticleManager::collection()
{
  if(g_roup)
    return g_roup;
  if(f_older)
   return f_older;

  return 0;
}


bool KNArticleManager::loadArticle(KNArticle *a)
{
  if (!a)
    return false;

  if (a->hasContent())
    return true;

  if (a->isLocked()) {
    if (a->type()==KMime::Base::ATremote)
      return true;   // locked == we are already loading this article...
    else
      return false;
  }

  if(a->type()==KMime::Base::ATremote) {
    KNGroup *g=static_cast<KNGroup*>(a->collection());
    if(g)
      emitJob( new KNJobData(KNJobData::JTfetchArticle, this, g->account(), a) );
    else
      return false;
  }
  else { // local article
    KNFolder *f=static_cast<KNFolder*>(a->collection());
   if( f && f->loadArticle( static_cast<KNLocalArticle*>(a) ) )
      knGlobals.memoryManager()->updateCacheEntry(a);
    else
      return false;
  }
  return true;
}


bool KNArticleManager::unloadArticle(KNArticle *a, bool force)
{
  if(!a || a->isLocked() )
    return false;
  if(!a->hasContent())
    return true;

  if (!force && a->isNotUnloadable())
    return false;

  if (!force && KNArticleWidget::articleVisible(a))
    return false;

  if (!force && (a->type()==KMime::Base::ATlocal) &&
      (knGlobals.artFactory->findComposer(static_cast<KNLocalArticle*>(a))!=0))
    return false;

  if (!KNArticleWindow::closeAllWindowsForArticle(a, force))
    if (!force)
      return false;

  KNArticleWidget::articleRemoved(a);
  if (!a->type()==KMime::Base::ATlocal)
    knGlobals.artFactory->deleteComposerForArticle(static_cast<KNLocalArticle*>(a));
  a->KMime::Content::clear();
  a->updateListItem();
  knGlobals.memoryManager()->removeCacheEntry(a);

  return true;
}


void KNArticleManager::copyIntoFolder(KNArticle::List &l, KNFolder *f)
{
  if(!f) return;

  KNArticle *org;
  KNLocalArticle *loc;
  KNLocalArticle::List l2;

  for(org=l.first(); org; org=l.next()) {
    if(!org->hasContent())
      continue;
    loc=new KNLocalArticle(0);
    loc->setEditDisabled(true);
    loc->setContent(org->encodedContent());
    loc->parse();
    l2.append(loc);
  }

  if(!l2.isEmpty()) {

    f->setNotUnloadable(true);

    if (!f->isLoaded() && !knGlobals.folderManager()->loadHeaders(f)) {
      l2.setAutoDelete(true);
      l2.clear();
      f->setNotUnloadable(false);
      return;
    }

    if(!f->saveArticles(&l2)) {
      for(KNLocalArticle *a=l2.first(); a; a=l2.next()) {
        if(a->isOrphant())
          delete a; // ok, this is ugly; we simply delete orphant articles
        else
          a->KMime::Content::clear(); // no need to keep them in memory
      }
      KNHelper::displayInternalFileError();
    } else {
      for(KNLocalArticle *a=l2.first(); a; a=l2.next())
        a->KMime::Content::clear(); // no need to keep them in memory
      knGlobals.memoryManager()->updateCacheEntry(f);
    }

    f->setNotUnloadable(false);
  }
}


void KNArticleManager::moveIntoFolder(KNLocalArticle::List &l, KNFolder *f)
{
  if(!f) return;
  kdDebug(5003) << k_funcinfo << " Target folder: " << f->name() << endl;

  f->setNotUnloadable(true);

  if (!f->isLoaded() && !knGlobals.folderManager()->loadHeaders(f)) {
    f->setNotUnloadable(false);
    return;
  }

  if(f->saveArticles(&l)) {
    for(KNLocalArticle *a=l.first(); a; a=l.next())
      knGlobals.memoryManager()->updateCacheEntry( a );
    knGlobals.memoryManager()->updateCacheEntry(f);
  } else {
    for(KNLocalArticle *a=l.first(); a; a=l.next())
      if(a->isOrphant())
        delete a; // ok, this is ugly; we simply delete orphant articles
    KNHelper::displayInternalFileError();
  }

  f->setNotUnloadable(false);
}


bool KNArticleManager::deleteArticles(KNLocalArticle::List &l, bool ask)
{
  if(ask) {
    QStringList lst;
    for(KNLocalArticle *a=l.first(); a; a=l.next()) {
      if(a->isLocked())
        continue;
      if(a->subject()->isEmpty())
        lst << i18n("no subject");
      else
        lst << a->subject()->asUnicodeString();
    }
    if( KMessageBox::Cancel == KMessageBox::warningContinueCancelList(
      knGlobals.topWidget, i18n("Do you really want to delete these articles?"), lst,
        i18n("Delete Articles"), KGuiItem(i18n("&Delete"),"editdelete")) )
      return false;
  }

  for(KNLocalArticle *a=l.first(); a; a=l.next())
    knGlobals.memoryManager()->removeCacheEntry(a);

  KNFolder *f=static_cast<KNFolder*>(l.first()->collection());
  if(f) {
    f->removeArticles(&l, true);
    knGlobals.memoryManager()->updateCacheEntry( f );
  }
  else {
    for(KNLocalArticle *a=l.first(); a; a=l.next())
      delete a;
  }

  return true;
}


void KNArticleManager::setAllRead(bool r)
{
  if(!g_roup)
    return;

  int new_count = 0, ignore_count=0;
  KNRemoteArticle *a;
  for(int i=0; i<g_roup->length(); i++) {
    a=g_roup->at(i);
    if(a->getReadFlag()!=r) {
      a->setRead(r);
      a->setChanged(true);
      if(a->isNew() && !a->isIgnored())
        new_count++;
      if(a->isIgnored())
        ignore_count++;
    }
  }

  g_roup->updateThreadInfo();
  if(r) {
    g_roup->setReadCount(g_roup->length());
    g_roup->setNewCount(0);
  } else {
    g_roup->setReadCount(ignore_count);
    g_roup->setNewCount(new_count);
  }

  g_roup->updateListItem();
  showHdrs(true);
}


void KNArticleManager::setAllRead(int lastcount, bool r)
{
  if(!g_roup)
    return;

  int groupLength=g_roup->length();
  int newCount=g_roup->newCount();
  int readCount=g_roup->readCount();

  if(lastcount>groupLength)
    lastcount=groupLength;

  KNRemoteArticle *a;
  for(int i=groupLength-lastcount; i<groupLength; i++) {
    a=g_roup->at(i);
    if(a->getReadFlag()!=r) {
      a->setRead(r);
      a->setChanged(true);
      if(!r) {
        readCount--;
        if (a->isNew() && !a->isIgnored())
          newCount++;
      }
      else {
        readCount++;
        if (a->isNew() && !a->isIgnored())
          newCount--;
      }
    }
  }

  g_roup->updateThreadInfo();
  g_roup->setReadCount(readCount);
  g_roup->setNewCount(newCount);

  g_roup->updateListItem();
  showHdrs(true);
}


void KNArticleManager::setRead(KNRemoteArticle::List &l, bool r, bool handleXPosts)
{
  if(l.isEmpty())
    return;

  KNRemoteArticle *a=l.first(), *ref=0;
  KNGroup *g=static_cast<KNGroup*>(a->collection() );
  int changeCnt=0, idRef=0;

  for( ; a; a=l.next()) {
    if( r && knGlobals.configManager()->readNewsGeneral()->markCrossposts() &&
        handleXPosts && a->newsgroups()->isCrossposted() ) {

      QStringList groups = a->newsgroups()->getGroups();
      KNGroup *targetGroup=0;
      KNRemoteArticle *xp=0;
      KNRemoteArticle::List al;
      QCString mid=a->messageID()->as7BitString(false);

      for (QStringList::Iterator it = groups.begin(); it != groups.end(); ++it) {
        targetGroup = knGlobals.groupManager()->group(*it, g->account());
        if (targetGroup) {
          if (targetGroup->isLoaded() && (xp=targetGroup->byMessageId(mid)) ) {
            al.clear();
            al.append(xp);
            setRead(al, r, false);
          } else {
            targetGroup->appendXPostID(mid);
          }
        }
      }
    }

    else if(a->getReadFlag()!=r) {
      a->setRead(r);
      a->setChanged(true);
      a->updateListItem();

      if (!a->isIgnored()) {
        changeCnt++;
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
  }

  if(changeCnt>0) {
    g->updateListItem();
    if(g==g_roup)
      updateStatusString();
  }
}


void KNArticleManager::setAllNotNew()
{
  if ( !g_roup )
    return;
  kdDebug(5003) << k_funcinfo << "begin" << endl;
  KNRemoteArticle *a;
  for ( int i = 0; i < g_roup->length(); ++i) {
    a = g_roup->at(i);
    if ( a->isNew() ) {
      a->setNew( false );
      a->setChanged( true );
    }
  }
  g_roup->setFirstNewIndex( -1 );
  g_roup->setNewCount( 0 );
  g_roup->updateThreadInfo();
}


bool KNArticleManager::toggleWatched(KNRemoteArticle::List &l)
{
  if(l.isEmpty())
    return true;

  KNRemoteArticle *a=l.first(), *ref=0;
  bool watch = (!a->isWatched());
  KNGroup *g=static_cast<KNGroup*>(a->collection() );
  int changeCnt=0, idRef=0;

  for(KNRemoteArticle *a=l.first(); a; a=l.next()) {
    if (a->isIgnored()) {
      a->setIgnored(false);

      if (!a->getReadFlag()) {
        changeCnt++;
        idRef=a->idRef();

        while(idRef!=0) {
          ref=g->byId(idRef);

          ref->incUnreadFollowUps();
          if(a->isNew())
            ref->incNewFollowUps();

          if(ref->listItem() &&
             ((ref->unreadFollowUps()==0 || ref->unreadFollowUps()==1) ||
              (ref->newFollowUps()==0 || ref->newFollowUps()==1)))
            ref->updateListItem();

          idRef=ref->idRef();
        }
        g->decReadCount();
        if(a->isNew())
          g->incNewCount();
      }
    }

    a->setWatched(watch);
    a->updateListItem();
    a->setChanged(true);
  }

  if(changeCnt>0) {
    g->updateListItem();
    if(g==g_roup)
      updateStatusString();
  }

  return watch;
}


bool KNArticleManager::toggleIgnored(KNRemoteArticle::List &l)
{
  if(l.isEmpty())
    return true;

  KNRemoteArticle *a=l.first(), *ref=0;
  bool ignore=(!a->isIgnored());
  KNGroup *g=static_cast<KNGroup*>(a->collection() );
  int changeCnt=0, idRef=0;

  for(; a; a=l.next()) {
    a->setWatched(false);
    if (a->isIgnored() != ignore) {
      a->setIgnored(ignore);

      if (!a->getReadFlag()) {
        changeCnt++;
        idRef=a->idRef();

        while(idRef!=0) {
          ref=g->byId(idRef);

          if(ignore) {
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

        if(ignore) {
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
    a->updateListItem();
    a->setChanged(true);
  }

  if(changeCnt>0) {
    g->updateListItem();
    if(g==g_roup)
      updateStatusString();
  }

  return ignore;
}


void  KNArticleManager::rescoreArticles(KNRemoteArticle::List &l)
{
  KNRemoteArticle *a=l.first();

  if (a) {
    KNGroup *g=static_cast<KNGroup*>(a->collection());
    KScoringManager *sm = knGlobals.scoringManager();
    sm->initCache(g->groupname());

    for(; a; a=l.next()) {
      int defScore = 0;
      if (a->isIgnored())
        defScore = knGlobals.configManager()->scoring()->ignoredThreshold();
      else if (a->isWatched())
        defScore = knGlobals.configManager()->scoring()->watchedThreshold();
      a->setScore(defScore);

      KNScorableArticle sa(a);
      sm->applyRules(sa);
      a->updateListItem();
      a->setChanged(true);
    }
  }
}


void KNArticleManager::processJob(KNJobData *j)
{
  if(j->type()==KNJobData::JTfetchArticle && !j->canceled()) {
    if(j->success()) {
      KNRemoteArticle *a=static_cast<KNRemoteArticle*>(j->data());
      KNArticleWidget::articleChanged(a);
      if(!a->isOrphant()) //orphant articles are deleted by the displaying widget
        knGlobals.memoryManager()->updateCacheEntry(a);
      if(a->listItem())
        a->updateListItem();
    }
    else
      KNArticleWidget::articleLoadError(static_cast<KNRemoteArticle*>(j->data()), j->errorString());
  }

  delete j;
}


void KNArticleManager::createThread(KNRemoteArticle *a)
{
  KNRemoteArticle *ref=a->displayedReference();

  if(ref) {
    if(!ref->listItem())
      createThread(ref);
    a->setListItem(new KNHdrViewItem(ref->listItem()));
  }
  else
    a->setListItem(new KNHdrViewItem(v_iew));

  a->setThreadMode(knGlobals.configManager()->readNewsGeneral()->showThreads());
  a->initListItem();
}


void KNArticleManager::createCompleteThread(KNRemoteArticle *a)
{
  KNRemoteArticle *ref=a->displayedReference(), *art, *top;
  bool inThread=false;
  bool showThreads=knGlobals.configManager()->readNewsGeneral()->showThreads();
  KNConfig::ReadNewsGeneral *rng=knGlobals.configManager()->readNewsGeneral();

  while (ref->displayedReference() != 0)
    ref=ref->displayedReference();

  top = ref;

  if (!top->listItem())  // shouldn't happen
    return;

  for(int i=0; i<g_roup->count(); i++) {
    art=g_roup->at(i);
    if(art->filterResult() && !art->listItem()) {

      if(art->displayedReference()==top) {
        art->setListItem(new KNHdrViewItem(top->listItem()));
        art->setThreadMode(showThreads);
        art->initListItem();
      }
      else {
        ref=art->displayedReference();
        inThread=false;
        while(ref && !inThread) {
          inThread=(ref==top);
          ref=ref->displayedReference();
        }
        if(inThread)
          createThread(art);
      }
    }
  }

  if(rng->totalExpandThreads())
    top->listItem()->expandChildren();
}


void KNArticleManager::updateStatusString()
{
  int displCnt=0;

  if(g_roup) {
    if(f_ilter)
      displCnt=f_ilter->count();
    else
      displCnt=g_roup->count();

    QString name = g_roup->name();
    if (g_roup->status()==KNGroup::moderated)
      name += i18n(" (moderated)");

    knGlobals.setStatusMsg(i18n(" %1: %2 new , %3 displayed")
                        .arg(name).arg(g_roup->newCount()).arg(displCnt),SB_GROUP);

    if(f_ilter)
      knGlobals.setStatusMsg(i18n(" Filter: %1").arg(f_ilter->translatedName()), SB_FILTER);
    else
      knGlobals.setStatusMsg(QString::null, SB_FILTER);
  }
  else if(f_older) {
    if(f_ilter)
      displCnt=f_ilter->count();
    else
      displCnt=f_older->count();
    knGlobals.setStatusMsg(i18n(" %1: %2 displayed")
      .arg(f_older->name()).arg(displCnt), SB_GROUP);
    knGlobals.setStatusMsg(QString::null, SB_FILTER);
  } else {
    knGlobals.setStatusMsg(QString::null, SB_GROUP);
    knGlobals.setStatusMsg(QString::null, SB_FILTER);
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
  if (d_isableExpander)  // we don't want to call this method recursively
    return;
  d_isableExpander = true;

  KNRemoteArticle *top, *art, *ref;
  KNHdrViewItem *hdrItem;
  bool inThread=false;
  bool showThreads=knGlobals.configManager()->readNewsGeneral()->showThreads();
  KNConfig::ReadNewsGeneral *rng=knGlobals.configManager()->readNewsGeneral();
  hdrItem=static_cast<KNHdrViewItem*>(p);
  top=static_cast<KNRemoteArticle*>(hdrItem->art);

  if (p->childCount() == 0) {

    knGlobals.top->setCursorBusy(true);

    for(int i=0; i<g_roup->count(); i++) {
      art=g_roup->at(i);
      if(art->filterResult() && !art->listItem()) {

        if(art->displayedReference()==top) {
          art->setListItem(new KNHdrViewItem(hdrItem));
          art->setThreadMode(showThreads);
          art->initListItem();
        }
        else if(rng->totalExpandThreads()) { //totalExpand
          ref=art->displayedReference();
          inThread=false;
          while(ref && !inThread) {
            inThread=(ref==top);
            ref=ref->displayedReference();
          }
          if(inThread)
            createThread(art);
        }
      }
    }

    knGlobals.top->setCursorBusy(false);
  }

  if(rng->totalExpandThreads())
    hdrItem->expandChildren();

  d_isableExpander = false;
}


void KNArticleManager::setView(KNHeaderView* v) {
  v_iew = v;
  if(v) {
    connect(v, SIGNAL(expanded(QListViewItem*)), this,
      SLOT(slotItemExpanded(QListViewItem*)));
  }
}

//-----------------------------
#include "knarticlemanager.moc"
