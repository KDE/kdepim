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

#include <QByteArray>
#include <QDir>
#include <QFile>

#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kcharsets.h>

#include "articlewidget.h"
#include "knmainwidget.h"
#include "knarticlemanager.h"
#include "kngroupdialog.h"
#include "knnntpaccount.h"
#include "kncleanup.h"
#include "scheduler.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "nntpjobs.h"
#include "resource.h"
#include "knarticlewindow.h"
#include "knmemorymanager.h"
#include "settings.h"

using namespace KNode;


//=================================================================================

// helper classes for the group selection dialog (getting the server's grouplist,
// getting recently created groups)


KNGroupInfo::KNGroupInfo()
{
}


KNGroupInfo::KNGroupInfo(const QString &n_ame, const QString &d_escription, bool n_ewGroup, bool s_ubscribed, KNGroup::Status s_tatus)
  : name(n_ame), description(d_escription), newGroup(n_ewGroup), subscribed(s_ubscribed),
    status(s_tatus)
{
}


KNGroupInfo::~KNGroupInfo()
{
}


bool KNGroupInfo::operator== (const KNGroupInfo &gi2) const
{
  return (name == gi2.name);
}


bool KNGroupInfo::operator< (const KNGroupInfo &gi2) const
{
  return (name < gi2.name);
}


//===============================================================================


KNGroupListData::KNGroupListData()
  : codecForDescriptions(0)
{
  groups = new QList<KNGroupInfo>;
}



KNGroupListData::~KNGroupListData()
{
  delete groups;
}



bool KNGroupListData::readIn(KNJobData *job)
{
  QFile f( path + "groups" );
  QByteArray line;
  int sepPos1,sepPos2;
  QString name,description;
  bool sub;
  KNGroup::Status status=KNGroup::unknown;
  QTime timer;
  uint size=f.size()+2;

  timer.start();
  if(job) {
    job->setProgress(0);
  }

  if(f.open(QIODevice::ReadOnly)) {
    while(!f.atEnd()) {
      line = f.readLine();
      sepPos1 = line.indexOf( ' ' );

      if (sepPos1==-1) {        // no description
        name = QString::fromUtf8(line);
        description.clear();
        status = KNGroup::unknown;
      } else {
        name = QString::fromUtf8(line.left(sepPos1));

        sepPos2 = line.indexOf( ' ', sepPos1 + 1 );
        if (sepPos2==-1) {        // no status
          description = QString::fromUtf8(line.right(line.length()-sepPos1-1));
          status = KNGroup::unknown;
        } else {
          description = QString::fromUtf8( line.right( line.length() - sepPos2 - 1 ).trimmed() );
          switch (line[sepPos1+1]) {
            case 'u':   status = KNGroup::unknown;
                        break;
            case 'n':   status = KNGroup::readOnly;
                        break;
            case 'y':   status = KNGroup::postingAllowed;
                        break;
            case 'm':   status = KNGroup::moderated;
                        break;
          }
        }
      }

      if (subscribed.contains(name)) {
        subscribed.removeAll( name );    // group names are unique, we wont find it again anyway...
        sub = true;
      } else
        sub = false;

      groups->append(KNGroupInfo(name,description,false,sub,status));

      if (timer.elapsed() > 200) {           // don't flicker
        timer.restart();
        if(job) {
          job->setProgress( (f.at()*100)/size );
        }
      }
    }

    f.close();
    return true;
  } else {
    kWarning(5003) <<"unable to open" << f.fileName() <<" reason" << f.status();
    return false;
  }
}



bool KNGroupListData::writeOut()
{
  QFile f(path+"groups");
  QByteArray temp;

  if(f.open(QIODevice::WriteOnly)) {
    Q_FOREACH(const KNGroupInfo& i, *groups) {
      temp = i.name.toUtf8();
      switch (i.status) {
        case KNGroup::unknown: temp += " u ";
                               break;
        case KNGroup::readOnly: temp += " n ";
                                break;
        case KNGroup::postingAllowed: temp += " y ";
                                      break;
        case KNGroup::moderated: temp += " m ";
                                 break;
      }
      temp += i.description.toUtf8() + '\n';
      f.write(temp.data(),temp.length());
    }
    f.close();
    return true;
  } else {
    kWarning(5003) <<"unable to open" << f.fileName() <<" reason" << f.status();
    return false;
  }
}



