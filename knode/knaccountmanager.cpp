/***************************************************************************
                          knaccountmanager.cpp  -  description
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

#include <stdlib.h>

#include <qdir.h>

#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kurl.h>
#include <kstddirs.h>

#include "kngroup.h"
#include "kngroupmanager.h"
#include "knnntpaccount.h"
#include "knserverinfo.h"
#include "knsavedarticlemanager.h"
#include "knaccnewssettings.h"
#include "kncollectionviewitem.h"
#include "knglobals.h"
#include "utilities.h"
#include "knaccountmanager.h"


KNAccountManager::KNAccountManager(KNGroupManager *gm, KNListView *v, QObject * parent, const char * name)
  : QObject(parent, name), gManager(gm), set(0), lastId(0), view(v)
{
	accList=new QList<KNNntpAccount>;
	accList->setAutoDelete(true);
	s_mtp=new KNServerInfo();
	s_mtp->setType(KNServerInfo::STsmtp);
	s_mtp->setId(0);
	readConfig();
	loadAccounts();
	
  actProperties = new KAction(i18n("&Properties..."), 0, this, SLOT(slotProperties()),
                              &actionCollection, "account_properties");
  actSubscribe = new KAction(i18n("&Subscribe to Newsgroups..."),"grpdlg", 0, this, SLOT(slotSubscribe()),
                             &actionCollection, "account_subscribe");
  actLoadHdrs = new KAction(i18n("&Get New Articles"), "dlall", 0, this, SLOT(slotLoadHdrs()),
                            &actionCollection, "account_dnlHeaders");
  actDelete = new KAction(i18n("&Delete"), 0, this, SLOT(slotDelete()),
                          &actionCollection, "account_delete");
  actPostNewArticle = new KAction(i18n("&Post new article"), "newmsg", Key_P , this, SLOT(slotPostNewArticle()),
                                  &actionCollection, "article_postNew");
	
	setCurrentAccount(0);
}



KNAccountManager::~KNAccountManager()
{
	delete accList;
	delete s_mtp;
}



void KNAccountManager::readConfig()
{
	KConfig *conf=KGlobal::config();
	conf->setGroup("SERVER");
	s_mtp->setServer((conf->readEntry("Smtp","")).latin1());
	s_mtp->setPort(conf->readNumEntry("sPort", 25));	
}



void KNAccountManager::saveYourself()
{
	for(KNNntpAccount *a=accList->first(); a; a=accList->next())
		a->syncInfo();
}



void KNAccountManager::loadAccounts()
{
	QString dir(KGlobal::dirs()->saveLocation("appdata"));
	if (dir==QString::null) {
		displayInternalFileError();
		return;
	}
	QDir d(dir);
	int id;
	KNNntpAccount *a;
	
	QStringList entries(d.entryList("nntp.*", QDir::Dirs));

  QStringList::Iterator it;
	for(it = entries.begin(); it != entries.end(); it++) {
		KSimpleConfig conf(dir+(*it)+"/info");
		id=conf.readNumEntry("id", -1);
  	if(id!=-1) {
  		a=new KNNntpAccount();
  		a->setId(id);
  		if(lastId<id) lastId=id;
  		a->setName(conf.readEntry("name", "unknown"));
  		a->setServer(conf.readEntry("server", "localhost").local8Bit());
  		a->setPort(conf.readNumEntry("port", 119));
  		a->setUser(conf.readEntry("user").local8Bit());
  		a->setPass(decryptStr(conf.readEntry("pass")).local8Bit());
  		a->setUnsentCount(conf.readNumEntry("unsentCnt", 0));
  		accList->append(a);
  		KNCollectionViewItem* cvit=new KNCollectionViewItem(view);
  		a->setListItem(cvit);
  		cvit->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTnntp));
  		gManager->loadGroups(a);
  	}
  }	
}



KNNntpAccount* KNAccountManager::account(int i)
{
	KNNntpAccount *ret=0;
	if(i>0)
	{
		for(KNNntpAccount *a=accList->first(); a; a=accList->next()) {
			if(a->id()==i) {
				ret=a;
				break;
			}
		}
	}
	return ret;	
}



void KNAccountManager::setCurrentAccount(KNNntpAccount *a)
{
	c_urrentAccount=a;
	if (a) {
		actProperties->setEnabled(true);
		actSubscribe->setEnabled(true);
		actLoadHdrs->setEnabled(true);
		actDelete->setEnabled(true);
		actPostNewArticle->setEnabled(true);
	} else {
		actProperties->setEnabled(false);
		actSubscribe->setEnabled(false);
		actLoadHdrs->setEnabled(false);
		actDelete->setEnabled(false);
		actPostNewArticle->setEnabled(false);		
	}
}



void KNAccountManager::startConfig(KNAccNewsSettings *s)
{
	set=s;
	for(KNNntpAccount *a=accList->first(); a; a=accList->next())
		set->addItem(a);
}



void KNAccountManager::newAccount()
{
	KNNntpAccount *a=new KNNntpAccount();
	a->setId(++lastId);
		
	QString dir(KGlobal::dirs()->saveLocation("appdata",QString("nntp.%1/").arg(a->id())));
	if (dir!=QString::null) {
		accList->append(a);
		applySettings(a);
		set->addItem(a);
		KNCollectionViewItem *it = new KNCollectionViewItem(view);
  	it->setPixmap(0, UserIcon("server"));
  	a->setListItem(it);
	}
	else {
		delete a;
		KMessageBox::error(0, i18n("Cannot create a directory for this account!"));
	}
}



void KNAccountManager::applySettings(KNNntpAccount *a)
{
	a->setName(set->name());
	a->setServer(set->server().local8Bit());
	a->setPort(a->port());
	if(set->logonNeeded()) {
		a->setUser(set->user().local8Bit());
		a->setPass(set->pass().local8Bit());
	}
	else {
		a->setUser("");
		a->setPass("");
	}	
	a->saveInfo();
	a->updateListItem();
}


void KNAccountManager::removeAccount(KNNntpAccount *a)
{
	if(!a) a=c_urrentAccount;
	if(!a) return;

	QList<KNGroup> *lst;
	if(a->hasUnsent()) {
		KMessageBox::information(0, i18n("This account cannot be deleted, since there are some unsent messages for it."));
	}	
	else if(KMessageBox::questionYesNo(0, i18n("Do you really want to delete this account?"))==KMessageBox::Yes) {
		lst=new QList<KNGroup>;
		lst->setAutoDelete(false);
		gManager->getGroupsOfAccount(a, lst);
		for(KNGroup *g=lst->first(); g; g=lst->next()) {
		  if(g->locked()) {
		    KMessageBox::information(0, i18n("At least one group of this account is currently in use.\nThe account cannot be deleted at the moment."));
		    return;
		  }
		}
		
		for(KNGroup *g=lst->first(); g; g=lst->next())
		  gManager->unsubscribeGroup(g);
		
		delete lst;
		set->removeItem(a);
		QString dir(a->path());
		if (dir != QString::null) {
			QString cmd = "rm -rf "+a->path();
			system(cmd.local8Bit().data());
		}
		accList->removeRef(a);
		
		if(c_urrentAccount==a) setCurrentAccount(0);
	}			
}



void KNAccountManager::endConfig()
{
	set=0;
}



void KNAccountManager::slotProperties()
{
  #warning FIXME: stub (open conf dialog and show account properties)
}
  	


void KNAccountManager::slotSubscribe()
{
	if (c_urrentAccount)
		gManager->showGroupDialog(c_urrentAccount);
}

 	

void KNAccountManager::slotLoadHdrs()
{
  if (c_urrentAccount)
    gManager->checkAll(c_urrentAccount);
}



void KNAccountManager::slotDelete()
{
  removeAccount();
}



void KNAccountManager::slotPostNewArticle()
{
  if (c_urrentAccount) {
  	if(gManager->hasCurrentGroup())
	    knGlobals.sArtManager->post(gManager->currentGroup());
  	else
  	  knGlobals.sArtManager->post(c_urrentAccount);	
  }
}

//--------------------------------

#include "knaccountmanager.moc"

