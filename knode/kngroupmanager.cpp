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
#include "kngroupmanager.h"


KNGroupManager::KNGroupManager(KNFetchArticleManager *a, QObject * parent, const char * name)
  : QObject(parent,name), aManager(a), gDialog(0)
{
	gList=new QList<KNGroup>;
	gList->setAutoDelete(true);
		
	readConfig();	
	
	actProperties = new KAction(i18n("&Properties..."), 0, this, SLOT(slotProperties()),
                              &actionCollection, "group_properties");
  actLoadHdrs = new KAction(i18n("&Get New Articles"), 0, this, SLOT(slotLoadHdrs()),
                            &actionCollection, "group_dnlHeaders");
  actExpire = new KAction(i18n("E&xpire Now"), 0, this, SLOT(slotExpire()),
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
	delete gDialog;
}



void KNGroupManager::readConfig()
{
	KConfig *conf=KGlobal::config();
	conf->setGroup("READNEWS");
	a_utoCheck=conf->readBoolEntry("autoCheck",true);
	defaultMaxFetch=conf->readNumEntry("maxFetch", 500);		
}



bool KNGroupManager::timeToExpire()
{
	bool doExpire;
	QDate today=QDate::currentDate();
	QDate lastExpDate;
	int y, m, d, interval;

	KConfig *c=KGlobal::config();
	c->setGroup("EXPIRE");
	doExpire=c->readBoolEntry("doExpire", true);
	
	if(!doExpire) return false;
	
	y=c->readNumEntry("lastExpY", 0);
	m=c->readNumEntry("lastExpM", 0);
	d=c->readNumEntry("lastExpD", 0);
	interval=c->readNumEntry("expInterval", 5);
  lastExpDate.setYMD(y,m,d);
	if(!lastExpDate.isValid()) {
		c->writeEntry("lastExpY", today.year());
		c->writeEntry("lastExpM", today.month());
		c->writeEntry("lastExpD", today.day());
		return false;
	}
	if(lastExpDate==today) return false;
	if(lastExpDate.daysTo(today) >= interval) return true;
	else return false;
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
	KNUserEntry *usr=0;
	QString fName;
	
	QString dir(a->path());
	if (dir == QString::null)
		return;
	QDir d(dir);	
	 	
	QStringList entries(d.entryList("*.grpinfo"));
	for(QStringList::Iterator it=entries.begin(); it != entries.end(); it++) {
	  fName=dir+(*it);
		KSimpleConfig info(fName);
		/*if(tmp.isEmpty()) {
			tmp=info.deleteEntry("name", false);
			info.writeEntry("groupname", tmp);
			qDebug("Group info-file converted");
			info.sync();
		}*/
		group=new KNGroup(a);
		group->setGroupname(info.readEntry("groupname").utf8());
		group->setName(info.readEntry("name"));
		group->setCount(info.readNumEntry("count",0));
		group->setReadCount(info.readNumEntry("read",0));
		group->setLastNr(info.readNumEntry("lastMsg",0));
		if(!usr) usr=new KNUserEntry();
		usr->load(&info);
		if(!usr->isEmpty()) {
			qDebug("alternative user");
			group->setUser(usr);
			usr=0;
		}
		gList->append(group);
	  KNCollectionViewItem *cvit=new KNCollectionViewItem(a->listItem());
	  cvit->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTgroup));
	  group->setListItem(cvit);
	  group->updateListItem();
	}
	
	delete usr;
}



void KNGroupManager::getSubscribed(KNNntpAccount *a, QStrList *l)
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
	QDate today=QDate::currentDate();
	KConfig *c=KGlobal::config();
	
	if(dlg) dlg->init(i18n("Deleting expired articles ..."), gList->count());
	
	for(KNGroup *var=gList->first(); var; var=gList->next()) {
		if(dlg) {
			dlg->setInfo(var->name());
			kapp->processEvents();
		}
		cup.group(var);
		qDebug("%s => %d expired , %d left", var->name().latin1(), cup.deleted(), cup.left());
		if(dlg) dlg->progress();
	}
	if(dlg) kapp->processEvents();
	
	c->setGroup("EXPIRE");
	c->writeEntry("lastExpY", today.year());
	c->writeEntry("lastExpM", today.month());
	c->writeEntry("lastExpD", today.day());
}