// merge in new groups, we want to preserve the "subscribed"-flag
// of the loaded groups and the "new"-flag of the new groups.
void KNGroupListData::merge(QList<KNGroupInfo>* newGroups)
{
  bool subscribed;

  Q_FOREACH(const KNGroupInfo& i, *newGroups) {
  int current;
    if ( (current=groups->indexOf(i)) != -1) {
      subscribed = groups->at(current).subscribed;
      groups->removeAt(current);   // avoid duplicates
    } else
      subscribed = false;
      groups->append(KNGroupInfo(i.name,i.description,true,subscribed,i.status));
  }
}


QList<KNGroupInfo>* KNGroupListData::extractList()
{
  QList<KNGroupInfo>* temp = groups;
  groups = 0;
  return temp;
}


//===============================================================================


KNGroupManager::KNGroupManager( QObject * parent )
  : QObject( parent )
{
  c_urrentGroup=0;
  a_rticleMgr = knGlobals.articleManager();
}


KNGroupManager::~KNGroupManager()
{
  qDeleteAll( mGroupList );
}


void KNGroupManager::syncGroups()
{
  for ( KNGroup::List::Iterator it = mGroupList.begin(); it != mGroupList.end(); ++it ) {
    (*it)->syncDynamicData();
    (*it)->saveInfo();
  }
}


void KNGroupManager::loadGroups(KNNntpAccount *a)
{
  KNGroup *group;

  QString dir(a->path());
  if (dir.isNull())
    return;
  QDir d(dir);

  QStringList entries(d.entryList(QStringList("*.grpinfo")));
  for(QStringList::Iterator it=entries.begin(); it != entries.end(); ++it) {
    group=new KNGroup(a);
    if (group->readInfo(dir+(*it))) {
      mGroupList.append( group );
      emit groupAdded(group);
    } else {
      delete group;
      kError(5003) <<"Unable to load" << (*it) <<"!";
    }
  }
}


void KNGroupManager::getSubscribed(KNNntpAccount *a, QStringList &l)
{
  l.clear();
  for ( KNGroup::List::Iterator it = mGroupList.begin(); it != mGroupList.end(); ++it )
    if ( (*it)->account() == a )
      l.append( (*it)->groupname() );
}


KNGroup::List KNGroupManager::groupsOfAccount( KNNntpAccount *a )
{
  KNGroup::List ret;
  for ( KNGroup::List::Iterator it = mGroupList.begin(); it != mGroupList.end(); ++it )
    if ( (*it)->account() == a )
      ret.append( (*it) );
  return ret;
}


bool KNGroupManager::loadHeaders(KNGroup *g)
{
  if (!g)
    return false;

  if (g->isLoaded())
    return true;

  // we want to delete old stuff first => reduce vm fragmentation
  knGlobals.memoryManager()->prepareLoad(g);

  if (g->loadHdrs()) {
    knGlobals.memoryManager()->updateCacheEntry( g );
    return true;
  }

  return false;
}


bool KNGroupManager::unloadHeaders(KNGroup *g, bool force)
{
  if(!g || g->isLocked())
    return false;

  if(!g->isLoaded())
    return true;

  if (!force && (c_urrentGroup == g))
    return false;

  if (g->unloadHdrs(force))
    knGlobals.memoryManager()->removeCacheEntry(g);
  else
    return false;

  return true;
}


KNGroup* KNGroupManager::group(const QString &gName, const KNServerInfo *s)
{
  for ( KNGroup::List::Iterator it = mGroupList.begin(); it != mGroupList.end(); ++it )
    if ( (*it)->account() == s && (*it)->groupname() == gName )
      return (*it);

  return 0;
}


