/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "knarticlemanager.h"

#include "utils/scoped_cursor_override.h"

#include <QByteArray>
#include <QList>
#include <krun.h>
#include <kmessagebox.h>
#include <kmimetypetrader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kwindowsystem.h>
#include <ktemporaryfile.h>

#include "articlewidget.h"
#include "knmainwidget.h"
#include "knglobals.h"
#include "utilities.h"
#include "knarticlemanager.h"
#include "kngroupmanager.h"
#include "knsearchdialog.h"
#include "knfiltermanager.h"
#include "knfolder.h"
#include "knarticlefilter.h"
#include "knhdrviewitem.h"
#include "scheduler.h"
#include "knnntpaccount.h"
#include "knscoring.h"
#include "knmemorymanager.h"
#include "knarticlefactory.h"
#include "knarticlewindow.h"
#include "knfoldermanager.h"
#include "headerview.h"
#include "nntpjobs.h"
#include "settings.h"

using namespace KNode;
using namespace KNode::Utilities;


KNArticleManager::KNArticleManager() : QObject(0)
{
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
  for ( QList<KTemporaryFile*>::Iterator it = mTempFiles.begin(); it != mTempFiles.end(); ++it ) {
    delete (*it);
  }
  mTempFiles.clear();
}


void KNArticleManager::saveContentToFile(KMime::Content *c, QWidget *parent)
{
  KNSaveHelper helper(c->contentType()->name(),parent);

  QFile *file = helper.getFile(i18n("Save Attachment"));

  if (file) {
    QByteArray data=c->decodedContent();
    if (file->write(data.data(), data.size()) == -1 )
      KNHelper::displayExternalFileError( parent );
  }
}


void KNArticleManager::saveArticleToFile( KNArticle::Ptr a, QWidget *parent )
{
  QString fName = a->subject()->asUnicodeString();
  QString s = "";

  for ( int i = 0; i < fName.length(); ++i )
    if (fName[i].isLetterOrNumber())
      s.append(fName[i]);
    else
      s.append(' ');
  fName = s.simplified();
  fName.replace(QRegExp("[\\s]"),"_");

  KNSaveHelper helper(fName,parent);
  QFile *file = helper.getFile(i18n("Save Article"));

  if (file) {
    QByteArray tmp=a->encodedContent(false);
    if ( file->write(tmp.data(), tmp.size()) == -1 )
      KNHelper::displayExternalFileError( parent );
  }
}


QString KNArticleManager::saveContentToTemp(KMime::Content *c)
{
  QString path;
  KTemporaryFile* tmpFile;
  KMime::Headers::Base *pathHdr=c->headerByType("X-KNode-Tempfile");  // check for existing temp file

  if(pathHdr) {
    path = pathHdr->asUnicodeString();
    bool found=false;

    // lets see if the tempfile-path is still valid...
    for ( QList<KTemporaryFile*>::Iterator it = mTempFiles.begin(); it != mTempFiles.end(); ++it ) {
      if ( (*it)->fileName() == path ) {
        found = true;
        break;
      }
    }

    if (found)
      return path;
    else
      c->removeHeader("X-KNode-Tempfile");
  }

  tmpFile=new KTemporaryFile();
  if (!tmpFile->open()) {
    KNHelper::displayTempFileError();
    delete tmpFile;
    return QString();
  }

  mTempFiles.append(tmpFile);
  QByteArray data=c->decodedContent();
  tmpFile->write(data.data(), data.size());
  tmpFile->flush();
  path=tmpFile->fileName();
  pathHdr=new KMime::Headers::Generic("X-KNode-Tempfile", c, path, "UTF-8");
  c->setHeader(pathHdr);

  return path;
}


void KNArticleManager::openContent(KMime::Content *c)
{
  QString path=saveContentToTemp(c);
  if(path.isNull()) return;

  KService::Ptr offer = KMimeTypeTrader::self()->preferredService(c->contentType()->mimeType(), "Application");
  KUrl::List lst;
  KUrl url;
  url.setPath(path);
  lst.append(url);

  if (offer)
    KRun::run(*offer, lst, knGlobals.top);
  else
    KRun::displayOpenWithDialog(lst, knGlobals.top);
}