void KNGroupManager::showGroupDialog(KNNntpAccount *a)
{
	KNGroup *g=0;
	QString dir(a->path());
	
	if(dir==QString::null)
		return;
		
	if(!QFileInfo(dir+"groups").exists()) {
		if (KMessageBox::Yes==KMessageBox::questionYesNo(0,i18n("You don't have any groups for this account.\nFetch now?")))
	 	 	slotDialogNewList(a);
	 	else return;
	}
		
	gDialog=new KNGroupDialog(knGlobals.top, a);
	connect(gDialog, SIGNAL(newList(KNNntpAccount*)), this, SLOT(slotDialogNewList(KNNntpAccount*)));
	
	if(gDialog->exec()) {
	  QStrList lst;
	  gDialog->toUnsubscribe(&lst);
	  for(char *var=lst.first(); var; var=lst.next()) {
	    if((g=group(var, a)))
	      unsubscribeGroup(g);
	  }
	
	  gDialog->toSubscribe(&lst);
	  for(char *var=lst.first(); var; var=lst.next()) {
      subscribeGroup(var, a);
    }
  }	
		
	delete gDialog;
	gDialog=0;	
}



void KNGroupManager::subscribeGroup(const QCString &gName, KNNntpAccount *a)
{
	KNGroup *grp;
	KNCollectionViewItem *it;
		
	grp=new KNGroup(a);
	grp->setGroupname(gName);
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
		KMessageBox::error(0,
		  QString(i18n("The group \"%1\" is being updated currently.\nIt is not possible to unsubscrib it at the moment.")).arg(g->name()));	
	  return;
	}
	
	acc=g->account();
	
	QDir dir(acc->path(),g->name()+"*");
	if (dir.exists()) {
    const QFileInfoList *list = dir.entryInfoList();  // get list of matching files and delete all
    if (list) {
      QFileInfoListIterator it( *list );
      while (it.current()) {
        dir.remove(it.current()->fileName());
        ++it;
      }
    }
		qDebug("Files deleted!\n");
		
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
		qDebug("KNGroupManager::setCurrentGroup() : group locked - returning");
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
	KNCleanUp cup;
	cup.group(g, true);
	qDebug("KNExpire: %s => %d expired , %d left", g->name().latin1(), cup.deleted(), cup.left());
	
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
	qDebug("KNGroupManager::setCurrentGroup() : group changed");
	
	if (g) {
		knGlobals.top->setCursorBusy(true);
		loaded=g->loadHdrs();
		knGlobals.top->setCursorBusy(false);	
		if (loaded) {
			aManager->showHdrs();
			if(a_utoCheck) checkGroupForNewHeaders(g);
		}	else
		  KMessageBox::error(0, i18n("Cannot load saved headers"));
		
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
	KNGroup *group=0;
	QStrList *groups=0;
	
	if(j->canceled()){
		delete j;
		return;
	}
	
	if(j->success()) {
		if(j->type()==KNJobData::JTlistGroups) {			
			groups=(QStrList*)j->data();
			QString dir(((KNNntpAccount*)j->account())->path());
			if (dir != QString::null) {
				QFile f(dir+"groups");		
			  if(f.open(IO_WriteOnly)) {
		     	for (char *str=groups->first(); str; str=groups->next()) {
		     	  f.writeBlock(str,strlen(str));
		     	  f.putch('\n');
		     	}					
					f.close();
				  if(gDialog) gDialog->newList();				
				}
				else displayInternalFileError();
			}			
		}
		else if(j->type()==KNJobData::JTfetchNewHeaders) {
			group=(KNGroup*)j->data();
			if(group->newCount()>0) {
				group->updateListItem();
				group->saveInfo();
				if(group==c_urrentGroup) aManager->showHdrs(false);
			}
		}
	}
	else {
		if(group) {
			if(group==c_urrentGroup) aManager->showHdrs();
		}
		KMessageBox::error(0, j->errorString());
	}
	delete j;	
}



void KNGroupManager::slotDialogNewList(KNNntpAccount *a)
{
	QStrList *groups=new QStrList();
	groups->setAutoDelete(true);
	KNJobData *job=new KNJobData(KNJobData::JTlistGroups, a, groups);
	knGlobals.netAccess->addJob(job);
}



void KNGroupManager::slotUnsubscribe()
{
  if(KMessageBox::Yes == KMessageBox::questionYesNo(0, i18n("Do you really want to unsubscribe this group?")))
	  unsubscribeGroup();
}


//--------------------------------

#include "kngroupmanager.moc"

