/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "favoritefolderview.h"

#include "kmfolder.h"
#include "kmfoldermgr.h"
#include "kmfolderseldlg.h"
#include "kmmainwidget.h"
#include "kmailicalifaceimpl.h"
#include "folderstorage.h"
#include "kmfolderimap.h"
#include "kmfoldercachedimap.h"
#include "kmacctcachedimap.h"
#include "folderviewtooltip.h"
#include "korghelper.h"

#include <libkdepim/maillistdrag.h>
#include <libkdepim/kaddrbook.h>

#include <dcopclient.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kio/global.h>

#include <tqheader.h>
#include <tqtimer.h>

#include <cassert>

using namespace KMail;

FavoriteFolderViewItem::FavoriteFolderViewItem(FavoriteFolderView * parent, const TQString & name, KMFolder * folder)
  : KMFolderTreeItem( parent, name, folder ),
  mOldName( folder->label() )
{
  // same stuff as in KMFolderTreeItem again, this time even with virtual methods working
  init();
  connect( folder, TQT_SIGNAL(nameChanged()), TQT_SLOT(nameChanged()) );
  connect( folder, TQT_SIGNAL(iconsChanged()), TQT_SLOT(slotIconsChanged()) );

  connect( folder, TQT_SIGNAL(msgAdded(KMFolder*,Q_UINT32)), TQT_SLOT(updateCount()) );
  connect( folder, TQT_SIGNAL(numUnreadMsgsChanged(KMFolder*)), TQT_SLOT(updateCount()) );
  connect( folder, TQT_SIGNAL(msgRemoved(KMFolder*)), TQT_SLOT(updateCount()) );
  connect( folder, TQT_SIGNAL(folderSizeChanged( KMFolder* )), TQT_SLOT(updateCount()) );

  TQTimer::singleShot( 0, this, TQT_SLOT(updateCount()) );

  if ( unreadCount() > 0 )
    setPixmap( 0, unreadIcon( iconSize() ) );
  else
    setPixmap( 0, normalIcon( iconSize() ) );
}

void FavoriteFolderViewItem::nameChanged()
{
  TQString txt = text( 0 );
  txt.replace( mOldName, folder()->label() );
  setText( 0, txt );
  mOldName = folder()->label();
}

TQValueList<FavoriteFolderView*> FavoriteFolderView::mInstances;

FavoriteFolderView::FavoriteFolderView( KMMainWidget *mainWidget, TQWidget * parent) :
    FolderTreeBase( mainWidget, parent ),
    mContextMenuItem( 0 ),
    mReadingConfig( false )
{
  assert( mainWidget );
  addColumn( i18n("Favorite Folders") );
  setResizeMode( LastColumn );
  header()->setClickEnabled( false );
  setDragEnabled( true );
  setAcceptDrops( true );
  setRootIsDecorated( false );
  setSelectionModeExt( KListView::Single );
  setSorting( -1 );
  setShowSortIndicator( false );

  connect( this, TQT_SIGNAL(selectionChanged()), TQT_SLOT(selectionChanged()) );
  connect( this, TQT_SIGNAL(clicked(TQListViewItem*)), TQT_SLOT(itemClicked(TQListViewItem*)) );
  connect( this, TQT_SIGNAL(dropped(TQDropEvent*,TQListViewItem*)), TQT_SLOT(dropped(TQDropEvent*,TQListViewItem*)) );
  connect( this, TQT_SIGNAL(contextMenuRequested(TQListViewItem*, const TQPoint &, int)),
           TQT_SLOT(contextMenu(TQListViewItem*,const TQPoint&)) );
  connect( this, TQT_SIGNAL(moved()), TQT_SLOT(notifyInstancesOnChange()) );
  connect( this, TQT_SIGNAL(triggerRefresh()), TQT_SLOT(refresh()) );

  connect( kmkernel->folderMgr(), TQT_SIGNAL(changed()), TQT_SLOT(initializeFavorites()) );
  connect( kmkernel->dimapFolderMgr(), TQT_SIGNAL(changed()), TQT_SLOT(initializeFavorites()) );
  connect( kmkernel->imapFolderMgr(), TQT_SIGNAL(changed()), TQT_SLOT(initializeFavorites()) );
  connect( kmkernel->searchFolderMgr(), TQT_SIGNAL(changed()), TQT_SLOT(initializeFavorites()) );

  connect( kmkernel->folderMgr(), TQT_SIGNAL(folderRemoved(KMFolder*)), TQT_SLOT(folderRemoved(KMFolder*)) );
  connect( kmkernel->dimapFolderMgr(), TQT_SIGNAL(folderRemoved(KMFolder*)), TQT_SLOT(folderRemoved(KMFolder*)) );
  connect( kmkernel->imapFolderMgr(), TQT_SIGNAL(folderRemoved(KMFolder*)), TQT_SLOT(folderRemoved(KMFolder*)) );
  connect( kmkernel->searchFolderMgr(), TQT_SIGNAL(folderRemoved(KMFolder*)), TQT_SLOT(folderRemoved(KMFolder*)) );

  TQFont f = font();
  f.setItalic( true );
  setFont( f );

  new FolderViewToolTip( this );

  mInstances.append( this );
}

