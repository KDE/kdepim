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
#include "knode.h"
#include "utilities.h"
#include "knaccountmanager.h"


KNAccountManager::KNAccountManager(KNGroupManager *gm, KNListView *v, QObject * parent, const char * name)
  : QObject(parent, name), gManager(gm), view(v)
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
	conf->setGroup("MAILSERVER");
	s_mtp->readConf(conf);
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
  KNNntpAccount *a;
	
  QStringList entries(d.entryList("nntp.*", QDir::Dirs));

  QStringList::Iterator it;	
  for(it = entries.begin(); it != entries.end(); it++) {
    a=new KNNntpAccount();
    if (a->readInfo(dir+(*it)+"/info")) {
      accList->append(a);
      KNCollectionViewItem* cvit=new KNCollectionViewItem(view);
      a->setListItem(cvit);
      cvit->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTnntp));
      gManager->loadGroups(a);
    } else {
      delete a;
      qDebug("Unable to load account %s!",(*it).local8Bit().data());
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



// a is new account allocated and configured by the caller
void KNAccountManager::newAccount(KNNntpAccount *a)
{
	// find a unused id for the new account...
	QString dir(KGlobal::dirs()->saveLocation("appdata"));
  if (dir==QString::null) {
    displayInternalFileError();
    return;
  }
  QDir d(dir);
  QStringList entries(d.entryList("nntp.*", QDir::Dirs));

  int id = 1;
  while (entries.findIndex(QString("nntp.%1").arg(id))!=-1)
    ++id;
	
	a->setId(id);
		
	dir = KGlobal::dirs()->saveLocation("appdata",QString("nntp.%1/").arg(a->id()));
	if (dir!=QString::null) {
		accList->append(a);
		applySettings(a);
		KNCollectionViewItem *it = new KNCollectionViewItem(view);
  	it->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTnntp));
  	a->setListItem(it);
  	emit(accountAdded(a));
	}
	else {
		delete a;
		KMessageBox::error(0, i18n("Cannot create a directory for this account!"));
	}
}



// commit changes on a the caller made
void KNAccountManager::applySettings(KNNntpAccount *a)
{
	a->saveInfo();
	a->updateListItem();
	emit(accountModified(a));
}



// a==0: remove current account
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
		
  	QDir dir(a->path());
  	if (dir.exists()) {
      const QFileInfoList *list = dir.entryInfoList();  // get list of matching files and delete all
      if (list) {
        QFileInfoListIterator it( *list );
        while (it.current()) {
          dir.remove(it.current()->fileName());
          ++it;
        }
      }
      dir.cdUp();                                       // directory should now be empty, deleting it
      dir.rmdir(QString("nntp.%1/").arg(a->id()));
    }
		
  	if(c_urrentAccount==a) setCurrentAccount(0);
		
		emit(accountRemoved(a));
		accList->removeRef(a);		  // finally delete a
	}			
}
	


void KNAccountManager::slotProperties()
{
  if (c_urrentAccount) {
    KNAccNewsConfDialog *confDlg = new KNAccNewsConfDialog(c_urrentAccount,knGlobals.top);

    if (confDlg->exec())
      applySettings(c_urrentAccount);

    delete confDlg;
  }
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

