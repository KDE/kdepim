/*
    knfoldermanager.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
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
#include <qdir.h>
#include <qhbox.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
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
    KNHelper::displayInternalFileError();
    return;
  }

  KNFolder *f;

  f=new KNFolder(1, i18n("Drafts"), "drafts");
  f_List.append(f);

  f=new KNFolder(2, i18n("Outbox"), "outbox");
  f_List.append(f);

  f=new KNFolder(3, i18n("Sent"), "sent");
  f_List.append(f);

  l_astId=3;

  //custom folders
  loadCustomFolders();

  showListItems();
  setCurrentFolder(0);
}


KNFolderManager::~KNFolderManager()
{
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


void KNFolderManager::newFolder(KNFolder *p)
{
  KNFolder *f=new KNFolder(++l_astId, i18n("New folder"), p);
  f_List.append(f);
  renameFolder(f, true);
}


bool KNFolderManager::deleteFolder(KNFolder *f)
{
  if(f->lockedArticles()>0)
    return false;

  QList<KNFolder> del;
  del.setAutoDelete(false);
  KNFolder *fol;
  KNCollection *p;

  for(fol=f_List.first(); fol; fol=f_List.next()) {
    p=fol->parent();
    while(p) {
      if(p==f) {
        if(fol->lockedArticles()>0)
          return false;
        del.append(fol);
        break;
      }
      p=p->parent();
    }
  }

  del.append(f);
  for(fol=del.first(); fol; fol=del.next()) {
    if(c_urrentFolder==fol)
      c_urrentFolder=0;
    fol->killYourself();
    f_List.removeRef(fol); // deletes fol
  }

  return true;
}


void KNFolderManager::renameFolder(KNFolder *f, bool isNew)
{
  if(!f || f->id()<=3)
    return;


  KDialogBase *dlg =
  new KDialogBase(
    knGlobals.topWidget, 0,true,
    isNew ?  i18n("New folder") : i18n("Rename folder"),
    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok
  );

  QHBox *page = dlg->makeHBoxMainWidget();

  QLabel *label = new QLabel(i18n("&Name:"),page);
  QLineEdit *edit = new QLineEdit(page);
  edit->setText(f->name());

  label->setBuddy(edit);
  edit->setFocus();
  KNHelper::restoreWindowSize("renameFolder", dlg, QSize(325,66));

  if(dlg->exec()) {
    f->setName(edit->text());
    if(!f->listItem())
      showListItems();
    else
      f->updateListItem();
  }
  else if(isNew)
    f_List.removeRef(f);

  delete dlg;
}


bool KNFolderManager::moveFolder(KNFolder *f, KNFolder *p)
{
  if(!f || p==f->parent()) // nothing to be done
    return true;

  // is "p" a child of "f" ?
  KNCollection *p2=p?p->parent():0;
  while(p2) {
    if(p2==f)
      break;
    p2=p2->parent();
  }

  if( (p2 && p2==f) || f==p || f->id()<=3 ) //  no way ;-)
    return false;

  // reparent
  f->setParent(p);
  f->saveInfo();

  // recreate list-item
  delete f->listItem();
  showListItems();
  if(c_urrentFolder==f) {
    v_iew->setActive(f->listItem(), true);
    v_iew->ensureItemVisible(f->listItem());
  }

  return true;
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
    KNHelper::displayInternalFileError();
    return;
  }

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
  int cnt=0;
  QString dir(KGlobal::dirs()->saveLocation("appdata","folders/"));
  KNFolder *f;

  if (dir == QString::null) {
    KNHelper::displayInternalFileError();
    return 0;
  }

  QDir d(dir);
  QStringList entries(d.entryList("*.info"));
  for(QStringList::Iterator it=entries.begin(); it != entries.end(); it++) {
    f=new KNFolder();
    if(f->readInfo(d.absFilePath(*it))) {
      if(f->id()>l_astId)
        l_astId=f->id();
      f_List.append(f);
      cnt++;
    }
    else
      delete f;
  }

  // set parents
  if(cnt>0) {
    QList<KNFolder> l(f_List);
    l.setAutoDelete(false);
    for(f=l.first(); f; f=l.next()) {
      if(f->parentId()>-1)
        f->setParent( folder(f->parentId()) );
      else
        f->setParent(0);
    }
  }

  return cnt;
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