FavoriteFolderView::~FavoriteFolderView()
{
  mInstances.remove( this );
}

void FavoriteFolderView::readConfig()
{
  mReadingConfig = true;
  clear();
  TQValueList<int> folderIds = GlobalSettings::self()->favoriteFolderIds();
  TQStringList folderNames = GlobalSettings::self()->favoriteFolderNames();
  TQListViewItem *afterItem = 0;
  for ( uint i = 0; i < folderIds.count(); ++i ) {
    KMFolder *folder = kmkernel->folderMgr()->findById( folderIds[i] );
    if ( !folder )
      folder = kmkernel->imapFolderMgr()->findById( folderIds[i] );
    if ( !folder )
      folder = kmkernel->dimapFolderMgr()->findById( folderIds[i] );
    if ( !folder )
      folder = kmkernel->searchFolderMgr()->findById( folderIds[i] );
    TQString name;
    if ( folderNames.count() > i )
      name = folderNames[i];
    afterItem = addFolder( folder, name, afterItem );
  }
  if ( firstChild() )
    ensureItemVisible( firstChild() );

  // folder tree is not yet populated at this point
  TQTimer::singleShot( 0, this, TQT_SLOT(initializeFavorites()) );

  readColorConfig();
  mReadingConfig = false;
}

void FavoriteFolderView::writeConfig()
{
  TQValueList<int> folderIds;
  TQStringList folderNames;
  for ( TQListViewItemIterator it( this ); it.current(); ++it ) {
    KMFolderTreeItem *fti = static_cast<KMFolderTreeItem*>( it.current() );
    folderIds << fti->folder()->id();
    folderNames << fti->text( 0 );
  }
  GlobalSettings::self()->setFavoriteFolderIds( folderIds );
  GlobalSettings::self()->setFavoriteFolderNames( folderNames );
}

bool FavoriteFolderView::acceptDrag(TQDropEvent * e) const
{
  KMFolderTree *ft = mainWidget()->folderTree();
  assert( ft );
  if ( e->provides( "application/x-qlistviewitem" ) &&
       (e->source() == ft->viewport() || e->source() == viewport() ) )
    return true;
  return FolderTreeBase::acceptDrag( e );
}

KMFolderTreeItem* FavoriteFolderView::addFolder(KMFolder * folder, const TQString &name, TQListViewItem *after)
{
  if ( !folder )
    return 0;
  KMFolderTreeItem *item = new FavoriteFolderViewItem( this, name.isEmpty() ? folder->label() : name, folder );
  if ( after )
    item->moveItem( after );
  else
    item->moveItem( lastItem() );
  ensureItemVisible( item );
  insertIntoFolderToItemMap( folder, item );
  notifyInstancesOnChange();
  return item;
}

void FavoriteFolderView::selectionChanged()
{
  KMFolderTreeItem *fti = static_cast<KMFolderTreeItem*>( selectedItem() );
  if ( !fti )
    return;
  KMFolderTree *ft = mainWidget()->folderTree();
  assert( ft );
  assert( fti );
  ft->showFolder( fti->folder() );
  handleGroupwareFolder( fti );
}