KNGroup* KNGroupManager::firstGroupOfAccount(const KNServerInfo *s)
{
  for ( KNGroup::List::Iterator it = mGroupList.begin(); it != mGroupList.end(); ++it )
    if ( (*it)->account() == s )
      return (*it);

  return 0;
}


void KNGroupManager::expireAll(KNCleanUp *cup)
{
  for ( KNGroup::List::Iterator it = mGroupList.begin(); it != mGroupList.end(); ++it ) {
    if( (*it)->isLocked() || (*it)->lockedArticles() > 0 )
      continue;
    if ( !(*it)->activeCleanupConfig()->expireToday() )
      continue;
    cup->appendCollection( *(it) );
  }
}


void KNGroupManager::expireAll(KNNntpAccount *a)
{
  KNCleanUp *cup = new KNCleanUp();

  for ( KNGroup::List::Iterator it = mGroupList.begin(); it != mGroupList.end(); ++it ) {
    if( (*it)->account() != a  || (*it)->isLocked() || (*it)->lockedArticles() > 0 )
      continue;

    ArticleWindow::closeAllWindowsForCollection( (*it) );
    cup->appendCollection( (*it) );
  }

  cup->start();

  for ( KNGroup::List::Iterator it = mGroupList.begin(); it != mGroupList.end(); ++it ) {
    if( (*it)->account() != a  || (*it)->isLocked() || (*it)->lockedArticles() > 0 )
      continue;

    emit groupUpdated( (*it) );
    if ( (*it) == c_urrentGroup ) {
      if ( loadHeaders( (*it) ) )
        a_rticleMgr->showHdrs();
      else
        a_rticleMgr->setGroup(0);
    }
  }

  delete cup;
}


void KNGroupManager::showGroupDialog(KNNntpAccount *a, QWidget *parent)
{
  KNGroupDialog* gDialog=new KNGroupDialog((parent!=0)? parent:knGlobals.topWidget, a);

  connect(gDialog, SIGNAL(loadList(KNNntpAccount*)), this, SLOT(slotLoadGroupList(KNNntpAccount*)));
  connect(gDialog, SIGNAL(fetchList(KNNntpAccount*)), this, SLOT(slotFetchGroupList(KNNntpAccount*)));
  connect(gDialog, SIGNAL(checkNew(KNNntpAccount*,QDate)), this, SLOT(slotCheckForNewGroups(KNNntpAccount*,QDate)));
  connect(this, SIGNAL(newListReady(KNGroupListData*)), gDialog, SLOT(slotReceiveList(KNGroupListData*)));

  QWidget *oldTopWidget = knGlobals.topWidget;
  // if the list of groups is empty, the parent of the message box
  // asking whether to fetch will be "knGlobals.topWidget"
  knGlobals.topWidget = gDialog;
  int accept = gDialog->exec();
  knGlobals.topWidget = oldTopWidget;
  if(accept) {
    KNGroup *g=0;

    QStringList lst;
    gDialog->toUnsubscribe(&lst);
    if (lst.count()>0) {
      if (KMessageBox::Yes == KMessageBox::questionYesNoList((parent!=0)? parent:knGlobals.topWidget,i18n("Do you really want to unsubscribe\nfrom these groups?"),
                                                              lst, QString(), KGuiItem(i18n("Unsubscribe")), KStandardGuiItem::cancel())) {
        for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
          if((g=group(*it, a)))
            unsubscribeGroup(g);
        }
      }
    }

    QList<KNGroupInfo> lst2;
    gDialog->toSubscribe(&lst2);
    Q_FOREACH( const KNGroupInfo& var, lst2) {
      subscribeGroup(&var, a);
    }
  }

  delete gDialog;
}


void KNGroupManager::subscribeGroup(const KNGroupInfo *gi, KNNntpAccount *a)
{
  KNGroup *grp;

  grp=new KNGroup(a);
  grp->setGroupname(gi->name);
  grp->setDescription(gi->description);
  grp->setStatus(gi->status);
  grp->saveInfo();
  mGroupList.append( grp );
  emit groupAdded(grp);
}


