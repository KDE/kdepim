/*
    kngroupmanager.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <stdio.h>
#include <stdlib.h>
#include <qheader.h>
#include <qdir.h>

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kurl.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "knode.h"
#include "knarticlemanager.h"
#include "knjobdata.h"
#include "kngroupdialog.h"
#include "kngroup.h"
#include "kncollectionviewitem.h"
#include "knnntpaccount.h"
#include "knprotocolclient.h"
#include "kncleanup.h"
#include "knnetaccess.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "resource.h"
#include "utilities.h"
#include "knarticlewindow.h"
#include "knarticlewidget.h"
#include "kngroupmanager.h"


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


bool KNGroupInfo::operator== (const KNGroupInfo &gi2)
{
  return (name == gi2.name);
}


bool KNGroupInfo::operator< (const KNGroupInfo &gi2)
{
  return (name < gi2.name);
}


//===============================================================================


KNGroupListData::KNGroupListData()
{
  groups = new QSortedList<KNGroupInfo>;
  groups->setAutoDelete(true);
}



KNGroupListData::~KNGroupListData()
{
  delete groups;
}



bool KNGroupListData::readIn(KNProtocolClient *client)
{
  KNFile f(path+"groups");
  QCString line;
  int sepPos1,sepPos2;
  QString name,description;
  bool sub;
  KNGroup::Status status=KNGroup::unknown;
  QTime timer;
  uint size=f.size()+2;

  timer.start();
  if (client) client->updatePercentage(0);

  if(f.open(IO_ReadOnly)) {
    while(!f.atEnd()) {
      line = f.readLine();
      sepPos1 = line.find(' ');

      if (sepPos1==-1) {        // no description
        name = QString::fromUtf8(line);
        description = QString::null;
        status = KNGroup::unknown;
      } else {
        name = QString::fromUtf8(line.left(sepPos1));

        sepPos2 = line.find(' ',sepPos1+1);
        if (sepPos2==-1) {        // no status
          description = QString::fromUtf8(line.right(line.length()-sepPos1-1));
          status = KNGroup::unknown;
        } else {
          description = QString::fromUtf8(line.right(line.length()-sepPos2-1));
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
        subscribed.remove(name);    // group names are unique, we wont find it again anyway...
        sub = true;
      } else
        sub = false;

      groups->append(new KNGroupInfo(name,description,false,sub,status));

      if (timer.elapsed() > 200) {           // don't flicker
        timer.restart();
        if (client) client->updatePercentage((f.at()*100)/size);
      }
    }

    f.close();
    return true;
  } else {
    kdWarning(5003) << "unable to open " << f.name() << " reason " << f.status() << endl;
    return false;
  }
}



bool KNGroupListData::writeOut()
{
  QFile f(path+"groups");
  QCString temp;

  if(f.open(IO_WriteOnly)) {
    for (KNGroupInfo *i=groups->first(); i; i=groups->next()) {
      temp = i->name.utf8();
      switch (i->status) {
        case KNGroup::unknown: temp += " u ";
                               break;
        case KNGroup::readOnly: temp += " n ";
                                break;
        case KNGroup::postingAllowed: temp += " y ";
                                      break;
        case KNGroup::moderated: temp += " m ";
                                 break;
      }
      temp += i->description.utf8() + "\n";
      f.writeBlock(temp.data(),temp.length());
    }
    f.close();
    return true;
  } else {
    kdWarning(5003) << "unable to open " << f.name() << " reason " << f.status() << endl;
    return false;
  } 
}



// merge in new groups, we want to preserve the "subscribed"-flag
// of the loaded groups and the "new"-flag of the new groups.
void KNGroupListData::merge(QSortedList<KNGroupInfo>* newGroups)
{
  bool subscribed;

  for (KNGroupInfo *i=newGroups->first(); i; i=newGroups->next()) {
    if (groups->find(i)>=0) {
      subscribed = groups->current()->subscribed;
      groups->remove();   // avoid duplicates
    } else
      subscribed = false;
    groups->append(new KNGroupInfo(i->name,i->description,true,subscribed,i->status));
  }   

  groups->sort();
}


QSortedList<KNGroupInfo>* KNGroupListData::extractList()
{
  QSortedList<KNGroupInfo>* temp = groups;
  groups = 0;
  return temp;
}


//===============================================================================



KNGroupManager::KNGroupManager(KNArticleManager *a, QObject * parent, const char * name)
  : QObject(parent,name)
{
  g_List=new QList<KNGroup>;
  g_List->setAutoDelete(true);
  c_urrentGroup=0;
  a_rticleMgr=a;
}



KNGroupManager::~KNGroupManager()
{
  syncGroups();
  delete g_List;
}


void KNGroupManager::syncGroups()
{
  for(KNGroup *var=g_List->first(); var; var=g_List->next()) {
    var->syncDynamicData();
    var->saveInfo();
  }
}



void KNGroupManager::loadGroups(KNNntpAccount *a)
{   
  KNGroup *group;
  KNConfig::Appearance *app=knGlobals.cfgManager->appearance();

  QString dir(a->path());
  if (dir == QString::null)
    return;
  QDir d(dir);  
    
  QStringList entries(d.entryList("*.grpinfo"));
  for(QStringList::Iterator it=entries.begin(); it != entries.end(); it++) {
    group=new KNGroup(a);
    if (group->readInfo(dir+(*it))) {
      g_List->append(group);
      KNCollectionViewItem *cvit=new KNCollectionViewItem(a->listItem());
      cvit->setPixmap(0, app->icon(KNConfig::Appearance::group));
      group->setListItem(cvit);
      group->updateListItem();
    } else {
      delete group;
      kdError(5003) << "Unable to load " << (*it) << "!" << endl;
    } 
  }
}



void KNGroupManager::getSubscribed(KNNntpAccount *a, QStringList &l)
{
  l.clear();
  for(KNGroup *var=g_List->first(); var; var=g_List->next()) {
    if(var->account()==a) l.append(var->groupname());
  }
}



void KNGroupManager::getGroupsOfAccount(KNNntpAccount *a, QList<KNGroup> *l)
{
  l->clear();
  for(KNGroup *var=g_List->first(); var; var=g_List->next()) {
    if(var->account()==a) l->append(var);
  }
}



KNGroup* KNGroupManager::group(const QString &gName, const KNServerInfo *s)
{
  for(KNGroup *var=g_List->first(); var; var=g_List->next())
    if(var->account()==s && var->groupname()==gName) return var;

  return 0;
}



void KNGroupManager::expireAll(KNCleanUp *cup)
{
  for(KNGroup *var=g_List->first(); var; var=g_List->next()) {
    if((var->isLocked()) || (var->lockedArticles()>0))
      continue;

    cup->appendCollection(var);
  }
}



void KNGroupManager::showGroupDialog(KNNntpAccount *a, QWidget *parent)
{
  KNGroupDialog* gDialog=new KNGroupDialog((parent!=0)? parent:knGlobals.topWidget, a);

  connect(gDialog, SIGNAL(loadList(KNNntpAccount*)), this, SLOT(slotLoadGroupList(KNNntpAccount*)));
  connect(gDialog, SIGNAL(fetchList(KNNntpAccount*)), this, SLOT(slotFetchGroupList(KNNntpAccount*)));
  connect(gDialog, SIGNAL(checkNew(KNNntpAccount*,QDate)), this, SLOT(slotCheckForNewGroups(KNNntpAccount*,QDate)));
  connect(this, SIGNAL(newListReady(KNGroupListData*)), gDialog, SLOT(slotReceiveList(KNGroupListData*)));

  if(gDialog->exec()) {
    KNGroup *g=0;

    QStringList lst;
    gDialog->toUnsubscribe(&lst);
    if (lst.count()>0) {
      if (KMessageBox::Yes == KMessageBox::questionYesNoList((parent!=0)? parent:knGlobals.topWidget,i18n("Do you really want to unsubscribe\nfrom these groups?"),
                                                              lst)) {
        for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
          if((g=group(*it, a)))
            unsubscribeGroup(g);
        }
      }
    }

    QSortedList<KNGroupInfo> lst2;
    gDialog->toSubscribe(&lst2);
    for(KNGroupInfo *var=lst2.first(); var; var=lst2.next()) {
      subscribeGroup(var, a);
    }
  }

  delete gDialog;
}



void KNGroupManager::subscribeGroup(const KNGroupInfo *gi, KNNntpAccount *a)
{
  KNGroup *grp;
  KNCollectionViewItem *it;

  grp=new KNGroup(a);
  grp->setGroupname(gi->name);
  grp->setDescription(gi->description);
  grp->setStatus(gi->status);
  grp->saveInfo();
  g_List->append(grp);
  it=new KNCollectionViewItem(a->listItem());
  it->setPixmap(0,UserIcon("group"));
  grp->setListItem(it);
  grp->updateListItem();
}



void KNGroupManager::unsubscribeGroup(KNGroup *g)
{
  KNNntpAccount *acc;
  if(!g) g=c_urrentGroup;
  if(!g) return;


  if((g->isLocked()) || (g->lockedArticles()>0)) {
    KMessageBox::sorry(knGlobals.topWidget, QString(i18n("The group \"%1\" is being updated currently.\nIt is not possible to unsubscribe it at the moment.")).arg(g->groupname()));
    return;
  }

  KNArticleWindow::closeAllWindowsForCollection(g);
  KNArticleWidget::collectionRemoved(g);

  acc=g->account();

  QDir dir(acc->path(),g->groupname()+"*");
  if (dir.exists()) {
    const QFileInfoList *list = dir.entryInfoList();  // get list of matching files and delete all
    if (list) {
      QFileInfoListIterator it( *list );
      while (it.current()) {
        dir.remove(it.current()->fileName());
        ++it;
      }
    }
    kdDebug(5003) << "Files deleted!" << endl;

    if(c_urrentGroup==g) setCurrentGroup(0);

    g_List->removeRef(g);
  }
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
    kdDebug(5003) << "KNGroupManager::setCurrentGroup() : group locked - returning" << endl;
    return;
  }
  g->setMaxFetch(knGlobals.cfgManager->readNewsGeneral()->maxToFetch());
  emitJob( new KNJobData(KNJobData::JTfetchNewHeaders, this, g->account(), g) );
}



void KNGroupManager::expireGroupNow(KNGroup *g)
{
  if(!g) return;

  if((g->isLocked()) || (g->lockedArticles()>0)) {
    KMessageBox::sorry(knGlobals.topWidget,
      i18n("This group cannot be expired because it is currently being updated.\n Please try again later."));
    return;
  }

  KNArticleWindow::closeAllWindowsForCollection(g);

  KNCleanUp cup(knGlobals.cfgManager->cleanup());
  cup.expireGroup(g, true);

  g->updateListItem();
  if(g==c_urrentGroup) {
    if(g->loadHdrs())
      a_rticleMgr->showHdrs();
    else
      a_rticleMgr->setGroup(0);
  }
}


void KNGroupManager::resortGroup(KNGroup *g)
{
  if(!g) g=c_urrentGroup;
  if(!g) return;
  g->resort();
  if(g==c_urrentGroup)
    a_rticleMgr->showHdrs();
}



void KNGroupManager::setCurrentGroup(KNGroup *g)
{
  c_urrentGroup=g;
  a_rticleMgr->setGroup(g);
  bool loaded;
  kdDebug(5003) << "KNGroupManager::setCurrentGroup() : group changed" << endl;

  if (g) {
    loaded=g->loadHdrs();
    if (loaded) {
     a_rticleMgr->showHdrs();
      if(knGlobals.cfgManager->readNewsGeneral()->autoCheckGroups())
        checkGroupForNewHeaders(g);
    }
    else
      KMessageBox::error(knGlobals.topWidget, i18n("Cannot load saved headers"));
  }
}



void KNGroupManager::checkAll(KNNntpAccount *a)
{
  if(!a) return;

  for(KNGroup *g=g_List->first(); g; g=g_List->next()) {
    if(g->account()==a) {
      g->setMaxFetch(knGlobals.cfgManager->readNewsGeneral()->maxToFetch());
      if(g->loadHdrs())
        emitJob( new KNJobData(KNJobData::JTfetchNewHeaders, this, a, g) );
    }
  }
}



void KNGroupManager::processJob(KNJobData *j)
{
  if((j->type()==KNJobData::JTLoadGroups)||(j->type()==KNJobData::JTFetchGroups)||(j->type()==KNJobData::JTCheckNewGroups)) {
    KNGroupListData *d=static_cast<KNGroupListData*>(j->data());

    if (!j->canceled()) {
      if (j->success()) {
        if ((j->type()==KNJobData::JTFetchGroups)||(j->type()==KNJobData::JTCheckNewGroups)) {
          // update the descriptions of the subscribed groups
          for(KNGroup *var=g_List->first(); var; var=g_List->next()) {
            if(var->account()==j->account()) {
              for (KNGroupInfo* inf = d->groups->first(); inf; inf=d->groups->next())
                if (inf->name == var->groupname()) {
                  var->setDescription(inf->description);
                  var->setStatus(inf->status);
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
        if(group->newCount()>0) {
          group->updateListItem();
          group->saveInfo();
        }
      } else
        KMessageBox::error(knGlobals.topWidget, j->errorString());
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
    if (KMessageBox::Yes==KMessageBox::questionYesNo(knGlobals.topWidget,i18n("You don't have any groups for this account.\nDo you want to fetch a current list?"))) {
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

  emitJob( new KNJobData(KNJobData::JTLoadGroups, this, a, d) );
}


// fetch group list from server
void KNGroupManager::slotFetchGroupList(KNNntpAccount *a)
{
  KNGroupListData *d = new KNGroupListData();
  d->path = a->path();  
  getSubscribed(a,d->subscribed);
  d->getDescriptions = a->fetchDescriptions();

  emitJob( new KNJobData(KNJobData::JTFetchGroups, this, a, d) );
}


// check for new groups (created after the given date)
void KNGroupManager::slotCheckForNewGroups(KNNntpAccount *a, QDate date)
{
  KNGroupListData *d = new KNGroupListData();
  d->path = a->path();  
  getSubscribed(a,d->subscribed);
  d->getDescriptions = a->fetchDescriptions();
  d->fetchSince = date;
  
  emitJob( new KNJobData(KNJobData::JTCheckNewGroups, this, a, d) );
}


//--------------------------------

#include "kngroupmanager.moc"