void FavoriteFolderView::handleGroupwareFolder( KMFolderTreeItem *fti )
{
  if ( !fti || !fti->folder() || !fti->folder()->storage() )
    return;
  switch ( fti->folder()->storage()->contentsType() ) {
    case KMail::ContentsTypeContact:
      KAddrBookExternal::openAddressBook( this );
      break;
    case KMail::ContentsTypeNote:
    {
      TQByteArray arg;
      TQDataStream s( arg, IO_WriteOnly );
      s << TQString( "kontact_knotesplugin" );
      kapp->dcopClient()->send( "kontact", "KontactIface", "selectPlugin(TQString)", arg );
      break;
    }
    case KMail::ContentsTypeCalendar:
    case KMail::ContentsTypeTask:
    case KMail::ContentsTypeJournal:
    {
      KMail::KorgHelper::ensureRunning();
      TQByteArray arg;
      TQDataStream s( arg, IO_WriteOnly );
      switch ( fti->folder()->storage()->contentsType() ) {
        case KMail::ContentsTypeCalendar:
          s << TQString( "kontact_korganizerplugin" ); break;
        case KMail::ContentsTypeTask:
          s << TQString( "kontact_todoplugin" ); break;
        case KMail::ContentsTypeJournal:
          s << TQString( "kontact_journalplugin" ); break;
        default: assert( false );
      }
      kapp->dcopClient()->send( "kontact", "KontactIface", "selectPlugin(TQString)", arg );
      break;
    }
    default: break;
  }
}

void FavoriteFolderView::itemClicked(TQListViewItem * item)
{
  if ( !item ) return;
  if ( !item->isSelected() )
    item->setSelected( true );
  item->repaint();
  handleGroupwareFolder( static_cast<KMFolderTreeItem*>( item ) );
}

void FavoriteFolderView::folderTreeSelectionChanged(KMFolder * folder)
{
  blockSignals( true );
  bool found = false;
  for ( TQListViewItemIterator it( this ); it.current(); ++it ) {
    KMFolderTreeItem *fti = static_cast<KMFolderTreeItem*>( it.current() );
    if ( fti->folder() == folder && !fti->isSelected() ) {
      fti->setSelected( true );
      setCurrentItem( fti );
      ensureItemVisible( fti );
      fti->repaint();
      found = true;
    } else if ( fti->folder() != folder && fti->isSelected() ) {
      fti->setSelected( false );
      fti->repaint();
    }
  }
  blockSignals( false );
  if ( !found ) {
    clearSelection();
    setSelectionModeExt( KListView::NoSelection );
    setSelectionModeExt( KListView::Single );
  }
}

void FavoriteFolderView::folderRemoved(KMFolder * folder)
{
  TQValueList<KMFolderTreeItem*> delItems;
  for ( TQListViewItemIterator it( this ); it.current(); ++it ) {
    KMFolderTreeItem *fti = static_cast<KMFolderTreeItem*>( it.current() );
    if ( fti->folder() == folder )
      delItems << fti;
    if ( fti == mContextMenuItem )
      mContextMenuItem = 0;
  }
  for ( uint i = 0; i < delItems.count(); ++i )
    delete delItems[i];
  removeFromFolderToItemMap(folder);
}

void FavoriteFolderView::dropped(TQDropEvent * e, TQListViewItem * after)
{
  TQListViewItem* afterItem = after;
  KMFolderTree *ft = mainWidget()->folderTree();
  assert( ft );
  if ( e->source() == ft->viewport() && e->provides( "application/x-qlistviewitem" ) ) {
    for ( TQListViewItemIterator it( ft ); it.current(); ++it ) {
      if ( !it.current()->isSelected() )
        continue;
      KMFolderTreeItem *fti = static_cast<KMFolderTreeItem*>( it.current() );
      if ( !fti->folder() )
        continue;
      if( !mFolderToItem.contains( fti->folder() ) )
         afterItem = addFolder( fti->folder(), prettyName( fti ), afterItem );
    }
    e->accept();
  }
}

