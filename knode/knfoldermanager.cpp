/***************************************************************************
                          knfoldermanager.cpp  -  description
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

#include <qlistview.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <kaction.h>
#include <kglobal.h>
#include <kapp.h>
#include <kurl.h>
#include <kdebug.h>

#include "knode.h"
#include "knglobals.h"
#include "knfolder.h"
#include "kncollectionviewitem.h"
#include "kncleanup.h"
#include "knpurgeprogressdialog.h"
#include "knsavedarticlemanager.h"
#include "utilities.h"
#include "knfoldermanager.h"


KNFolderManager::KNFolderManager(KNSavedArticleManager *a, KNListView *v,  QObject * parent, const char * name)
  : QObject(parent, name), view(v), aManager(a), lastId(3), c_ount(3)
{
  fList=new QList<KNFolder>;
  fList->setAutoDelete(true);
  createStandardFolders();
  c_ount+=loadCustomFolders();
  showListItems();
    
  actCompactFolder = new KAction(i18n("&Compact Folder"), 0, this, SLOT(slotCompactFolder()),
                                 &actionCollection, "folder_compact");
  actEmptyFolder = new KAction(i18n("&Empty Folder"), 0, this, SLOT(slotEmptyFolder()),
                               &actionCollection, "folder_empty");

  setCurrentFolder(0);  
}



KNFolderManager::~KNFolderManager()
{
  delete fList;
}



bool KNFolderManager::timeToCompact()
{
  KConfig *c=KGlobal::config();
  c->setGroup("EXPIRE");
  
  if (!c->readBoolEntry("doCompact", true))
    return false;

  QDate today=QDate::currentDate();
  QDate lastComDate=c->readDateTimeEntry("lastCompact").date();
  int interval=c->readNumEntry("comInterval", 5);
  
  if (lastComDate==today) {
    c->writeEntry("lastCompact", QDateTime::currentDateTime());  // important! otherwise lastComDate will be at its default value (current date) forever
    return false;
  }

  if(lastComDate.daysTo(today) >= interval)
    return true;
  else
    return false;
}



void KNFolderManager::syncFolders()
{
  QString dir(KGlobal::dirs()->saveLocation("appdata","folders/"));
  if (dir==QString::null) {
    displayInternalFileError();
    return;
  }
  KSimpleConfig info(dir+"standard.info");
  int idx=0;
  info.writeEntry("draftsCount", fList->at(0)->count());
  info.writeEntry("outboxCount", fList->at(1)->count());
  info.writeEntry("sentCount", fList->at(2)->count());
  for(KNFolder *f=fList->first(); f; f=fList->next()) {
    f->syncDynamicData();
    if(idx++>2) f->saveInfo();
  }
}



void KNFolderManager::createStandardFolders()
{
  QString dir(KGlobal::dirs()->saveLocation("appdata","folders/"));
  if (dir==QString::null) {
    displayInternalFileError();
    return;
  }
  KSimpleConfig info(dir+"standard.info");
  KNFolder *d, *o, *s;
  d=new KNFolder();
  d->setName(i18n("Drafts"));
  d->setId(1);
  d->setCount(info.readNumEntry("draftsCount", 0));
  fList->append(d);
  o=new KNFolder();
  o->setName(i18n("Outbox"));
  o->setId(2);
  o->setCount(info.readNumEntry("outboxCount", 0));
  fList->append(o);
  s=new KNFolder();
  s->setName(i18n("Sent"));
  s->setId(3);
  s->setCount(info.readNumEntry("sentCount", 0));
  fList->append(s);
  aManager->setStandardFolders(d,o,s);
}



int KNFolderManager::loadCustomFolders()
{
  return 0;
}



void KNFolderManager::showListItems()
{
  for(KNFolder *f=fList->first(); f; f=fList->next())
    if(!f->listItem()) createListItem(f);
}



void KNFolderManager::createListItem(KNFolder *f)
{
  KNCollectionViewItem *it;
  if(f->parent()==0) {
    it=new KNCollectionViewItem(view);
    f->setListItem(it);
  }
  else {
    if(!f->parent()->listItem()) createListItem((KNFolder*)f->parent());
    it=new KNCollectionViewItem(f->parent()->listItem());
    f->setListItem(it);
  }
  f->setListItem(it);
  it->setPixmap(0, KNLVItemBase::icon(KNLVItemBase::PTfolder));
  f->updateListItem();
}



void KNFolderManager::setCurrentFolder(KNFolder *f)
{
  c_urrentFolder=f;
  aManager->setFolder(f);
  
  kdDebug(5003) << "KNFolderManager::setCurrentFolder() : folder changed" << endl;
  
  if(f) {
    if(f->loadHdrs())
      aManager->showHdrs();
    else
       KMessageBox::error(knGlobals.topWidget, i18n("Cannot load index-file!"));
    actCompactFolder->setEnabled(true);
    actEmptyFolder->setEnabled(true);   
  } else {
    actCompactFolder->setEnabled(false);
    actEmptyFolder->setEnabled(false);
  } 
}



KNFolder* KNFolderManager::standardFolder(stFolder stf)
{
  return fList->at((int)stf);
}



KNFolder* KNFolderManager::folder(int i)
{
  KNFolder *ret=0;
  for(ret=fList->first(); ret; ret=fList->next())
    if(ret->id()==i) break;
  return ret;   
}



void KNFolderManager::newFolder(KNFolder *f)
{
  if(!f) f=c_urrentFolder;
  if(!f) return;
}



void KNFolderManager::deleteFolder(KNFolder *f)
{
  if(!f) f=c_urrentFolder;
  if(!f) return;
  if(f->id()<=3)
    KMessageBox::sorry(knGlobals.topWidget, i18n("This folder can not be deleted!"));
  else {}
}



void KNFolderManager::removeFolder(KNFolder *f)
{
  if(!f) f=c_urrentFolder;
  if(!f) return;
  if(f->id()<=3)
    KMessageBox::sorry(knGlobals.topWidget, i18n("This folder can not be removed!"));
  else {}
}



void KNFolderManager::emptyFolder(KNFolder *f)
{
  if(!f) f=c_urrentFolder;
  if(!f) return;
  if(KMessageBox::questionYesNo(knGlobals.topWidget,i18n("Really empty this folder?"))==KMessageBox::Yes) {
    f->deleteAll();
    if(f==c_urrentFolder)
      aManager->setCurrentArticle(0);
  }   
}



void KNFolderManager::compactFolder(KNFolder *f)
{
  KNCleanUp cup;
  if(!f) f=c_urrentFolder;
  if(!f) return;
  cup.folder(f);
}



void KNFolderManager::compactAll(KNPurgeProgressDialog *dlg)
{
  KNCleanUp cup;
  
  if (dlg) {
    knGlobals.top->blockUI(true);
    dlg->init(i18n("Compacting folders ..."), fList->count());
  }

  for(KNFolder *var=fList->first(); var; var=fList->next()) {
    if(dlg) {
      dlg->setInfo(var->name());
      kapp->processEvents();
    }
    cup.folder(var);
    kdDebug(5003) << var->name() << " => " << cup.deleted() << " deleted , " << cup.left() << " left" << endl;
    if(dlg) dlg->progress();
  }
  if (dlg) {
    knGlobals.top->blockUI(false);
    kapp->processEvents();
  }
  
  KConfig *c=KGlobal::config();
  c->setGroup("EXPIRE");
  c->writeEntry("lastCompact", QDateTime::currentDateTime());
}

//--------------------------------

#include "knfoldermanager.moc"
