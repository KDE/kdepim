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

#include <qdir.h>

#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>

#include "knaccountmanager.h"
#include "knaccnewssettings.h"
#include "kncollectionviewitem.h"
#include "knglobals.h"
#include "utilities.h"

KNAccountManager::KNAccountManager(KNGroupManager *gm, KNListView *v)
{
	gManager=gm;
	view=v;
	accList=new QList<KNNntpAccount>;
	accList->setAutoDelete(true);
	lastId=0;
	set=0;
	setCurrentAccount(0);
	s_mtp=new KNServerInfo();
	s_mtp->setType(KNServerInfo::STsmtp);
	s_mtp->setId(0);
	readConfig();
	loadAccounts();
}



KNAccountManager::~KNAccountManager()
{
	delete accList;
	delete s_mtp;
}



void KNAccountManager::readConfig()
{
	KConfig *conf=CONF();
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
	xTop->accountSelected((a!=0));
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
  	it->setPixmap(0, UserIcon("server.xpm"));
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
	if(a->hasUnsent()) {
		KMessageBox::information(0, i18n("This account cannot be deleted, since there are some unsent messages for it."));
	}	
	else if(KMessageBox::questionYesNo(0, i18n("Do you really want to delete this account?"))==KMessageBox::Yes) {
		gManager->unsubscribeAccount(a);
		set->removeItem(a);
		QString dir(a->path());
		if (dir != QString::null) {
			QString cmd = "rm -rf "+a->path();
			system(cmd.local8Bit().data());
		}
		accList->removeRef(a);
	}			
}



void KNAccountManager::endConfig()
{
	set=0;
}
