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

#include <qcursor.h>
#include <qheader.h>

#include <kiconloader.h>
#include <klistview.h>
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
#include "headerview.h"
#include "kncollectionview.h"
#include "kncollectionviewitem.h"

KNCollectionView::KNCollectionView(QWidget *parent, const char* name) :
  KFolderTree(parent, name),
  mActiveItem(0)
{
  setDragEnabled(true);
  addAcceptableDropMimetype("x-knode-drag/article", false);
  addAcceptableDropMimetype("x-knode-drag/folder", true);
  addColumn(i18n("Name"),162);
  addTotalColumn(i18n("Total"),36);
  addUnreadColumn(i18n("Unread"),48);
  setDropHighlighter(true);

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

  // we need to repaint if the column size has been changed due to our
  // special group name squeezing
  disconnect(header(), SIGNAL(sizeChange(int,int,int)));
  connect(header(), SIGNAL(sizeChange(int,int,int)), SLOT(slotSizeChanged(int,int,int)));

  installEventFilter(this);
}



void KNCollectionView::addAccount(KNNntpAccount *a)
{
  // add account item
  KNCollectionViewItem* it = new KNCollectionViewItem(this, KFolderTreeItem::News);
  a->setListItem(it);
  it->setOpen(a->wasOpen());

  // add groups for this account
  QPtrList<KNGroup> groups;
  groups.setAutoDelete(false);
  knGlobals.groupManager()->getGroupsOfAccount(a, &groups);
  for(KNGroup *g = groups.first(); g; g = groups.next()) {
    KNCollectionViewItem *gitem = new KNCollectionViewItem(it, KFolderTreeItem::News);
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

  KNCollectionViewItem *gitem =
      new KNCollectionViewItem( g->account()->listItem(), KFolderTreeItem::News );
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
    // root folder
    it = new KNCollectionViewItem(this, KFolderTreeItem::Local);
  } else {
    // make sure the parent folder has already been added
    if (!f->parent()->listItem())
      addFolder( static_cast<KNFolder*>(f->parent()) );
    // handle special folders
    KFolderTreeItem::Type type = KFolderTreeItem::Other;
    switch ( f->id() ) {
      case 1:
        type = KFolderTreeItem::Drafts; break;
      case 2:
        type = KFolderTreeItem::Outbox; break;
      case 3:
        type = KFolderTreeItem::SentMail; break;
    }
    it = new KNCollectionViewItem( f->parent()->listItem(), KFolderTreeItem::Local, type );
  }
  f->setListItem( it );
  updateFolder( f );
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
    setActive( f->listItem() );
}


void KNCollectionView::updateFolder(KNFolder* f)
{
  f->updateListItem();
}



void KNCollectionView::setActive( QListViewItem *i )
{
  if (!i || mActiveItem == i)
    return;

  clearSelection();
  setSelected( i, true );
  setCurrentItem( i );
  mActiveItem = i;
  emit( selectionChanged( i ) );
}


void KNCollectionView::decCurrentFolder()
{
  QListViewItemIterator it( currentItem() );
  --it;
  KFolderTreeItem* fti = static_cast<KFolderTreeItem*>(it.current());
  if (fti) {
    ensureItemVisible( fti );
    setFocus();
    setCurrentItem( fti );
  }
}


void KNCollectionView::incCurrentFolder()
{
  QListViewItemIterator it( currentItem() );
  ++it;
  KFolderTreeItem* fti = static_cast<KFolderTreeItem*>(it.current());
  if (fti) {
    ensureItemVisible( fti );
    setFocus();
    setCurrentItem( fti );
  }
}


void KNCollectionView::selectCurrentFolder()
{
  KFolderTreeItem* fti = static_cast<KFolderTreeItem*>( currentItem() );
  if (fti) {
    ensureItemVisible( fti );
    setActive( fti );
  }
}


QDragObject* KNCollectionView::dragObject()
{
  KFolderTreeItem *item = static_cast<KFolderTreeItem*>
      (itemAt(viewport()->mapFromGlobal(QCursor::pos())));
  if (item->protocol() == KFolderTreeItem::Local && item->type() == KFolderTreeItem::Other) {
    QDragObject *d = new QStoredDrag( "x-knode-drag/folder", viewport() );
    d->setPixmap( SmallIcon("folder") );
    return d;
  }
  return 0;
}


void KNCollectionView::contentsDropEvent( QDropEvent *e )
{
  cleanItemHighlighter(); // necessary since we overwrite KListView::contentsDropEvent()
  QListViewItem *item = itemAt( contentsToViewport(e->pos()) );
  KNCollectionViewItem *fti = static_cast<KNCollectionViewItem*>(item);
  if (fti && (fti->coll) && acceptDrag(e)) {
    emit folderDrop( e, fti );
    e->accept( true );
  }
  else
    e->accept( false );
}


void KNCollectionView::slotSizeChanged(int section, int, int newSize)
{
  viewport()->repaint(
      header()->sectionPos(section), 0, newSize, visibleHeight(), false );
}


bool KNCollectionView::eventFilter(QObject *o, QEvent *e)
{
  if ((e->type() == QEvent::KeyPress) && (static_cast<QKeyEvent*>(e)->key() == Key_Tab)) {
    emit(focusChangeRequest(this));
    if (!hasFocus())  // focusChangeRequest was successful
      return true;
  }
  return KListView::eventFilter(o, e);
}


void KNCollectionView::focusInEvent(QFocusEvent *e)
{
  QListView::focusInEvent(e);
  emit focusChanged(e);
}


void KNCollectionView::focusOutEvent(QFocusEvent *e)
{
  QListView::focusOutEvent(e);
  emit focusChanged(e);
}


#include "kncollectionview.moc"