void FavoriteFolderView::contextMenu(TQListViewItem * item, const TQPoint & point)
{
  KMFolderTree *ft = mainWidget()->folderTree();
  assert( ft );
  KMFolderTreeItem *fti = static_cast<KMFolderTreeItem*>( item );
  mContextMenuItem = fti;
  KPopupMenu contextMenu;
  if ( fti && fti->folder() ) {
    mainWidget()->action("mark_all_as_read")->plug( &contextMenu );
    if ( fti->folder()->folderType() == KMFolderTypeImap || fti->folder()->folderType() == KMFolderTypeCachedImap )
      mainWidget()->action("refresh_folder")->plug( &contextMenu );
    if ( fti->folder()->isMailingListEnabled() )
      mainWidget()->action("post_message")->plug( &contextMenu );
    mainWidget()->action("search_messages")->plug( &contextMenu );
    if ( fti->folder()->canDeleteMessages() && ( fti->folder()->count() > 0 ) )
       mainWidget()->action("empty")->plug( &contextMenu );
    contextMenu.insertSeparator();

    contextMenu.insertItem( SmallIconSet("configure_shortcuts"), i18n("&Assign Shortcut..."), fti, TQT_SLOT(assignShortcut()) );
    contextMenu.insertItem( i18n("Expire..."), fti, TQT_SLOT(slotShowExpiryProperties()) );
    mainWidget()->action("modify")->plug( &contextMenu );
    contextMenu.insertSeparator();

    contextMenu.insertItem( SmallIconSet("editdelete"), i18n("Remove From Favorites"),
                            this, TQT_SLOT(removeFolder()) );
    contextMenu.insertItem( SmallIconSet("edit"), i18n("Rename Favorite"), this, TQT_SLOT(renameFolder()) );

  } else {
    contextMenu.insertItem( SmallIconSet("bookmark_add"), i18n("Add Favorite Folder..."),
                            this, TQT_SLOT(addFolder()) );
  }
  contextMenu.exec( point, 0 );
}

void FavoriteFolderView::removeFolder()
{
  KMFolderTreeItem *fti = mContextMenuItem;
  KMFolder *folder = 0;
  if( fti )
    folder = fti->folder();
  delete mContextMenuItem;
  mContextMenuItem = 0;
  removeFromFolderToItemMap(folder);
  notifyInstancesOnChange();
}

void FavoriteFolderView::initializeFavorites()
{
  TQValueList<int> seenInboxes = GlobalSettings::self()->favoriteFolderViewSeenInboxes();
  KMFolderTree *ft = mainWidget()->folderTree();
  assert( ft );
  for ( TQListViewItemIterator it( ft ); it.current(); ++it ) {
    KMFolderTreeItem *fti = static_cast<KMFolderTreeItem*>( it.current() );
    if ( fti->type() == KFolderTreeItem::Inbox && fti->folder() && !seenInboxes.contains( fti->folder()->id() ) ) {
      seenInboxes.append( fti->folder()->id() );
      if ( fti->folder() == kmkernel->inboxFolder() && hideLocalInbox() )
        continue;
      if ( kmkernel->iCalIface().hideResourceFolder( fti->folder() ) )
        continue;
      addFolder( fti->folder(), prettyName( fti ) );
    }
  }
  GlobalSettings::self()->setFavoriteFolderViewSeenInboxes( seenInboxes );
}

void FavoriteFolderView::renameFolder()
{
  if ( !mContextMenuItem )
    return;
  bool ok;
  TQString name = KInputDialog::getText( i18n("Rename Favorite"), i18n("Name"), mContextMenuItem->text( 0 ), &ok, this );
  if ( !ok )
    return;
  mContextMenuItem->setText( 0, name );
  notifyInstancesOnChange();
}

TQString FavoriteFolderView::prettyName(KMFolderTreeItem * fti)
{
  assert( fti );
  assert( fti->folder() );
  TQString name = fti->folder()->label();
  TQListViewItem *accountFti = fti;
  while ( accountFti->parent() )
    accountFti = accountFti->parent();
  if ( fti->type() == KFolderTreeItem::Inbox ) {
    if ( fti->protocol() == KFolderTreeItem::Local || fti->protocol() == KFolderTreeItem::NONE ) {
      name = i18n( "Local Inbox" );
    } else {
      name = i18n( "Inbox of %1" ).arg( accountFti->text( 0 ) );
    }
  } else {
    if ( fti->protocol() != KFolderTreeItem::Local && fti->protocol() != KFolderTreeItem::NONE ) {
      name = i18n( "%1 on %2" ).arg( fti->text( 0 ) ).arg( accountFti->text( 0 ) );
    } else {
      name = i18n( "%1 (local)" ).arg( fti->text( 0 ) );
    }
  }
  return name;
}

