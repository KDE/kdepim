/***************************************************************************
                          kngroupmanager.cpp  -  description
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

#include "knpurgeprogressdialog.h"
#include "knode.h"
#include "knfetcharticlemanager.h"
#include "knjobdata.h"
#include "kngroupdialog.h"
#include "kngroup.h"
#include "kncollectionviewitem.h"
#include "knnntpaccount.h"
#include "kncleanup.h"
#include "knuserentry.h"
#include "knnetaccess.h"
#include "knglobals.h"
#include "resource.h"
#include "utilities.h"
#include "knarticlewindow.h"
#include "kngroupmanager.h"


//=================================================================================

// helper classes for the group selection dialog (getting the server's grouplist,
// getting recently created groups)


KNGroupInfo::KNGroupInfo()
{
}


KNGroupInfo::KNGroupInfo(const char *n_ame, const char *d_escription, bool n_ewGroup, bool s_ubscribed)
  : name(n_ame), description(d_escription), newGroup(n_ewGroup), subscribed(s_ubscribed)
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



bool KNGroupListData::readIn()
{
  KNFile f(path+"groups");
  QCString line,name;
  int sepPos;

  if(f.open(IO_ReadOnly)) {
    while(!f.atEnd()) {
      line = f.readLine();
      sepPos = line.find(' ');

      if (sepPos==-1) {        // no description

        if (subscribed.contains(line)) {
          subscribed.remove(line);    // group names are unique, we wont find it again anyway...
          groups->append(new KNGroupInfo(line,"",false,true));
        } else {
          groups->append(new KNGroupInfo(line,"",false,false));
        }

      } else {
        name = line.left(sepPos);

        if (subscribed.contains(name)) {
          subscribed.remove(name);    // group names are unique, we wont find it again anyway...
          groups->append(new KNGroupInfo(name,line.right(line.length()-sepPos-1),false,true));
        } else {
          groups->append(new KNGroupInfo(name,line.right(line.length()-sepPos-1),false,false));
        }

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
      temp = i->name + " " + i->description + "\n";
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
    groups->append(new KNGroupInfo(i->name,i->description,true,subscribed));
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


KNGroupManager::KNGroupManager(KNFetchArticleManager *a, QObject * parent, const char * name)
  : QObject(parent,name), aManager(a)
{
  gList=new QList<KNGroup>;
  gList->setAutoDelete(true);
    
  readConfig(); 
  
  actProperties = new KAction(i18n("&Properties..."), 0, this, SLOT(slotProperties()),
                              &actionCollection, "group_properties");
  actLoadHdrs = new KAction(i18n("&Get New Articles"), "mail_get" , 0, this, SLOT(slotLoadHdrs()),
                            &actionCollection, "group_dnlHeaders");
  actExpire = new KAction(i18n("E&xpire Now"), "wizard", 0, this, SLOT(slotExpire()),
                          &actionCollection, "group_expire");
  actResort = new KAction(i18n("Res&ort"), 0, this, SLOT(slotResort()),
                          &actionCollection, "group_resort");
  actUnsubscribe = new KAction(i18n("&Unsubscribe"), 0, this, SLOT(slotUnsubscribe()),
                               &actionCollection, "group_unsubscribe");     
  
  setCurrentGroup(0);
}



KNGroupManager::~KNGroupManager()
{
  delete gList;
}



void KNGroupManager::readConfig()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("READNEWS");
  a_utoCheck=conf->readBoolEntry("autoCheck",true);
  defaultMaxFetch=conf->readNumEntry("maxFetch", 1000);
}



bool KNGroupManager::timeToExpire()
{
  KConfig *c=KGlobal::config();
  c->setGroup("EXPIRE");
  
  if (!c->readBoolEntry("doExpire", true))
    return false;

  QDate today=QDate::currentDate();
  QDate lastExpDate=c->readDateTimeEntry("lastExpire").date();
  int interval=c->readNumEntry("expInterval", 5);
  
  if (lastExpDate==today) {
    c->writeEntry("lastExpire", QDateTime::currentDateTime());  // important! otherwise lastExpDate will be at its default value (current date) forever
    return false;
  }

  if(lastExpDate.daysTo(today) >= interval)
    return true;
  else
    return false;
}



void KNGroupManager::syncGroups()
{
  for(KNGroup *var=gList->first(); var; var=gList->next()) {
    var->syncDynamicData();
    var->saveInfo();
  }
}



void KNGroupManager::loadGroups(KNNntpAccount *a)
{   
  KNGroup *group;
  
  QString dir(a->path());
  if (dir == QString::null)
    return;
  QDir d(dir);  
    
  QStringList entries(d.entryList("*.grpinfo"));
  for(QStringList::Iterator it=entries.begin(); it != entries.end(); it++) {
    group=new KNGroup(a);
    if (group->readInfo(dir+(*it))) {
      gList->append(group);
      KNCollectionViewItem *cvit=new KNCollectionViewItem(a->listItem());
      cvit->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTgroup));
      group->setListItem(cvit);
      group->updateListItem();
    } else {
      delete group;
      kdError(5003) << "Unable to load " << (*it) << "!" << endl;
    } 
  }
}



void KNGroupManager::getSubscribed(KNNntpAccount *a, QStrList* l)
{
  l->clear();
  for(KNGroup *var=gList->first(); var; var=gList->next()) {
    if(var->account()==a) l->append(var->groupname());
  }
}



void KNGroupManager::getGroupsOfAccount(KNNntpAccount *a, QList<KNGroup> *l)
{
  l->clear();
  for(KNGroup *var=gList->first(); var; var=gList->next()) {
    if(var->account()==a) l->append(var);
  }
}



KNGroup* KNGroupManager::group(const QCString &gName, const KNServerInfo *s)
{
  for(KNGroup *var=gList->first(); var; var=gList->next())
    if(var->account()==s && var->groupname()==gName) return var;
  
  return 0;
}



void KNGroupManager::expireAll(KNPurgeProgressDialog *dlg)
{
  KNCleanUp cup;
  
  if (dlg) {
    knGlobals.top->blockUI(true);
    dlg->init(i18n("Deleting expired articles ..."), gList->count());
  }

  for(KNGroup *var=gList->first(); var; var=gList->next()) {
    if(dlg) {
      dlg->setInfo(var->groupname());
      kapp->processEvents();
    }
    KNArticleWindow::closeAllWindowsForCollection(var);
    cup.group(var);
    kdDebug() << var->groupname() << " => " << cup.deleted() << " expired , " << cup.left() << " left" << endl;
    if(dlg) dlg->progress();
  }
  if (dlg) {
    knGlobals.top->blockUI(false);
    kapp->processEvents();
  }
  
  KConfig *c=KGlobal::config();
  c->setGroup("EXPIRE");
  c->writeEntry("lastExpire", QDateTime::currentDateTime());
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
    
    QStrList lst;
    gDialog->toUnsubscribe(&lst);
    if (lst.count()>0) {
      if (KMessageBox::Yes == KMessageBox::questionYesNoList((parent!=0)? parent:knGlobals.topWidget,i18n("Do you really want to unsubscribe\nfrom these groups?"),
                                                             QStringList::fromStrList(lst))) {
        for(char *var=lst.first(); var; var=lst.next()) {
          if((g=group(var, a)))
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
  grp->saveInfo();
  gList->append(grp);
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

  if(g->locked()) {
    KMessageBox::sorry(knGlobals.topWidget, QString(i18n("The group \"%1\" is being updated currently.\nIt is not possible to unsubscribe it at the moment.")).arg(g->groupname()));
    return;
  }

  KNArticleWindow::closeAllWindowsForCollection(g);

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
    
    gList->removeRef(g);
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
  if(g->locked()) {
    kdDebug(5003) << "KNGroupManager::setCurrentGroup() : group locked - returning" << endl;
    return;
  } 
  g->setMaxFetch(defaultMaxFetch);
  KNJobData *job=new KNJobData(KNJobData::JTfetchNewHeaders, g->account(), g);
  knGlobals.netAccess->addJob(job);
}



void KNGroupManager::expireGroupNow(KNGroup *g)
{
  if(!g) g=c_urrentGroup;
  if(!g) return;

  KNArticleWindow::closeAllWindowsForCollection(g);

  KNCleanUp cup;
  cup.group(g, true);
  kdDebug(5003) << "KNExpire: " << g->groupname() << " => " << cup.deleted() << " expired , " << cup.left() << " left" << endl;
  
  if(cup.deleted()>0) {
    g->updateListItem();
    if(g==c_urrentGroup) {
      if(g->loadHdrs()) aManager->showHdrs();
      else aManager->setGroup(0);
    }
  }     
}


void KNGroupManager::resortGroup(KNGroup *g)
{
  if(!g) g=c_urrentGroup;
  if(!g) return;
  g->resort();
  if(g==c_urrentGroup) aManager->showHdrs();
}


  
void KNGroupManager::setCurrentGroup(KNGroup *g)
{
  c_urrentGroup=g;
  aManager->setGroup(g);
  bool loaded;
  kdDebug(5003) << "KNGroupManager::setCurrentGroup() : group changed" << endl;
  
  if (g) {
    loaded=g->loadHdrs();
    if (loaded) {
      aManager->showHdrs();
      if(a_utoCheck) checkGroupForNewHeaders(g);
    } else
      KMessageBox::error(knGlobals.topWidget, i18n("Cannot load saved headers"));
    
    actProperties->setEnabled(true);
    actLoadHdrs->setEnabled(true);
    actExpire->setEnabled(true);
    actResort->setEnabled(true);
    actUnsubscribe->setEnabled(true);   
  } else {
    actProperties->setEnabled(false);
    actLoadHdrs->setEnabled(false);
    actExpire->setEnabled(false);
    actResort->setEnabled(false);
    actUnsubscribe->setEnabled(false);    
  }
}



void KNGroupManager::checkAll(KNNntpAccount *a)
{
  KNJobData *j;
  if(!a) return;
  
  for(KNGroup *g=gList->first(); g; g=gList->next()) {
    if(g->account()==a) {
      g->setMaxFetch(defaultMaxFetch);
      if(g->loadHdrs()) {
        j=new KNJobData(KNJobData::JTfetchNewHeaders, a, g);
        knGlobals.netAccess->addJob(j);
      }
    }
  } 
}



void KNGroupManager::jobDone(KNJobData *j)
{
  if((j->type()==KNJobData::JTLoadGroups)||(j->type()==KNJobData::JTFetchGroups)||(j->type()==KNJobData::JTCheckNewGroups)) {
    KNGroupListData *d=static_cast<KNGroupListData*>(j->data());  

    if (!j->canceled()) {
      if (j->success()) {
        if ((j->type()==KNJobData::JTFetchGroups)||(j->type()==KNJobData::JTCheckNewGroups)) {
          // update the descriptions of the subscribed groups
          for(KNGroup *var=gList->first(); var; var=gList->next()) {
            if(var->account()==j->account()) {
              for (KNGroupInfo* inf = d->groups->first(); inf; inf=d->groups->next())
                if (inf->name == var->groupname()) {
                  var->setDescription(inf->description.copy());
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
    if(group==c_urrentGroup) aManager->showHdrs(false);         
  }
  
  delete j;
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
      
  getSubscribed(a,&(d->subscribed));
  d->getDescriptions = a->fetchDescriptions();

  KNJobData *job=new KNJobData(KNJobData::JTLoadGroups, a, d);
  knGlobals.netAccess->addJob(job);
}


// fetch group list from server
void KNGroupManager::slotFetchGroupList(KNNntpAccount *a)
{
  KNGroupListData *d = new KNGroupListData();
  d->path = a->path();  
  getSubscribed(a,&(d->subscribed));
  d->getDescriptions = a->fetchDescriptions();

  KNJobData *job=new KNJobData(KNJobData::JTFetchGroups, a, d);
  knGlobals.netAccess->addJob(job);
}


// check for new groups (created after the given date)
void KNGroupManager::slotCheckForNewGroups(KNNntpAccount *a, QDate date)
{
  KNGroupListData *d = new KNGroupListData();
  d->path = a->path();  
  getSubscribed(a,&(d->subscribed));
  d->getDescriptions = a->fetchDescriptions();
  d->fetchSince = date;
  
  KNJobData *job=new KNJobData(KNJobData::JTCheckNewGroups, a, d);
  knGlobals.netAccess->addJob(job);
}


void KNGroupManager::slotUnsubscribe()
{
  if (!c_urrentGroup)
    return;
  if(KMessageBox::Yes == KMessageBox::questionYesNo(knGlobals.topWidget, i18n("Do you really want to unsubscribe from %1?").arg(c_urrentGroup->groupname())))
    unsubscribeGroup();
}


//--------------------------------

#include "kngroupmanager.moc"

