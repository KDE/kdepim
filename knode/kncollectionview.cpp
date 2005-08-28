/*
    KNode, the KDE newsreader
    Copyright (c) 2004-2005 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <qcursor.h>
#include <qheader.h>

#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <kpopupmenu.h>

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
  mActiveItem( 0 ),
  mPopup( 0 )
{
  setDragEnabled(true);
  addAcceptableDropMimetype("x-knode-drag/article", false);
  addAcceptableDropMimetype("x-knode-drag/folder", true);
  addColumn(i18n("Name"),162);
  setDropHighlighter(true);

  // popup menu to enable/disable unread and total columns
  header()->setClickEnabled( true );
  header()->installEventFilter( this );
  mPopup = new KPopupMenu( this );
  mPopup->insertTitle( i18n("View Columns") );
  mPopup->setCheckable( true );
  mUnreadPop = mPopup->insertItem( i18n("Unread Column"), this, SLOT(toggleUnreadColumn()) );
  mTotalPop = mPopup->insertItem( i18n("Total Column"), this, SLOT(toggleTotalColumn()) );

  // add unread and total columns if necessary
  readConfig();

  // load accounts and folders
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

  installEventFilter(this);
}


KNCollectionView::~KNCollectionView()
{
  writeConfig();
}



void KNCollectionView::readConfig()
{
  KConfig *conf = knGlobals.config();
  conf->setGroup( "GroupView" );

  // execute the listview layout stuff only once
  static bool initDone = false;
  if (!initDone) {
    initDone = true;
    const int unreadColumn = conf->readNumEntry("UnreadColumn", 1);
    const int totalColumn = conf->readNumEntry("TotalColumn", 2);

    // we need to _activate_ them in the correct order
    // this is ugly because we can't use header()->moveSection
    // but otherwise the restoreLayout doesn't know that to do
    if (unreadColumn != -1 && unreadColumn < totalColumn)
      addUnreadColumn( i18n("Unread"), 48 );
    if (totalColumn != -1)
      addTotalColumn( i18n("Total"), 36 );
    if (unreadColumn != -1 && unreadColumn > totalColumn)
      addUnreadColumn( i18n("Unread"), 48 );
    updatePopup();

    restoreLayout( knGlobals.config(), "GroupView" );
  }

  // font & color settings
  KNConfig::Appearance *app = knGlobals.configManager()->appearance();
  setFont( app->groupListFont() );

  QPalette p = palette();
  p.setColor( QColorGroup::Base, app->backgroundColor() );
  p.setColor( QColorGroup::Text, app->textColor() );
  setPalette( p );
  setAlternateBackground( app->backgroundColor() );
  // FIXME: make this configurable
  mPaintInfo.colUnread = QColor( "blue" );
  mPaintInfo.colFore = app->textColor();
  mPaintInfo.colBack = app->backgroundColor();
}


void KNCollectionView::writeConfig()
{
  KConfig *conf = knGlobals.config();
  conf->setGroup( "GroupView" );
  saveLayout( knGlobals.config(), "GroupView" );
  conf->writeEntry( "UnreadColumn", unreadIndex() );
  conf->writeEntry( "TotalColumn", totalIndex() );
}



void KNCollectionView::addAccount(KNNntpAccount *a)
{
  // add account item
  KNCollectionViewItem* item = new KNCollectionViewItem( this, KFolderTreeItem::News );
  a->setListItem( item );
  item->setOpen( a->wasOpen() );

  // add groups for this account
  QValueList<KNGroup*> groups = knGlobals.groupManager()->groupsOfAccount( a );
  for ( QValueList<KNGroup*>::Iterator it = groups.begin(); it != groups.end(); ++it ) {
    KNCollectionViewItem *gitem = new KNCollectionViewItem( item, KFolderTreeItem::News );
    (*it)->setListItem( gitem );
    (*it)->updateListItem();
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
  QValueList<KNNntpAccount*>::Iterator it;
  for ( it = am->begin(); it != am->end(); ++it ) {
    removeAccount( *it );
    addAccount( *it );
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
  QValueList<KNFolder*> folders = knGlobals.folderManager()->folders();
  for ( QValueList<KNFolder*>::Iterator it = folders.begin(); it != folders.end(); ++it )
    if ( !(*it)->listItem() )
      addFolder( (*it) );
  // now open the folders if they were open in the last session
  for ( QValueList<KNFolder*>::Iterator it = folders.begin(); it != folders.end(); ++it )
    if ( (*it)->listItem())
      (*it)->listItem()->setOpen( (*it)->wasOpen() );
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


void KNCollectionView::reload()
{
  reloadAccounts();
  reloadFolders();
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


void KNCollectionView::nextGroup()
{
  incCurrentFolder();
  setActive( currentItem() );
}


void KNCollectionView::prevGroup()
{
  decCurrentFolder();
  setActive( currentItem() );
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
  if ( item && item->protocol() == KFolderTreeItem::Local && item->type() == KFolderTreeItem::Other ) {
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



void KNCollectionView::toggleUnreadColumn()
{
  if ( isUnreadActive() )
    removeUnreadColumn();
  else
    addUnreadColumn( i18n("Unread"), 48 );
  mPopup->setItemChecked( mUnreadPop, isUnreadActive() );
  reload();
}


void KNCollectionView::toggleTotalColumn()
{
  if ( isTotalActive() )
    removeTotalColumn();
  else
    addTotalColumn( i18n("Total"), 36 );
  mPopup->setItemChecked( mTotalPop, isTotalActive() );
  reload();
}

void KNCollectionView::updatePopup() const
{
  mPopup->setItemChecked( mUnreadPop, isUnreadActive() );
  mPopup->setItemChecked( mTotalPop, isTotalActive() );
}



bool KNCollectionView::eventFilter(QObject *o, QEvent *e)
{
  if ((e->type() == QEvent::KeyPress) && (static_cast<QKeyEvent*>(e)->key() == Key_Tab)) {
    emit(focusChangeRequest(this));
    if (!hasFocus())  // focusChangeRequest was successful
      return true;
  }

  // header popup menu
  if ( e->type() == QEvent::MouseButtonPress &&
       static_cast<QMouseEvent*>(e)->button() == RightButton &&
       o->isA("QHeader") )
  {
    mPopup->popup( static_cast<QMouseEvent*>(e)->globalPos() );
    return true;
  }

  return KFolderTree::eventFilter(o, e);
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