bool KNGroupManager::unsubscribeGroup(KNGroup *g)
{
  KNNntpAccount *acc;
  if(!g) g=c_urrentGroup;
  if(!g) return false;

  if((g->isLocked()) || (g->lockedArticles()>0)) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("The group \"%1\" is being updated currently.\nIt is not possible to unsubscribe from it at the moment.", g->groupname()));
    return false;
  }

  ArticleWindow::closeAllWindowsForCollection( g );
  ArticleWidget::collectionRemoved( g );

  acc=g->account();

  QDir dir( acc->path(), g->groupname() + '*' );
  if (dir.exists()) {
    if (unloadHeaders(g, true)) {
      if(c_urrentGroup==g) {
        setCurrentGroup(0);
        a_rticleMgr->updateStatusString();
      }

      QFileInfoList list = dir.entryInfoList();  // get list of matching files and delete all
      Q_FOREACH( const QFileInfo &it, list ) {
        if ( it.fileName() == g->groupname()+".dynamic" ||
             it.fileName() == g->groupname()+".static" ||
             it.fileName() == g->groupname()+".grpinfo" )
          dir.remove( it.fileName() );
      }
      kDebug(5003) <<"Files deleted!";

      emit groupRemoved(g);
      mGroupList.removeAll( g );
      delete g;

      return true;
    }
  }

  return false;
}


void KNGroupManager::showGroupProperties(KNGroup *g)
{
  if(!g) g=c_urrentGroup;
  if(!g) return;
  g->showProperties();
}


void KNGroupManager::checkGroupForNewHeaders(KNGroup *g)
{
  if(!g) g=c_urrentGroup;
  if(!g) return;
  if(g->isLocked()) {
    kDebug(5003) <<"KNGroupManager::checkGroupForNewHeaders() : group locked - returning";
    return;
  }

  g->setMaxFetch( knGlobals.settings()->maxToFetch() );
  emitJob( new ArticleListJob( this, g->account(), g ) );
}


void KNGroupManager::expireGroupNow(KNGroup *g)
{
  if(!g) return;

  if((g->isLocked()) || (g->lockedArticles()>0)) {
    KMessageBox::sorry(knGlobals.topWidget,
      i18n("This group cannot be expired because it is currently being updated.\n Please try again later."));
    return;
  }

  ArticleWindow::closeAllWindowsForCollection( g );

  KNCleanUp cup;
  cup.expireGroup(g, true);

  emit groupUpdated(g);
  if(g==c_urrentGroup) {
    if( loadHeaders(g) )
      a_rticleMgr->showHdrs();
    else
      a_rticleMgr->setGroup(0);
  }
}


void KNGroupManager::reorganizeGroup(KNGroup *g)
{
  if(!g) g=c_urrentGroup;
  if(!g) return;
  g->reorganize();
  if(g==c_urrentGroup)
    a_rticleMgr->showHdrs();
}


void KNGroupManager::setCurrentGroup(KNGroup *g)
{
  c_urrentGroup=g;
  a_rticleMgr->setGroup(g);
  kDebug(5003) <<"KNGroupManager::setCurrentGroup() : group changed";

  if(g) {
    if( !loadHeaders(g) ) {
      //KMessageBox::error(knGlobals.topWidget, i18n("Cannot load saved headers"));
      return;
    }
    a_rticleMgr->showHdrs();
    if ( knGlobals.settings()->autoCheckGroups() )
      checkGroupForNewHeaders(g);
  }
}


void KNGroupManager::checkAll(KNNntpAccount *a, bool silent)
{
  if(!a) return;

  for ( KNGroup::List::Iterator it = mGroupList.begin(); it != mGroupList.end(); ++it ) {
    if ( (*it)->account() == a ) {
      (*it)->setMaxFetch( knGlobals.settings()->maxToFetch() );
      if ( silent )
        emitJob( new ArticleListJob( this, (*it)->account(), (*it), true ) );
      else
        emitJob( new ArticleListJob( this, (*it)->account(), (*it) ) );
    }
  }
}