void KNArticleManager::showHdrs(bool clear)
{
  if(!g_roup && !f_older) return;

  bool setFirstChild=true;
  bool showThreads=knGlobals.settings()->showThreads();
  bool expandThreads=knGlobals.settings()->defaultToExpandedThreads();

  if(clear)
    v_iew->clear();

  ScopedCursorOverride cursor( Qt::WaitCursor );
  knGlobals.setStatusMsg(i18n(" Creating list..."));
  knGlobals.top->secureProcessEvents();

  if(g_roup) {
    KNRemoteArticle::Ptr art, ref, current;

    current = boost::static_pointer_cast<KNRemoteArticle>( knGlobals.top->articleViewer()->article() );

    if(current && (current->collection() != g_roup)) {
      current.reset();
      knGlobals.top->articleViewer()->setArticle( KNRemoteArticle::Ptr() );
    }

    if(g_roup->isLocked())
      knGlobals.scheduler()->nntpMutex().lock();

    if(f_ilter)
      f_ilter->doFilter(g_roup);
    else
      for(int i=0; i<g_roup->length(); ++i) {
        art=g_roup->at(i);
        art->setFilterResult(true);
        art->setFiltered(true);
        ref = ( art->idRef() ? g_roup->byId( art->idRef() ) : KNRemoteArticle::Ptr() );
        art->setDisplayedReference(ref);
        if(ref)
          ref->setVisibleFollowUps(true);
      }

    d_isableExpander=true;

    for(int i=0; i<g_roup->length(); ++i) {

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
          art->setListItem( new KNHdrViewItem( v_iew ), art );
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
      knGlobals.scheduler()->nntpMutex().unlock();
  }

  else if (f_older) {

    KNLocalArticle::Ptr art;
    if(f_ilter) {
      f_ilter->doFilter(f_older);
    } else {
      for(int i=0; i<f_older->length(); ++i) {
        art=f_older->at(i);
        art->setFilterResult(true);
      }
    }

    for(int idx=0; idx<f_older->length(); idx++) {
      art=f_older->at(idx);

      if(!art->listItem() &&  art->filterResult()) {
        art->setListItem( new KNHdrViewItem( v_iew ), art );
        art->updateListItem();
      } else if(art->listItem())
        art->updateListItem();
    }

  }

  if(setFirstChild && v_iew->firstChild()) {
    v_iew->setCurrentItem(v_iew->firstChild());
    knGlobals.top->articleViewer()->setArticle( KNArticle::Ptr() );
  }

  knGlobals.setStatusMsg( QString() );
  updateStatusString();
}


void KNArticleManager::updateViewForCollection( KNArticleCollection::Ptr c )
{
  if(g_roup==c || f_older==c)
    showHdrs(false);
}


void KNArticleManager::updateListViewItems()
{
  if(!g_roup && !f_older) return;

  if(g_roup) {
    KNRemoteArticle::Ptr art;

    for(int i=0; i<g_roup->length(); ++i) {
      art=g_roup->at(i);
      if(art->listItem())
        art->updateListItem();
    }
  } else { //folder
    KNLocalArticle::Ptr art;

    for(int idx=0; idx<f_older->length(); idx++) {
      art=f_older->at(idx);
      if(art->listItem())
        art->updateListItem();
    }
  }
}


void KNArticleManager::setAllThreadsOpen(bool b)
{
  KNRemoteArticle::Ptr art;
  if(g_roup) {
    ScopedCursorOverride cursor( Qt::WaitCursor );
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
  }
}


void KNArticleManager::search()
{
  if(s_earchDlg) {
    s_earchDlg->show();
#ifdef Q_OS_UNIX
    KWindowSystem::activateWindow(s_earchDlg->winId());
#endif
  } else {
    s_earchDlg = new SearchDialog( SearchDialog::STgroupSearch, 0 );
    connect(s_earchDlg, SIGNAL(doSearch(KNArticleFilter*)), this,
      SLOT(slotFilterChanged(KNArticleFilter*)));
    connect(s_earchDlg, SIGNAL(dialogDone()), this,
      SLOT(slotSearchDialogDone()));
    s_earchDlg->show();
  }
}


void KNArticleManager::setGroup( KNGroup::Ptr g )
{
  g_roup = g;
  if ( g )
    emit aboutToShowGroup();
}


void KNArticleManager::setFolder( KNFolder::Ptr f )
{
  f_older = f;
  if ( f )
    emit aboutToShowFolder();
}


KNArticleCollection::Ptr KNArticleManager::collection()
{
  if(g_roup)
    return g_roup;
  if(f_older)
   return f_older;

  return KNArticleCollection::Ptr();
}


bool KNArticleManager::loadArticle( KNArticle::Ptr a )
{
  if (!a)
    return false;

  if (a->hasContent())
    return true;

  if (a->isLocked()) {
    if ( a->type() == KNArticle::ATremote )
      return true;   // locked == we are already loading this article...
    else
      return false;
  }

  if ( a->type() == KNArticle::ATremote ) {
    KNGroup::Ptr g = boost::static_pointer_cast<KNGroup>( a->collection() );
    if(g)
      emitJob( new ArticleFetchJob( this, g->account(), a ) );
    else
      return false;
  }
  else { // local article
    KNFolder::Ptr f = boost::static_pointer_cast<KNFolder>( a->collection() );
   if( f && f->loadArticle( boost::static_pointer_cast<KNLocalArticle>( a ) ) )
      knGlobals.memoryManager()->updateCacheEntry(a);
    else
      return false;
  }
  return true;
}


bool KNArticleManager::unloadArticle( KNArticle::Ptr a, bool force )
{
  if(!a || a->isLocked() )
    return false;
  if(!a->hasContent())
    return true;

  if (!force && a->isNotUnloadable())
    return false;

  if ( !force && ( ArticleWidget::articleVisible( a ) ) )
    return false;

  if (!force && ( a->type()== KNArticle::ATlocal ) &&
      ( KNGlobals::self()->articleFactory()->findComposer( boost::static_pointer_cast<KNLocalArticle>( a ) ) != 0 ) )
    return false;

  if ( !ArticleWindow::closeAllWindowsForArticle( a, force ) )
    if (!force)
      return false;

  ArticleWidget::articleRemoved( a );
  if ( a->type() != KNArticle::ATlocal )
    KNGlobals::self()->articleFactory()->deleteComposerForArticle( boost::static_pointer_cast<KNLocalArticle>( a ) );
  a->updateListItem();
  knGlobals.memoryManager()->removeCacheEntry(a);

  return true;
}


void KNArticleManager::copyIntoFolder( KNArticle::List &l, KNFolder::Ptr f )
{
  if(!f) return;

  KNLocalArticle::Ptr loc;
  KNLocalArticle::List l2;

  for ( KNArticle::List::Iterator it = l.begin(); it != l.end(); ++it ) {
    if ( !(*it)->hasContent() )
      continue;
    loc = KNLocalArticle::Ptr( new KNLocalArticle( KNArticleCollection::Ptr() ) );
    loc->setEditDisabled(true);
    loc->setContent( (*it)->encodedContent() );
    loc->parse();
    l2.append(loc);
  }

  if ( !l2.isEmpty() ) {

    f->setNotUnloadable(true);

    if ( !f->isLoaded() && !knGlobals.folderManager()->loadHeaders( f ) ) {
      l2.clear();
      f->setNotUnloadable(false);
      return;
    }

    if( !f->saveArticles( l2 ) ) {
      for ( KNLocalArticle::List::Iterator it = l2.begin(); it != l2.end(); ++it ) {
        if ( (*it)->isOrphant() )
          (*it).reset(); // ok, this is ugly; we simply delete orphant articles
        else
          (*it)->KMime::Content::clear(); // no need to keep them in memory
      }
      KNHelper::displayInternalFileError();
    } else {
      for ( KNLocalArticle::List::Iterator it = l2.begin(); it != l2.end(); ++it )
        (*it)->KMime::Content::clear(); // no need to keep them in memory
      knGlobals.memoryManager()->updateCacheEntry( boost::static_pointer_cast<KNArticleCollection>( f ) );
    }

    f->setNotUnloadable(false);
  }
}


void KNArticleManager::moveIntoFolder( KNLocalArticle::List &l, KNFolder::Ptr f )
{
  if(!f) return;
  kDebug(5003) <<" Target folder:" << f->name();

  f->setNotUnloadable(true);

  if (!f->isLoaded() && !knGlobals.folderManager()->loadHeaders(f)) {
    f->setNotUnloadable(false);
    return;
  }

  if ( f->saveArticles( l ) ) {
    for ( KNLocalArticle::List::Iterator it = l.begin(); it != l.end(); ++it )
      knGlobals.memoryManager()->updateCacheEntry( boost::static_pointer_cast<KNArticle>(*it) );
    knGlobals.memoryManager()->updateCacheEntry( boost::static_pointer_cast<KNArticleCollection>( f ) );
  } else {
    for ( KNLocalArticle::List::Iterator it = l.begin(); it != l.end(); ++it )
      if ( (*it)->isOrphant() )
        (*it).reset(); // ok, this is ugly; we simply delete orphant articles
    KNHelper::displayInternalFileError();
  }

  f->setNotUnloadable(false);
}


bool KNArticleManager::deleteArticles(KNLocalArticle::List &l, bool ask)
{
  if(ask) {
    QStringList lst;
    for ( KNLocalArticle::List::Iterator it = l.begin(); it != l.end(); ++it ) {
      if ( (*it)->isLocked() )
        continue;
      if ( (*it)->subject()->isEmpty() )
        lst << i18n("no subject");
      else
        lst << (*it)->subject()->asUnicodeString();
    }
    if( KMessageBox::Cancel == KMessageBox::warningContinueCancelList(
      knGlobals.topWidget, i18n("Do you really want to delete these articles?"), lst,
        i18n("Delete Articles"), KGuiItem(i18n("&Delete"),"edit-delete")) )
      return false;
  }

  for ( KNLocalArticle::List::Iterator it = l.begin(); it != l.end(); ++it )
    knGlobals.memoryManager()->removeCacheEntry( boost::static_pointer_cast<KNArticle>(*it) );

  KNFolder::Ptr f = boost::static_pointer_cast<KNFolder>( l.first()->collection() );
  if ( f ) {
    f->removeArticles( l, true );
    knGlobals.memoryManager()->updateCacheEntry( boost::static_pointer_cast<KNArticleCollection>( f ) );
    return false; // composers for those articles were already removed in removeArticles
  } else {
    l.clear();
  }

  return true;
}


void KNArticleManager::setAllRead( bool read, int lastcount )
{
  if ( !g_roup )
    return;

  int groupLength = g_roup->length();
  int newCount = g_roup->newCount();
  int readCount = g_roup->readCount();
  int offset = lastcount;

  if ( lastcount > groupLength || lastcount < 0 )
    offset = groupLength;

  KNRemoteArticle::Ptr a;
  for ( int i = groupLength - offset; i < groupLength; ++i ) {
    a = g_roup->at( i );
    if ( a->getReadFlag() != read && !a->isIgnored() ) {
      a->setRead( read );
      a->setChanged( true );
      if ( !read ) {
        readCount--;
        if ( a->isNew() )
          newCount++;
      } else {
        readCount++;
        if ( a->isNew() )
          newCount--;
      }
    }
  }

  g_roup->updateThreadInfo();
  if ( lastcount < 0 && read ) {
    // HACK: try to hide the effects of the ignore/filter new/unread count bug
    g_roup->setReadCount( groupLength );
    g_roup->setNewCount( 0 );
  } else {
    g_roup->setReadCount( readCount );
    g_roup->setNewCount( newCount );
  }

  g_roup->updateListItem();
  showHdrs( true );
}


void KNArticleManager::setRead(KNRemoteArticle::List &l, bool r, bool handleXPosts)
{
  if ( l.isEmpty() )
    return;

  KNRemoteArticle::Ptr ref;
  KNGroup::Ptr g = boost::static_pointer_cast<KNGroup>( l.first()->collection() );
  int changeCnt=0, idRef=0;

  for ( KNRemoteArticle::List::Iterator it = l.begin(); it != l.end(); ++it ) {
    if( r && knGlobals.settings()->markCrossposts() &&
        handleXPosts && (*it)->newsgroups()->isCrossposted() ) {

      QList<QByteArray> groups = (*it)->newsgroups()->groups();
      KNGroup::Ptr targetGroup;
      KNRemoteArticle::Ptr xp;
      KNRemoteArticle::List al;
      QByteArray mid = (*it)->messageID()->as7BitString( false );

      for ( QList<QByteArray>::Iterator it2 = groups.begin(); it2 != groups.end(); ++it2 ) {
        targetGroup = knGlobals.groupManager()->group(*it2, g->account());
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

    else if ( (*it)->getReadFlag() != r ) {
      (*it)->setRead( r );
      (*it)->setChanged( true );
      (*it)->updateListItem();

      if ( !(*it)->isIgnored() ) {
        changeCnt++;
        idRef = (*it)->idRef();

        while ( idRef != 0 ) {
          ref=g->byId(idRef);
          if(r) {
            ref->decUnreadFollowUps();
            if ( (*it)->isNew() )
              ref->decNewFollowUps();
          }
          else {
            ref->incUnreadFollowUps();
            if ( (*it)->isNew() )
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
          if ( (*it)->isNew() )
            g->decNewCount();
        }
        else {
          g->decReadCount();
          if ( (*it)->isNew() )
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
  KNRemoteArticle::Ptr a;
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

  KNRemoteArticle::Ptr a = l.first();
  KNRemoteArticle::Ptr ref;
  bool watch = (!a->isWatched());
  KNGroup::Ptr g = boost::static_pointer_cast<KNGroup>( a->collection() );
  int changeCnt=0, idRef=0;

  for ( KNRemoteArticle::List::Iterator it = l.begin(); it != l.end(); ++it ) {
    if ( (*it)->isIgnored() ) {
      (*it)->setIgnored(false);

      if ( !(*it)->getReadFlag() ) {
        changeCnt++;
        idRef = (*it)->idRef();

        while ( idRef != 0 ) {
          ref=g->byId(idRef);

          ref->incUnreadFollowUps();
          if ( (*it)->isNew() )
            ref->incNewFollowUps();

          if(ref->listItem() &&
             ((ref->unreadFollowUps()==0 || ref->unreadFollowUps()==1) ||
              (ref->newFollowUps()==0 || ref->newFollowUps()==1)))
            ref->updateListItem();

          idRef=ref->idRef();
        }
        g->decReadCount();
        if ( (*it)->isNew() )
          g->incNewCount();
      }
    }

    (*it)->setWatched( watch );
    (*it)->updateListItem();
    (*it)->setChanged( true );
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

  KNRemoteArticle::Ptr ref;
  bool ignore = !l.first()->isIgnored();
  KNGroup::Ptr g = boost::static_pointer_cast<KNGroup>( l.first()->collection() );
  int changeCnt = 0, idRef = 0;

  for ( KNRemoteArticle::List::Iterator it = l.begin(); it != l.end(); ++it ) {
    (*it)->setWatched(false);
    if ( (*it)->isIgnored() != ignore ) {
      (*it)->setIgnored( ignore );

      if ( !(*it)->getReadFlag() ) {
        changeCnt++;
        idRef = (*it)->idRef();

        while ( idRef != 0 ) {
          ref = g->byId( idRef );

          if ( ignore ) {
            ref->decUnreadFollowUps();
            if ( (*it)->isNew() )
              ref->decNewFollowUps();
          } else {
            ref->incUnreadFollowUps();
            if ( (*it)->isNew() )
              ref->incNewFollowUps();
          }

          if(ref->listItem() &&
             ((ref->unreadFollowUps()==0 || ref->unreadFollowUps()==1) ||
              (ref->newFollowUps()==0 || ref->newFollowUps()==1)))
            ref->updateListItem();

          idRef=ref->idRef();
        }

        if ( ignore ) {
          g->incReadCount();
          if ( (*it)->isNew() )
            g->decNewCount();
        } else {
          g->decReadCount();
          if ( (*it)->isNew() )
            g->incNewCount();
        }

      }
    }
    (*it)->updateListItem();
    (*it)->setChanged(true);
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
  if ( l.isEmpty() )
    return;

  KNGroup::Ptr g = boost::static_pointer_cast<KNGroup>( l.first()->collection() );
  KScoringManager *sm = knGlobals.scoringManager();
  sm->initCache(g->groupname());

  for ( KNRemoteArticle::List::Iterator it = l.begin(); it != l.end(); ++it ) {
    int defScore = 0;
    if ( (*it)->isIgnored())
      defScore = knGlobals.settings()->ignoredThreshold();
    else if ( (*it)->isWatched() )
      defScore = knGlobals.settings()->watchedThreshold();
    (*it)->setScore(defScore);

    bool read = (*it)->isRead();

    KNScorableArticle sa( (*it) );
    sm->applyRules(sa);
    (*it)->updateListItem();
    (*it)->setChanged( true );

    if ( !read && (*it)->isRead() != read )
      g_roup->incReadCount();
  }
}


void KNArticleManager::processJob(KNJobData *j)
{
  if(j->type()==KNJobData::JTfetchArticle && !j->canceled()) {
    KNRemoteArticle::Ptr a = boost::static_pointer_cast<KNRemoteArticle>( j->data() );
    if(j->success()) {
      ArticleWidget::articleChanged( a );
      if(!a->isOrphant()) //orphant articles are deleted by the displaying widget
        knGlobals.memoryManager()->updateCacheEntry( boost::static_pointer_cast<KNArticle>( a ) );
      if(a->listItem())
        a->updateListItem();
    } else {
      if ( j->error() == KIO::ERR_DOES_NOT_EXIST ) {
        // article is not available at the server anymore
        QString msgId = a->messageID()->as7BitString( false );
        // strip of '<' and '>'
        msgId = msgId.mid( 1, msgId.length() - 2 );
        ArticleWidget::articleLoadError( a,
            i18n("The article you requested is not available on your news server."
            "<br />You could try to get it from <a href=\"http://groups.google.com/groups?selm=%1\">groups.google.com</a>.",
              msgId ) );
        // mark article as read
        if ( knGlobals.settings()->autoMark() && !a->isOrphant() ) {
          KNRemoteArticle::List l;
          l.append( a );
          setRead( l, true );
        }
      } else
        ArticleWidget::articleLoadError( a, j->errorString() );
    }
  }

  delete j;
}


void KNArticleManager::createThread( KNRemoteArticle::Ptr a )
{
  KNRemoteArticle::Ptr ref = a->displayedReference();

  if(ref) {
    if(!ref->listItem())
      createThread(ref);
    a->setListItem( new KNHdrViewItem( ref->listItem() ), a );
  }
  else
    a->setListItem( new KNHdrViewItem( v_iew ), a );

  a->setThreadMode( knGlobals.settings()->showThreads() );
  a->initListItem();
}


void KNArticleManager::createCompleteThread( KNRemoteArticle::Ptr a )
{
  KNRemoteArticle::Ptr ref = a->displayedReference();
  if ( !ref ) {
    return;
  }

  KNRemoteArticle::Ptr art, top;
  bool inThread=false;
  bool showThreads = knGlobals.settings()->showThreads();

  while (ref->displayedReference() != 0)
    ref=ref->displayedReference();

  top = ref;

  if (!top->listItem())  // shouldn't happen
    return;

  for(int i=0; i<g_roup->count(); ++i) {
    art=g_roup->at(i);
    if(art->filterResult() && !art->listItem()) {

      if(art->displayedReference()==top) {
        art->setListItem( new KNHdrViewItem( top->listItem() ), art );
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

  if ( knGlobals.settings()->totalExpandThreads() )
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

    knGlobals.setStatusMsg(i18n(" %1: %2 new , %3 displayed",
                         name, g_roup->newCount(), displCnt),SB_GROUP);

    if(f_ilter)
      knGlobals.setStatusMsg(i18n(" Filter: %1", f_ilter->translatedName()), SB_FILTER);
    else
      knGlobals.setStatusMsg( QString(), SB_FILTER );
  }
  else if(f_older) {
    if(f_ilter)
      displCnt=f_ilter->count();
    else
      displCnt=f_older->count();
    knGlobals.setStatusMsg(i18n(" %1: %2 displayed",
       f_older->name(), displCnt), SB_GROUP);
    knGlobals.setStatusMsg( QString(), SB_FILTER );
  } else {
    knGlobals.setStatusMsg( QString(), SB_GROUP );
    knGlobals.setStatusMsg( QString(), SB_FILTER );
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


void KNArticleManager::slotItemExpanded(Q3ListViewItem *p)
{
  if (d_isableExpander)  // we don't want to call this method recursively
    return;
  d_isableExpander = true;

  KNRemoteArticle::Ptr top, art, ref;
  KNHdrViewItem *hdrItem;
  bool inThread=false;
  bool showThreads = knGlobals.settings()->showThreads();
  hdrItem=static_cast<KNHdrViewItem*>(p);
  top = boost::static_pointer_cast<KNRemoteArticle>( hdrItem->art );

  if (p->childCount() == 0) {
    ScopedCursorOverride cursor( Qt::WaitCursor );

    for(int i=0; i<g_roup->count(); ++i) {
      art=g_roup->at(i);
      if(art->filterResult() && !art->listItem()) {

        if(art->displayedReference()==top) {
          art->setListItem( new KNHdrViewItem( hdrItem ), art );
          art->setThreadMode(showThreads);
          art->initListItem();
        }
        else if( knGlobals.settings()->totalExpandThreads() ) { //totalExpand
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

    cursor.restore();
  }

  if ( knGlobals.settings()->totalExpandThreads() )
    hdrItem->expandChildren();

  d_isableExpander = false;
}


void KNArticleManager::setView(KNHeaderView* v) {
  v_iew = v;
  if(v) {
    connect(v, SIGNAL(expanded(Q3ListViewItem*)), this,
      SLOT(slotItemExpanded(Q3ListViewItem*)));
  }
}

//-----------------------------
#include "knarticlemanager.moc"
