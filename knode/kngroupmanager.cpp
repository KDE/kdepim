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

#include "kngroupmanager.h"
#include "kngrouplistwidget.h"
#include "resource.h"
#include "utilities.h"
#include "kncollectionviewitem.h"
#include "knglobals.h"

#include "kncleanup.h"
#include "knuserentry.h"



KNGroupManager::KNGroupManager(KNFetchArticleManager *a) : QObject(0,0)
{
	aManager=a;
	g_dial=0;	

	gList=new QList<KNGroup>;
	gList->setAutoDelete(true);
		
	readConfig();	
	setCurrentGroup(0);
}



KNGroupManager::~KNGroupManager()
{
	delete gList;
	delete g_dial;
}



void KNGroupManager::readConfig()
{
	KConfig *conf=CONF();
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

	KConfig *c=CONF();
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
	KConfig *c=CONF();
	
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
	QString dir(a->path());
	if (dir == QString::null)
		return;
		
	//check if grouplist file exists
	QString fName(dir+"groups");
		
	if(!QFile(fName).exists()) {
		if (KMessageBox::Yes==KMessageBox::questionYesNo(0,i18n("You don´t have any groups for this account.\n Fetch now?")))
	 	 	slotDialogNewList(a);
	 	else return;
	}
	
	g_dial=new KNGroupDialog(a, xTop);
	connect(g_dial, SIGNAL(getNewList(KNNntpAccount*)),
		this, SLOT(slotDialogNewList(KNNntpAccount*)));
	connect(g_dial, SIGNAL(dialogDone(bool)), this, SLOT(slotDialogDone(bool)));
	g_dial->show();
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
		KMessageBox::error(0, i18n("This group is being updated currently.\nIt is not possible to unsubscrib it at the moment."));	
	  return;
	}
	
	acc=g->account();
	QString dir(acc->path());
	if (dir == QString::null)
		return;
	QString cmd(QString("rm -f %1%2.*").arg(dir).arg(g->name()));	
	if(gList->removeRef(g)) {
		system(cmd.local8Bit().data());
		qDebug("Files deleted!\n");

		if(c_urrentGroup==g) setCurrentGroup(0);
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
	xNet->addJob(job);
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
	
	if(g) {
		xTop->setCursorBusy(true);
		loaded=g->loadHdrs();
		xTop->setCursorBusy(false);	
		if(loaded) {
			aManager->showHdrs();
			if(a_utoCheck) checkGroupForNewHeaders(g);
		}
		else KMessageBox::error(0, i18n("Cannot load saved headers"));
		/*if(a_utoCheck) {
			checkGroupForNewHeaders(g);
		}	
		else {
			xTop->setCursorBusy(true);
			loaded=g->loadHdrs();
			xTop->setCursorBusy(false);	
			if(loaded) {
				aManager->showHdrs();
			}
			else MBox(err, i18n("Cannot load saved headers"));
		}*/
	}
	if(!c_urrentGroup) xTop->groupDisplayed(false);
	xTop->groupSelected((c_urrentGroup!=0));
}



void KNGroupManager::checkAll(KNNntpAccount *a)
{
	KNJobData *j;
	if(!a) return;
	
	for(KNGroup *g=gList->first(); g; g=gList->next()) {
		if(g->account()==a) {
  		g->setMaxFetch(defaultMaxFetch);
  		j=new KNJobData(KNJobData::JTfetchNewHeaders, a, g);
  		xNet->addJob(j);
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
				  if(g_dial) g_dial->glw->newList();				
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
	xNet->addJob(job);
}



void KNGroupManager::slotDialogDone(bool accepted)
{
	if (accepted) {
		QStrList *sub=g_dial->sub(), 	*unsub=g_dial->unsub();
		for(char *var=sub->first(); var; var=sub->next())
			subscribeGroup(var, g_dial->glw->account());
		for(char *var=unsub->first(); var; var=unsub->next())		
			for(KNGroup *g=gList->first(); g; g=gList->next())
				if(g->name() ==var) unsubscribeGroup(g);
	}	
	delete g_dial;
	g_dial=0;	
}




//--------------------------------

#include "kngroupmanager.moc"