void KNGroupManager::processJob(KNJobData *j)
{
  if ( j->type()==KNJobData::JTLoadGroups || j->type()==KNJobData::JTFetchGroups ) {
    KNGroupListData *d=static_cast<KNGroupListData*>(j->data());

    if (!j->canceled()) {
      if (j->success()) {
        if ( j->type() == KNJobData::JTFetchGroups ) {
          // update the descriptions of the subscribed groups
          for ( KNGroup::List::Iterator it = mGroupList.begin(); it != mGroupList.end(); ++it ) {
            if ( (*it)->account() == j->account() ) {
	      Q_FOREACH( const KNGroupInfo& inf, *d->groups )
                if ( inf.name == (*it)->groupname() ) {
                  (*it)->setDescription( inf.description );
                  (*it)->setStatus( inf.status );
                  break;
                }
            }
          }
        }
        emit(newListReady(d));
      } else {
        KMessageBox::error(knGlobals.topWidget, j->errorString());
        emit(newListReady(0));
      }
    } else
      emit(newListReady(0));

    delete j;
    delete d;


  } else {               //KNJobData::JTfetchNewHeaders
    KNGroup *group=static_cast<KNGroup*>(j->data());

    if (!j->canceled()) {
      if (j->success()) {
        if(group->lastFetchCount()>0) {
          group->scoreArticles();
          group->processXPostBuffer(true);
          emit groupUpdated(group);
          group->saveInfo();
          knGlobals.memoryManager()->updateCacheEntry(group);
        }
      } else {
        // ok, hack (?):
        // stop all other active fetch jobs, this prevents that
        // we show multiple error dialogs if a server is unavailable
        knGlobals.scheduler()->cancelJobs( KNJobData::JTfetchNewHeaders );
        ArticleListJob *lj = static_cast<ArticleListJob*>( j );
        if ( !lj->silent() ) {
          KMessageBox::error(knGlobals.topWidget, j->errorString());
        }
      }
    }
    if(group==c_urrentGroup)
      a_rticleMgr->showHdrs(false);

    delete j;
  }
}


// load group list from disk (if this fails: ask user if we should fetch the list)
void KNGroupManager::slotLoadGroupList(KNNntpAccount *a)
{
  KNGroupListData *d = new KNGroupListData();
  d->path = a->path();

  if(!QFileInfo(d->path+"groups").exists()) {
    if (KMessageBox::Yes==KMessageBox::questionYesNo(knGlobals.topWidget,i18n("You do not have any groups for this account;\ndo you want to fetch a current list?"), QString(), KGuiItem(i18n("Fetch List")), KGuiItem(i18n("Do Not Fetch")))) {
      delete d;
      slotFetchGroupList(a);
      return;
    } else {
      emit(newListReady(d));
      delete d;
      return;
    }
  }

  getSubscribed(a,d->subscribed);
  d->getDescriptions = a->fetchDescriptions();

  emitJob( new GroupLoadJob( this, a, d ) );
}


// fetch group list from server
void KNGroupManager::slotFetchGroupList(KNNntpAccount *a)
{
  KNGroupListData *d = new KNGroupListData();
  d->path = a->path();
  getSubscribed(a,d->subscribed);
  d->getDescriptions = a->fetchDescriptions();
  d->codecForDescriptions = KGlobal::charsets()->codecForName( knGlobals.settings()->charset() );

  emitJob( new GroupListJob( this, a, d ) );
}


// check for new groups (created after the given date)
void KNGroupManager::slotCheckForNewGroups(KNNntpAccount *a, QDate date)
{
  KNGroupListData *d = new KNGroupListData();
  d->path = a->path();
  getSubscribed(a,d->subscribed);
  d->getDescriptions = a->fetchDescriptions();
  d->fetchSince = date;
  d->codecForDescriptions = KGlobal::charsets()->codecForName( knGlobals.settings()->charset());

  emitJob( new GroupListJob( this, a, d, true ) );
}


//--------------------------------

#include "kngroupmanager.moc"
