/*
    knfoldermanager.cpp

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

#include "knglobals.h"
#include "knconfigmanager.h"
#include "knfolder.h"
#include "kncollectionviewitem.h"
#include "utilities.h"
#include "knfoldermanager.h"
#include "knarticlemanager.h"
#include "kncleanup.h"


KNFolderManager::KNFolderManager(KNListView *v, KNArticleManager *a) : v_iew(v), a_rtManager(a)
{
  f_List.setAutoDelete(true);

  //standard folders
  QString dir(KGlobal::dirs()->saveLocation("appdata","folders/"));
  if (dir==QString::null) {
    displayInternalFileError();
    return;
  }

  KSimpleConfig info(dir+".standard.info");
  KNFolder *f;

  f=new KNFolder(1, i18n("Drafts"), "drafts");
  f->setCount(info.readNumEntry("draftsCount", 0));
  f_List.append(f);

  f=new KNFolder(2, i18n("Outbox"), "outbox");
  f->setCount(info.readNumEntry("outboxCount", 0));
  f_List.append(f);

  f=new KNFolder(3, i18n("Sent"), "sent");
  f->setCount(info.readNumEntry("sentCount", 0));
  f_List.append(f);

  //custom folders
  loadCustomFolders();

  showListItems();
  setCurrentFolder(0);
}


KNFolderManager::~KNFolderManager()
{
  syncFolders();
}


void KNFolderManager::setCurrentFolder(KNFolder *f)
{
  c_urrentFolder=f;
  a_rtManager->setFolder(f);

  kdDebug(5003) << "KNFolderManager::setCurrentFolder() : folder changed" << endl;

  if(f) {
    if(f->loadHdrs())
      a_rtManager->showHdrs();
    else
      KMessageBox::error(knGlobals.topWidget, i18n("Cannot load index-file!"));
  }
}


KNFolder* KNFolderManager::folder(int i)
{
  KNFolder *ret=0;
  for(ret=f_List.first(); ret; ret=f_List.next())
    if(ret->id()==i) break;
  return ret;
}


void KNFolderManager::newFolder(KNFolder *)
{
}


void KNFolderManager::deleteFolder(KNFolder *)
{
}


void KNFolderManager::removeFolder(KNFolder *)
{
}


void KNFolderManager::showProperties(KNFolder *)
{
}


int KNFolderManager::unsentForAccount(int accId)
{
  int cnt=0;

  for(KNFolder *f=f_List.first(); f; f=f_List.next()) {
    for(int idx=0; idx < f->length(); idx++) {
      KNLocalArticle *a=f->at(idx);
      if(a->serverId()==accId && a->doPost() && !a->posted())
        cnt++;
    }
  }

  return cnt;
}


void KNFolderManager::compactFolder(KNFolder *f)
{
  if(!f)
    return;

  KNCleanUp cup(knGlobals.cfgManager->cleanup());
  cup.compactFolder(f);
}


void KNFolderManager::compactAll(KNCleanUp *cup)
{
  for(KNFolder *f=f_List.first(); f; f=f_List.next()) {
    if(f->lockedArticles()==0)
      cup->appendCollection(f);
  }
}


void KNFolderManager::syncFolders()
{
  QString dir(KGlobal::dirs()->saveLocation("appdata","folders/"));
  if (dir==QString::null) {
    displayInternalFileError();
    return;
  }

  //save standard info
  KSimpleConfig info(dir+".standard.info");
  info.writeEntry("draftsCount", drafts()->count());
  info.writeEntry("outboxCount", outbox()->count());
  info.writeEntry("sentCount", sent()->count());

  //sync
  int idx=0;
  for(KNFolder *f=f_List.first(); f; f=f_List.next()) {
    f->syncIndex();
    if(idx++>2) //do not save info for standard folders
      f->saveInfo();
  }
}


int KNFolderManager::loadCustomFolders()
{
  return 0;
}


void KNFolderManager::showListItems()
{
  for(KNFolder *f=f_List.first(); f; f=f_List.next())
    if(!f->listItem()) createListItem(f);
}


void KNFolderManager::createListItem(KNFolder *f)
{
  KNCollectionViewItem *it;
  if(f->parent()==0) {
    it=new KNCollectionViewItem(v_iew);
    f->setListItem(it);
  }
  else {
    if(!f->parent()->listItem()) createListItem(static_cast<KNFolder*>(f->parent()));
    it=new KNCollectionViewItem(f->parent()->listItem());
    f->setListItem(it);
  }
  f->setListItem(it);
  it->setPixmap(0, knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::folder));
  f->updateListItem();
}


/*void KNFolderManager::compactFolder(KNFolder *f)
{
  KNCleanUp cup;
  if(!f) f=c_urrentFolder;
  if(!f) return;
  cup.folder(f);
}


void KNFolderManager::compactAll(KNCleanupProgress *p)
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
*/

