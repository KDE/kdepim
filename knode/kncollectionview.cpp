/*
    kncollectionview.cpp

    KNode, the KDE newsreader
    Copyright (c) 2004 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <klocale.h>

#include "knglobals.h"
#include "knconfig.h"
#include "knconfigmanager.h"
#include "knnntpaccount.h"
#include "knaccountmanager.h"
#include "kngroup.h"
#include "kngroupmanager.h"
#include "knfolder.h"
#include "knfoldermanager.h"
#include "knlistview.h"
#include "kncollectionview.h"
#include "kncollectionviewitem.h"


KNCollectionView::KNCollectionView(QWidget *parent, const char* name)
  : KNListView(parent, name)
{
  setAcceptDrops(true);
  setDragEnabled(true);
  addAcceptableDropMimetype("x-knode-drag/article", false);
  addAcceptableDropMimetype("x-knode-drag/folder", true);
  setSelectionModeExt(KListView::Single);
  setFrameStyle(QFrame::Panel | QFrame::Plain);
  setLineWidth(1);
  setTreeStepSize(12);
  setRootIsDecorated(true);
  setShowSortIndicator(true);
  addColumn(i18n("Name"),162);
  addColumn(i18n("Total"),36);
  addColumn(i18n("Unread"),48);
  setColumnAlignment(1,AlignCenter);
  setColumnAlignment(2,AlignCenter);
  setAlternateBackground(QColor());
  
  reloadAccounts();
  reloadFolders();
  
  // connect to the account manager
  KNAccountManager* am = knGlobals.accountManager();
  connect(am, SIGNAL(accountAdded(KNNntpAccount*)), SLOT(addAccount(KNNntpAccount*)));
  connect(am, SIGNAL(accountRemoved(KNNntpAccount*)), SLOT(removeAccount(KNNntpAccount*)));
  connect(am, SIGNAL(accountModified(KNNntpAccount*)), SLOT(updateAccount(KNNntpAccount*)));

  // connect to the group manager
  KNGroupManager* gm = knGlobals.groupManager();
  connect(gm, SIGNAL(groupAdded(KNGroup*)), SLOT(addGroup(KNGroup*)));
  connect(gm, SIGNAL(groupRemoved(KNGroup*)), SLOT(removeGroup(KNGroup*)));
  connect(gm, SIGNAL(groupUpdated(KNGroup*)), SLOT(updateGroup(KNGroup*)));
  
  // connect to the folder manager  
  KNFolderManager* fm = knGlobals.folderManager();
  connect(fm, SIGNAL(folderAdded(KNFolder*)), SLOT(addPendingFolders()));
  connect(fm, SIGNAL(folderRemoved(KNFolder*)), SLOT(removeFolder(KNFolder*)));
  connect(fm, SIGNAL(folderActivated(KNFolder*)), SLOT(activateFolder(KNFolder*)));
}



void KNCollectionView::addAccount(KNNntpAccount *a)
{
  // add account item
  KNCollectionViewItem* it = new KNCollectionViewItem(this);
  a->setListItem(it);
  KNConfig::Appearance *app = knGlobals.cfgManager->appearance();
  it->setPixmap(0, app->icon(KNConfig::Appearance::nntp));
  it->setOpen(a->wasOpen());
  
  // add groups for this account
  QPtrList<KNGroup> groups;
  groups.setAutoDelete(false);
  knGlobals.groupManager()->getGroupsOfAccount(a, &groups);
  for(KNGroup *g = groups.first(); g; g = groups.next()) {
    KNCollectionViewItem *gitem = new KNCollectionViewItem(it);
    gitem->setPixmap(0, app->icon(KNConfig::Appearance::group));
    g->setListItem(gitem);
    g->updateListItem();
  }
}


void KNCollectionView::removeAccount(KNNntpAccount *a)
{
  if(!a->listItem())
    return;
  KNCollectionViewItem *child = 0, *aitem = a->listItem();
  while((child = static_cast<KNCollectionViewItem*>(aitem->firstChild())))
    removeGroup(static_cast<KNGroup*>(child->coll));
  delete aitem;
  a->setListItem(0);
}


void KNCollectionView::updateAccount(KNNntpAccount *a)
{
  a->updateListItem();
}


void KNCollectionView::reloadAccounts()
{
  KNAccountManager* am = knGlobals.accountManager();
  for(KNNntpAccount *a = am->first(); a; a = am->next()) {
    removeAccount(a);
    addAccount(a);
  }
}



void KNCollectionView::addGroup(KNGroup *g)
{
  if (!g->account()->listItem())
    return;
  
  KNCollectionViewItem *gitem = new KNCollectionViewItem(g->account()->listItem());
  gitem->setPixmap(0, knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::group));
  g->setListItem(gitem);
  updateGroup(g);
}


void KNCollectionView::removeGroup(KNGroup *g)
{
  if (!g->listItem())
    return;
      
  delete g->listItem();
  g->setListItem(0);
}


void KNCollectionView::updateGroup(KNGroup *g)
{
  g->updateListItem();
}



void KNCollectionView::addFolder(KNFolder *f)
{
  KNCollectionViewItem *it;
  if (!f->parent()) {
    it = new KNCollectionViewItem(this);
  }
  else {
    if (!f->parent()->listItem())
      addFolder(static_cast<KNFolder*>(f->parent()));
    it = new KNCollectionViewItem(f->parent()->listItem());
  }
  f->setListItem(it);
  QPixmap pix;
  if (f->isRootFolder())
    pix = knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::rootFolder);
  else
    if (f->isStandardFolder())
      pix = knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::customFolder);
    else
      pix = knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::folder);
  it->setPixmap(0, pix);
  updateFolder(f);
}


void KNCollectionView::removeFolder(KNFolder* f)
{
  if(!f->listItem())
    return;
  KNCollectionViewItem *child = 0, *it = f->listItem();
  while((child = static_cast<KNCollectionViewItem*>(it->firstChild())))
    removeFolder(static_cast<KNFolder*>(child->coll));
  delete f->listItem();
  f->setListItem(0);
}


void KNCollectionView::reloadFolders()
{
  // remove existing folder items
  removeFolder(knGlobals.folderManager()->root());
  
  // add folder items
  addPendingFolders();
}


void KNCollectionView::addPendingFolders()
{
  QPtrList<KNFolder> folders = knGlobals.folderManager()->folders();
  for(KNFolder *f = folders.first(); f; f = folders.next())
    if(!f->listItem())
      addFolder(f);
  // now open the folders if they were open in the last session
  for(KNFolder *f = folders.first(); f; f = folders.next())
    if (f->listItem())
      f->listItem()->setOpen(f->wasOpen());
}


void KNCollectionView::activateFolder(KNFolder* f)
{
  if(f->listItem())
    setActive(f->listItem(), true);
}


void KNCollectionView::updateFolder(KNFolder* f)
{
  f->updateListItem();
}

#include "kncollectionview.moc"
