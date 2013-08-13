/*
    KNode, the KDE newsreader
    Copyright (c) 2004-2005 Volker Krause <vkrause@kde.org>
    Copyright (c) 2009 Olivier Trichet <nive@nivalis.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "kncollectionview.h"

#include "knglobals.h"
#include "knconfig.h"
#include "knnntpaccount.h"
#include "knaccountmanager.h"
#include "kngroup.h"
#include "kngroupmanager.h"
#include "knfolder.h"
#include "knfoldermanager.h"
#include "kncollectionviewitem.h"
#include "settings.h"

#include <kiconloader.h>
#include <klocale.h>

#include <QDropEvent>
#include <QPainter>



KNCollectionView::KNCollectionView( QWidget *parent ) :
  FolderTreeWidget( parent ),
  mActiveItem( 0 ),
  mDragTargetItem( 0 )
{
  setDragEnabled(true);
  setDropIndicatorShown( false ); // We draw our own (see paintEvent)

  // add unread and total columns if necessary
  loadLayout();

  readConfig();

  // load accounts and folders
  reloadAccounts();
  reloadFolders();

  // connect to the account manager
  KNAccountManager* am = knGlobals.accountManager();
  connect( am, SIGNAL(accountAdded(KNNntpAccount::Ptr)), SLOT(addAccount(KNNntpAccount::Ptr)) );
  connect( am, SIGNAL(accountRemoved(KNNntpAccount::Ptr)), SLOT(removeAccount(KNNntpAccount::Ptr)) );
  connect( am, SIGNAL(accountModified(KNNntpAccount::Ptr)), SLOT(updateAccount(KNNntpAccount::Ptr)) );

  // connect to the group manager
  KNGroupManager* gm = knGlobals.groupManager();
  connect( gm, SIGNAL(groupAdded(KNGroup::Ptr)), SLOT(addGroup(KNGroup::Ptr)) );
  connect( gm, SIGNAL(groupRemoved(KNGroup::Ptr)), SLOT(removeGroup(KNGroup::Ptr)) );
  connect( gm, SIGNAL(groupUpdated(KNGroup::Ptr)), SLOT(updateGroup(KNGroup::Ptr)) );

  // connect to the folder manager
  KNFolderManager* fm = knGlobals.folderManager();
  connect( fm, SIGNAL(folderAdded(KNFolder::Ptr)), SLOT(addPendingFolders()) );
  connect( fm, SIGNAL(folderRemoved(KNFolder::Ptr)), SLOT(removeFolder(KNFolder::Ptr)) );
  connect( fm, SIGNAL(folderActivated(KNFolder::Ptr)), SLOT(activateFolder(KNFolder::Ptr)) );

  // Edition of label
  setEditTriggers( QAbstractItemView::NoEditTriggers );
}


KNCollectionView::~KNCollectionView()
{
  writeConfig();
}


void KNCollectionView::loadLayout()
{
  addLabelColumn( i18n("Name") );
  addUnreadColumn( i18n("Unread") );
  addTotalColumn( i18n("Total") );

  restoreLayout( knGlobals.config(), "GroupView" );

  if ( sortColumn() == -1 ) {
    sortByColumn( labelColumnIndex(), Qt::AscendingOrder );
  }
}

void KNCollectionView::readConfig()
{
  // font
  setFont( knGlobals.settings()->groupListFont() );
}


void KNCollectionView::writeConfig()
{
  saveLayout( knGlobals.config(), "GroupView" );
}



void KNCollectionView::addAccount( KNNntpAccount::Ptr a )
{
  // add account item
  KNCollectionViewItem* item = new KNCollectionViewItem( this, FolderTreeWidgetItem::News );
  a->setListItem( item );
  item->setExpanded( a->wasOpen() );

  // add groups for this account
  KNGroup::List groups = knGlobals.groupManager()->groupsOfAccount( a );
  for ( KNGroup::List::Iterator it = groups.begin(); it != groups.end(); ++it ) {
    KNCollectionViewItem *gitem = new KNCollectionViewItem( item, FolderTreeWidgetItem::News );
    (*it)->setListItem( gitem );
    (*it)->updateListItem();
  }
}


void KNCollectionView::removeAccount( KNNntpAccount::Ptr a )
{
  if(!a->listItem())
    return;
  KNCollectionViewItem *child = 0;
  KNCollectionViewItem *aitem = a->listItem();
  while ( ( child = static_cast<KNCollectionViewItem*>( aitem->takeChild( 0 ) ) ) ) {
    removeGroup( boost::static_pointer_cast<KNGroup>( child->collection() ) );
  }

  delete aitem;
  a->setListItem(0);
}


void KNCollectionView::updateAccount( KNNntpAccount::Ptr a )
{
  a->updateListItem();
}


void KNCollectionView::reloadAccounts()
{
  KNNntpAccount::List list = knGlobals.accountManager()->accounts();
  for ( KNNntpAccount::List::Iterator it = list.begin(); it != list.end(); ++it ) {
    removeAccount( *it );
    addAccount( *it );
  }
}



void KNCollectionView::addGroup( KNGroup::Ptr g )
{
  if (!g->account()->listItem())
    return;

  KNCollectionViewItem *gitem =
      new KNCollectionViewItem( g->account()->listItem(), FolderTreeWidgetItem::News );
  g->setListItem(gitem);
  updateGroup(g);
}


void KNCollectionView::removeGroup( KNGroup::Ptr g )
{
  if (!g->listItem())
    return;

  g->listItem()->setHidden( true ); // work around bug 248256
  delete g->listItem();
  g->setListItem(0);
}


void KNCollectionView::updateGroup( KNGroup::Ptr g )
{
  g->updateListItem();
}



void KNCollectionView::addFolder( KNFolder::Ptr f )
{
  KNCollectionViewItem *it;

  if (!f->parent()) {
    // root folder
    it = new KNCollectionViewItem(this, FolderTreeWidgetItem::Local);
  } else {
    // make sure the parent folder has already been added
    if (!f->parent()->listItem())
      addFolder( boost::static_pointer_cast<KNFolder>( f->parent() ) );
    // handle special folders
    FolderTreeWidgetItem::FolderType type = FolderTreeWidgetItem::Other;
    switch ( f->id() ) {
      case 1:
        type = FolderTreeWidgetItem::Drafts; break;
      case 2:
        type = FolderTreeWidgetItem::Outbox; break;
      case 3:
        type = FolderTreeWidgetItem::SentMail; break;
    }
    it = new KNCollectionViewItem( f->parent()->listItem(), FolderTreeWidgetItem::Local, type );
  }
  f->setListItem( it );
  updateFolder( f );
}


void KNCollectionView::removeFolder( KNFolder::Ptr f)
{
  if(!f->listItem())
    return;
  KNCollectionViewItem *child = 0;
  KNCollectionViewItem *it = f->listItem();
  while ( ( child = static_cast<KNCollectionViewItem*>( it->takeChild( 0 ) ) ) ) {
    removeFolder( boost::static_pointer_cast<KNFolder>( child->collection() ) );
  }

  f->listItem()->setHidden( true ); // work around bug 248256
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
  KNFolder::List folders = knGlobals.folderManager()->folders();
  for ( KNFolder::List::Iterator it = folders.begin(); it != folders.end(); ++it )
    if ( !(*it)->listItem() )
      addFolder( (*it) );
  // now open the folders if they were open in the last session
  for ( KNFolder::List::Iterator it = folders.begin(); it != folders.end(); ++it ) {
    if ( (*it)->listItem()) {
      (*it)->listItem()->setExpanded( (*it)->wasOpen() );
    }
  }
}


void KNCollectionView::activateFolder( KNFolder::Ptr f )
{
  if(f->listItem())
    setActive( f->listItem() );
}


void KNCollectionView::updateFolder( KNFolder::Ptr f )
{
  f->updateListItem();
}


void KNCollectionView::reload()
{
  reloadAccounts();
  reloadFolders();
}

void KNCollectionView::setActive( QTreeWidgetItem *i )
{
  if (!i || mActiveItem == i)
    return;

  i->setSelected( true );
  setCurrentItem( i );
  mActiveItem = i;
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
  QTreeWidgetItemIterator it( currentItem() );
  --it;
  FolderTreeWidgetItem* fti = static_cast<FolderTreeWidgetItem*>( *it );
  if (fti) {
    setFocus();
    setCurrentItem( fti );
  }
}


void KNCollectionView::incCurrentFolder()
{
  QTreeWidgetItemIterator it( currentItem() );
  ++it;
  FolderTreeWidgetItem* fti = static_cast<FolderTreeWidgetItem*>( *it );
  if (fti) {
    setFocus();
    setCurrentItem( fti );
  }
}


void KNCollectionView::selectCurrentFolder()
{
  FolderTreeWidgetItem* fti = static_cast<FolderTreeWidgetItem*>( currentItem() );
  if (fti) {
    setActive( fti );
  }
}

void KNCollectionView::contextMenuEvent( QContextMenuEvent *event )
{
  QTreeWidgetItem *item = itemAt( event->pos() );
  if(item) {
    emit contextMenu( item, event->globalPos() );
  }
}



void KNCollectionView::paintEvent( QPaintEvent *event )
{
  FolderTreeWidget::paintEvent( event );

  if ( mDragTargetItem ) {
    QRect rect = visualItemRect( mDragTargetItem );
    if ( rect.isValid() ) {
      QPainter p( viewport() );
      QBrush brush = KColorScheme( QPalette::Active, KColorScheme::Selection ).decoration( KColorScheme::HoverColor ).color();
      p.setPen( QPen( brush, 2 ) );
      p.drawRect( rect );
    }
  }
}


void KNCollectionView::dragEnterEvent( QDragEnterEvent *event )
{
  if ( event->mimeData() && event->mimeData()->hasFormat( "x-knode-drag/folder" ) ) {
    event->accept();
  } else {
    event->ignore();
  }
}

void KNCollectionView::dragLeaveEvent( QDragLeaveEvent *event )
{
  FolderTreeWidget::dragLeaveEvent( event );

  mDragTargetItem = 0;
  viewport()->update(); // repaint
}

void KNCollectionView::dragMoveEvent( QDragMoveEvent *event )
{
  // Needed for auto-scrolling
  FolderTreeWidget::dragMoveEvent( event );

  handleDragNDropEvent( event, false );
}

void KNCollectionView::dropEvent( QDropEvent *event )
{
  handleDragNDropEvent( event, true );

  mDragTargetItem = 0;
  viewport()->update(); // repaint
}

void KNCollectionView::handleDragNDropEvent( QDropEvent *event, bool enforceDrop )
{
  QTreeWidgetItem *item = itemAt( event->pos() );
  KNCollectionViewItem *fti = static_cast<KNCollectionViewItem*>( item );
  bool accepted = false;
  if ( fti && fti->collection() && fti->collection()->type()==KNCollection::CTfolder ) {
    const QMimeData *md = event->mimeData();
    if( md && md->hasFormat( "x-knode-drag/folder" ) ) {
      KNFolder::Ptr dest = boost::static_pointer_cast<KNFolder>( fti->collection() );
      KNFolderManager *folderManager = KNGlobals::self()->folderManager();
      if ( !enforceDrop ) {
        // Notify that the move is possible.
        accepted = folderManager->canMoveFolder( folderManager->currentFolder(), dest );
      } else {
        // Informs whether the move succeeded.
        accepted = folderManager->moveFolder( folderManager->currentFolder(), dest );
      }
    }
    /* TODO: redo drag and drop of articles.
    else if ( md && md->hasFormat( "message/news" ) ) {
      if ( !static_cast<KNFolder*>( fti->collection() )->isRootFolder() ) {   // don't drop articles on the root folder
        if ( !enforceDrop ) {
          accepted = true;
        } else if(f_olManager->currentFolder()) {
            if (e->dropAction() == Qt::MoveAction) {
              KNLocalArticle::List l;
              getSelectedArticles(l);
              a_rtManager->moveIntoFolder(l, dest);
              accepted = true;
            } else {
              KNArticle::List l;
              getSelectedArticles(l);
              a_rtManager->copyIntoFolder(l, dest);
              accepted = true;
            }
          }
          else if(g_rpManager->currentGroup()) {
            KNArticle::List l;
            getSelectedArticles(l);
            a_rtManager->copyIntoFolder(l, dest);
            accepted = true;
          }
      }
    }
    */
  }

  // Trigger an update of the target item (drawing of target's borders)
  bool repaintTarget =    ( mDragTargetItem == item )       /* target changes */
                       || ( !accepted && mDragTargetItem ); /* just entered an invalid target */
  mDragTargetItem = accepted ? item : 0;
  if ( repaintTarget ) {
    viewport()->update();
  }

  event->setAccepted( accepted );
}

QStringList KNCollectionView::mimeTypes() const
{
  QStringList l;
  l << "x-knode-drag/folder";
  return l;
}

void KNCollectionView::startDrag( Qt::DropActions supportedActions )
{
  Q_UNUSED( supportedActions );

  KNCollectionViewItem *item = static_cast<KNCollectionViewItem*>( currentItem() );
  // Only folders can be dragged
  if ( !item || !item->collection() || item->collection()->type()!=KNCollection::CTfolder ) {
    return;
  }

  KNFolder::Ptr folder = boost::static_pointer_cast<KNFolder>( item->collection() );

  // Cannot drag special folders
  if ( folder->isRootFolder() || folder->isStandardFolder() ) {
    return;
  }

  QMimeData *mimeData = new QMimeData();
  const QByteArray id( QString::number( folder->id() ).toLatin1() );
  mimeData->setData( "x-knode-drag/folder", id );

  QDrag *drag = new QDrag( this );
  drag->setMimeData( mimeData );
  drag->setPixmap( SmallIcon( "folder" ) );

  drag->exec( Qt::MoveAction );
}


#include "kncollectionview.moc"