void FavoriteFolderView::contentsDragEnterEvent(TQDragEnterEvent * e)
{
  if ( e->provides( "application/x-qlistviewitem" ) ) {
    setDropVisualizer( true );
    setDropHighlighter( false );
  } else if ( e->provides( KPIM::MailListDrag::format() ) ) {
    setDropVisualizer( false );
    setDropHighlighter( true );
  } else {
    setDropVisualizer( false );
    setDropHighlighter( false );
  }
  FolderTreeBase::contentsDragEnterEvent( e );
}

void FavoriteFolderView::readColorConfig()
{
  FolderTreeBase::readColorConfig();
  KConfig* conf = KMKernel::config();
  // Custom/System color support
  KConfigGroupSaver saver(conf, "Reader");
  TQColor c = KGlobalSettings::alternateBackgroundColor();
  if ( !conf->readBoolEntry("defaultColors", true) )
    mPaintInfo.colBack = conf->readColorEntry( "AltBackgroundColor",&c );
  else
    mPaintInfo.colBack = c;

  TQPalette newPal = palette();
  newPal.setColor( TQColorGroup::Base, mPaintInfo.colBack );
  setPalette( newPal );
}

void FavoriteFolderView::addFolder()
{
  KMFolderSelDlg dlg( mainWidget(), i18n("Add Favorite Folder"), false );
  if ( dlg.exec() != TQDialog::Accepted )
    return;
  KMFolder *folder = dlg.folder();
  if ( !folder )
    return;
  if ( mFolderToItem.contains( folder ) )
    return;

  KMFolderTreeItem *fti = findFolderTreeItem( folder );
  addFolder( folder, fti ? prettyName( fti ) : folder->label() );
}

void KMail::FavoriteFolderView::addFolder(KMFolderTreeItem * fti)
{
  if ( !fti || !fti->folder() )
    return;
  if ( !mFolderToItem.contains( fti->folder()  ) )
    addFolder( fti->folder(), prettyName( fti ) );
}

KMFolderTreeItem * FavoriteFolderView::findFolderTreeItem(KMFolder * folder) const
{
  assert( folder );
  KMFolderTree *ft = mainWidget()->folderTree();
  assert( ft );
  for ( TQListViewItemIterator it( ft ); it.current(); ++it ) {
    KMFolderTreeItem *fti = static_cast<KMFolderTreeItem*>( it.current() );
    if ( fti->folder() == folder )
      return fti;
  }
  return 0;
}

void FavoriteFolderView::checkMail()
{
  bool found = false;
  for ( TQListViewItemIterator it( this ); it.current(); ++it ) {
    KMFolderTreeItem *fti = static_cast<KMFolderTreeItem*>( it.current() );
    if ( fti->folder()->folderType() == KMFolderTypeImap || fti->folder()->folderType() == KMFolderTypeCachedImap ) {
      if ( !found )
        if ( !kmkernel->askToGoOnline() )
          break;
      found = true;
      if ( fti->folder()->folderType() == KMFolderTypeImap ) {
        KMFolderImap *imap = static_cast<KMFolderImap*>( fti->folder()->storage() );
        imap->getAndCheckFolder();
      } else if ( fti->folder()->folderType() == KMFolderTypeCachedImap ) {
        KMFolderCachedImap* f = static_cast<KMFolderCachedImap*>( fti->folder()->storage() );
        f->account()->processNewMailInFolder( fti->folder() );
      }
    }
  }
}

void FavoriteFolderView::notifyInstancesOnChange()
{
  if ( mReadingConfig )
    return;
  writeConfig();
  for ( TQValueList<FavoriteFolderView*>::ConstIterator it = mInstances.begin(); it != mInstances.end(); ++it ) {
    if ( (*it) == this || (*it)->mReadingConfig )
      continue;
    (*it)->readConfig();
  }
}

void FavoriteFolderView::refresh()
{
  for ( TQListViewItemIterator it( this ) ; it.current() ; ++it ) {
    KMFolderTreeItem* fti = static_cast<KMFolderTreeItem*>(it.current());
    if (!fti || !fti->folder())
      continue;
    fti->repaint();
  }
  update();
}

#include "favoritefolderview.moc"
