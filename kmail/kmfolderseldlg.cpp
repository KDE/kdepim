// kmfolderseldlg.cpp

#include <config.h>
#include "kmfolderseldlg.h"
#include "kmfoldertree.h"
#include "kmfolder.h"
#include "kmmainwidget.h"
#include "globalsettings.h"

#include <kdebug.h>
#include <klineedit.h>
#include <kpopupmenu.h>
#include <kiconloader.h>

#include <tqlayout.h>
#include <tqtoolbutton.h>

namespace KMail {

class FolderItem : public KFolderTreeItem
{
  public:
    FolderItem( KFolderTree * listView );
    FolderItem( KFolderTreeItem * listViewItem );

    void setFolder( KMFolder * folder ) { mFolder = folder; };
    const KMFolder * folder() { return mFolder; };

    // Redefine isAlternate() for proper row coloring behavior.
    // KListViewItem::isAlternate() is not virtual!  Therefore,
    // it is necessary to overload paintCell() below.  If it were
    // made virtual, paintCell() would no longer be necessary.
    bool isAlternate () {
      return mAlternate;
    }

    // Set the flag which determines if this is an alternate row
    void setAlternate ( bool alternate ) {
      mAlternate = alternate;
    }

    // Must overload paintCell because neither KListViewItem::isAlternate()
    // or KListViewItem::backgroundColor() are virtual!
    virtual void paintCell( TQPainter *p, const TQColorGroup &cg,
                             int column, int width, int alignment )
    {
      KListView* view = static_cast< KListView* >( listView() );

      // Set alternate background to invalid
      TQColor nocolor;
      TQColor alt = view->alternateBackground();
      view->setAlternateBackground( nocolor );

      // Set the base and text to the appropriate colors
      TQColorGroup *cgroup = (TQColorGroup *)&view->viewport()->colorGroup();
      TQColor base = cgroup->base();
      TQColor text = cgroup->text();
      cgroup->setColor( TQColorGroup::Base, isAlternate() ? alt : base );
      cgroup->setColor( TQColorGroup::Text, isEnabled() ? text : Qt::lightGray );

      // Call the parent paint routine
      KListViewItem::paintCell( p, cg, column, width, alignment );

      // Restore the base and alternate background
      cgroup->setColor( TQColorGroup::Base, base );
      cgroup->setColor( TQColorGroup::Text, text );
      view->setAlternateBackground( alt );
    }

  private:
    KMFolder * mFolder;
    bool mAlternate;
};

//-----------------------------------------------------------------------------
FolderItem::FolderItem( KFolderTree * listView )
  : KFolderTreeItem( listView ),
    mFolder( 0 )
{}

//-----------------------------------------------------------------------------
FolderItem::FolderItem( KFolderTreeItem * listViewItem )
  : KFolderTreeItem( listViewItem ),
    mFolder( 0 )
{}

//-----------------------------------------------------------------------------
SimpleFolderTree::SimpleFolderTree( TQWidget * parent,
                                    KMFolderTree * folderTree,
                                    const TQString & preSelection,
                                    bool mustBeReadWrite )
  : KFolderTree( parent ), mFolderTree( folderTree )
{
  setSelectionModeExt( Single );
  mFolderColumn = addColumn( i18n( "Folder" ), 0 );
  mPathColumn = addColumn( i18n( "Path" ), 0 );
  setAllColumnsShowFocus( true );
  setAlternateBackground( TQColor( 0xf0, 0xf0, 0xf0 ) );

  reload( mustBeReadWrite, true, true, preSelection );
  readColorConfig();

  applyFilter( "" );

  connect(this, TQT_SIGNAL(collapsed(TQListViewItem*)), TQT_SLOT(recolorRows()));
  connect(this, TQT_SIGNAL(expanded(TQListViewItem*)),  TQT_SLOT(recolorRows()));

  connect( this, TQT_SIGNAL( contextMenuRequested( TQListViewItem*, const TQPoint &, int ) ),
           this, TQT_SLOT( slotContextMenuRequested( TQListViewItem*, const TQPoint & ) ) );
}

//-----------------------------------------------------------------------------
void SimpleFolderTree::reload( bool mustBeReadWrite, bool showOutbox,
                               bool showImapFolders, const TQString& preSelection )
{
  mLastMustBeReadWrite = mustBeReadWrite;
  mLastShowOutbox = showOutbox;
  mLastShowImapFolders = showImapFolders;

  clear();
  FolderItem * lastItem = 0;
  FolderItem * lastTopItem = 0;
  FolderItem * selectedItem = 0;
  int lastDepth = 0;

  TQString selected = preSelection;
  if ( selected.isEmpty() && folder() )
    selected = folder()->idString();

  mFilter = "";
  TQString path;

  for ( TQListViewItemIterator it( mFolderTree ) ; it.current() ; ++it )
  {
    KMFolderTreeItem * fti = static_cast<KMFolderTreeItem *>( it.current() );

    // search folders are never shown
    if ( !fti || fti->protocol() == KFolderTreeItem::Search )
      continue;

    // imap folders?
    if ( fti->protocol() == KFolderTreeItem::Imap && !showImapFolders )
      continue;

    // the outbox?
    if ( fti->type() == KFolderTreeItem::Outbox && !showOutbox )
      continue;

    int depth = fti->depth();// - 1;
    FolderItem * item = 0;
    if ( depth <= 0 ) {
      // top level - first top level item or after last existing top level item
      item = new FolderItem( this );
      if ( lastTopItem )
        item->moveItem( lastTopItem );
      lastTopItem = item;
      depth = 0;
      path  = "";
    }
    else {
      if ( depth > lastDepth ) {
        // next lower level - parent node will get opened
        item = new FolderItem( lastItem );
        lastItem->setOpen(true);
      }
      else {
        path = path.section( '/', 0, -2 - (lastDepth-depth) );

        if ( depth == lastDepth ) {
          // same level - behind previous item
          item = new FolderItem( static_cast<FolderItem*>(lastItem->parent()) );
          item->moveItem( lastItem );
        } else if ( depth < lastDepth ) {
          // above previous level - might be more than one level difference
          // but highest possibility is top level
          while ( ( depth <= --lastDepth ) && lastItem->parent() ) {
            lastItem = static_cast<FolderItem *>( lastItem->parent() );
          }
          if ( lastItem->parent() ) {
            item = new FolderItem( static_cast<FolderItem*>(lastItem->parent()) );
            item->moveItem( lastItem );
          } else {
            // chain somehow broken - what does cause this ???
            kdDebug( 5006 ) << "You shouldn't get here: depth=" << depth
                            << "folder name=" << fti->text( 0 ) << endl;
            item = new FolderItem( this );
            lastTopItem = item;
          }
        }
      }
    }

    if ( depth > 0 )
      path += "/";
    path += fti->text( 0 );

    item->setText( mFolderColumn, fti->text( 0 ) );
    item->setText( mPathColumn, path );

    item->setProtocol( fti->protocol() );
    item->setType( fti->type() );

    // Make items without folders and readonly items unselectable
    // if we're told so
    if ( mustBeReadWrite && ( !fti->folder() || fti->folder()->isReadOnly() ) ) {
      item->setSelectable( false );
    } else {
      if ( fti->folder() ) {
        item->setFolder( fti->folder() );
        if ( selected == item->folder()->idString() )
          selectedItem = item;
      }
    }
    lastItem = item;
    lastDepth = depth;
  }

  if ( selectedItem ) {
    setSelected( selectedItem, true );
    ensureItemVisible( selectedItem );
  }
}

//-----------------------------------------------------------------------------
const KMFolder * SimpleFolderTree::folder() const
{
  TQListViewItem * item = currentItem();
  if ( item ) {
    const KMFolder * folder = static_cast<FolderItem *>( item )->folder();
    if( folder ) return folder;
  }
  return 0;
}

//-----------------------------------------------------------------------------
void SimpleFolderTree::setFolder( KMFolder *folder )
{
  for ( TQListViewItemIterator it( this ) ; it.current() ; ++it )
  {
    const KMFolder *fld = static_cast<FolderItem *>( it.current() )->folder();
    if ( fld == folder )
    {
      setSelected( it.current(), true );
      ensureItemVisible( it.current() );
    }
  }
}

//-----------------------------------------------------------------------------
void SimpleFolderTree::setFolder( const TQString& idString )
{
  setFolder( kmkernel->findFolderById( idString ) );
}

//-----------------------------------------------------------------------------
void SimpleFolderTree::addChildFolder()
{
  const KMFolder *fld = folder();
  if ( fld ) {
    mFolderTree->addChildFolder( (KMFolder *) fld, parentWidget() );
    reload( mLastMustBeReadWrite, mLastShowOutbox, mLastShowImapFolders );
    setFolder( (KMFolder *) fld );
  }
}

//-----------------------------------------------------------------------------
void SimpleFolderTree::slotContextMenuRequested( TQListViewItem *lvi,
                                                 const TQPoint &p )
{
  if (!lvi)
    return;
  setCurrentItem( lvi );
  setSelected( lvi, TRUE );

  const KMFolder * folder = static_cast<FolderItem *>( lvi )->folder();
  if ( !folder || folder->noContent() || folder->noChildren() )
    return;

  KPopupMenu *folderMenu = new KPopupMenu;
  folderMenu->insertTitle( folder->label() );
  folderMenu->insertSeparator();
  folderMenu->insertItem(SmallIconSet("folder_new"),
                          i18n("&New Subfolder..."), this,
                          TQT_SLOT(addChildFolder()));
  kmkernel->setContextMenuShown( true );
  folderMenu->exec (p, 0);
  kmkernel->setContextMenuShown( false );
  delete folderMenu;
  folderMenu = 0;
}

//-----------------------------------------------------------------------------
void SimpleFolderTree::readColorConfig (void)
{
  TQColor c1=TQColor(kapp->palette().active().text());
  TQColor c2=TQColor(kapp->palette().active().base());

  mPaintInfo.colFore = c1;
  mPaintInfo.colBack = c2;

  TQPalette newPal = kapp->palette();
  newPal.setColor( TQColorGroup::Base, mPaintInfo.colBack );
  newPal.setColor( TQColorGroup::Text, mPaintInfo.colFore );
  setPalette( newPal );
}


//-----------------------------------------------------------------------------
static int recurseFilter( TQListViewItem * item, const TQString& filter, int column )
{
  if ( item == 0 )
    return 0;

  TQListViewItem * child;
  child = item->firstChild();

  int enabled = 0;
  while ( child ) {
    enabled += recurseFilter( child, filter, column );
    child = child->nextSibling();
  }

  if ( filter.length() == 0 ||
       item->text( column ).find( filter, 0, false ) >= 0 ) {
    item->setVisible( true );
    ++enabled;
  }
  else {
    item->setVisible( !!enabled );
    item->setEnabled( false );
  }

  return enabled;
}

void SimpleFolderTree::recolorRows()
{
  // Iterate through the list to set the alternate row flags.
  int alt = 0;
  TQListViewItemIterator it ( this );
  while ( it.current() ) {
    FolderItem * item = static_cast< FolderItem* >( it.current() );

    if ( item->isVisible() ) {
      bool visible = true;
      TQListViewItem * parent = item->parent();
      while ( parent ) {
        if (!parent->isOpen()) {
          visible = false;
          break;
        }
        parent = parent->parent();
      }

      if ( visible ) {
        item->setAlternate( alt );
        alt = !alt;
      }
    }

    ++it;
  }
}
    
void SimpleFolderTree::applyFilter( const TQString& filter )
{
  // Reset all items to visible, enabled, and open
  TQListViewItemIterator clean( this );
  while ( clean.current() ) {
    TQListViewItem * item = clean.current();
    item->setEnabled( true );
    item->setVisible( true );
    item->setOpen( true );
    ++clean;
  }

  mFilter = filter;

  if ( filter.isEmpty() ) {
    setColumnText( mPathColumn, i18n("Path") );
    return;
  }

  // Set the visibility and enabled status of each list item.
  // The recursive algorithm is necessary because visiblity
  // changes are automatically applied to child nodes by Qt.
  TQListViewItemIterator it( this );
  while ( it.current() ) {
    TQListViewItem * item = it.current();
    if ( item->depth() <= 0 )
      recurseFilter( item, filter, mPathColumn );
    ++it;
  }

  // Recolor the rows appropriately
  recolorRows();

  // Iterate through the list to find the first selectable item
  TQListViewItemIterator first ( this );
  while ( first.current() ) {
    FolderItem * item = static_cast< FolderItem* >( first.current() );

    if ( item->isVisible() && item->isSelectable() ) {
      setSelected( item, true );
      ensureItemVisible( item );
      break;
    }

    ++first;
  }

  // Display and save the current filter
  if ( filter.length() > 0 )
    setColumnText( mPathColumn, i18n("Path") + "  ( " + filter + " )" );
  else
    setColumnText( mPathColumn, i18n("Path") );

  mFilter = filter;
}

//-----------------------------------------------------------------------------
void SimpleFolderTree::keyPressEvent( TQKeyEvent *e ) {
  int ch = e->ascii();

  if ( ch >= 32 && ch <= 126 )
    applyFilter( mFilter + ch );

  else if ( ch == 8 || ch == 127 ) {
    if ( mFilter.length() > 0 ) {
      mFilter.truncate( mFilter.length()-1 );
      applyFilter( mFilter );
    }
  }

  else
    KListView::keyPressEvent( e );
}


//-----------------------------------------------------------------------------
KMFolderSelDlg::KMFolderSelDlg( KMMainWidget * parent, const TQString& caption,
    bool mustBeReadWrite, bool useGlobalSettings )
  : KDialogBase( parent, "folder dialog", true, caption,
                 Ok|Cancel|User1, Ok, true,
                 KGuiItem(i18n("&New Subfolder..."), "folder_new",
                   i18n("Create a new subfolder under the currently selected folder"))
               ), // mainwin as parent, modal
    mUseGlobalSettings( useGlobalSettings )
{
  KMFolderTree * ft = parent->folderTree();
  assert( ft );

  TQString preSelection = mUseGlobalSettings ?
    GlobalSettings::self()->lastSelectedFolder() : TQString::null;
  mTreeView = new KMail::SimpleFolderTree( makeVBoxMainWidget(), ft,
                                           preSelection, mustBeReadWrite );
  init();
}

//----------------------------------------------------------------------------
KMFolderSelDlg::KMFolderSelDlg( TQWidget * parent, KMFolderTree * tree,
    const TQString& caption, bool mustBeReadWrite, bool useGlobalSettings )
  : KDialogBase( parent, "folder dialog", true, caption,
                 Ok|Cancel|User1, Ok, true,
                 KGuiItem(i18n("&New Subfolder..."), "folder_new",
                   i18n("Create a new subfolder under the currently selected folder"))
               ), // mainwin as parent, modal
    mUseGlobalSettings( useGlobalSettings )
{
  TQString preSelection = mUseGlobalSettings ?
    GlobalSettings::self()->lastSelectedFolder() : TQString::null;
  mTreeView = new KMail::SimpleFolderTree( makeVBoxMainWidget(), tree,
                                           preSelection, mustBeReadWrite );
  init();
}

//-----------------------------------------------------------------------------
void KMFolderSelDlg::init()
{
  mTreeView->setFocus();
  connect( mTreeView, TQT_SIGNAL( doubleClicked( TQListViewItem*, const TQPoint&, int ) ),
           this, TQT_SLOT( slotSelect() ) );
  connect( mTreeView, TQT_SIGNAL( selectionChanged() ),
           this, TQT_SLOT( slotUpdateBtnStatus() ) );

  readConfig();
}

//-----------------------------------------------------------------------------
KMFolderSelDlg::~KMFolderSelDlg()
{
  const KMFolder * cur = folder();
  if ( cur && mUseGlobalSettings ) {
    GlobalSettings::self()->setLastSelectedFolder( cur->idString() );
  }

  writeConfig();
}


//-----------------------------------------------------------------------------
KMFolder * KMFolderSelDlg::folder( void )
{
  return ( KMFolder * ) mTreeView->folder();
}

//-----------------------------------------------------------------------------
void KMFolderSelDlg::setFolder( KMFolder* folder )
{
  mTreeView->setFolder( folder );
}

//-----------------------------------------------------------------------------
void KMFolderSelDlg::slotSelect()
{
  accept();
}

//-----------------------------------------------------------------------------
void KMFolderSelDlg::slotUser1()
{
  mTreeView->addChildFolder();
}

//-----------------------------------------------------------------------------
void KMFolderSelDlg::slotUpdateBtnStatus()
{
  enableButton( User1, folder() &&
                ( !folder()->noContent() && !folder()->noChildren() ) );
}

//-----------------------------------------------------------------------------
void KMFolderSelDlg::setFlags( bool mustBeReadWrite, bool showOutbox,
                               bool showImapFolders )
{
  mTreeView->reload( mustBeReadWrite, showOutbox, showImapFolders );
}

void KMFolderSelDlg::readConfig()
{
  KConfig *config = KGlobal::config();
  config->setGroup( "FolderSelectionDialog" );

  TQSize size = config->readSizeEntry( "Size" );
  if ( !size.isEmpty() ) resize( size );
  else resize( 500, 300 );

  TQValueList<int> widths = config->readIntListEntry( "ColumnWidths" );
  if ( !widths.isEmpty() ) {
    mTreeView->setColumnWidth(mTreeView->mFolderColumn, widths[0]);
    mTreeView->setColumnWidth(mTreeView->mPathColumn,   widths[1]);
  }
  else {
    int colWidth = width() / 2;
    mTreeView->setColumnWidth(mTreeView->mFolderColumn, colWidth);
    mTreeView->setColumnWidth(mTreeView->mPathColumn,   colWidth);
  }
}

void KMFolderSelDlg::writeConfig()
{
  KConfig *config = KGlobal::config();
  config->setGroup( "FolderSelectionDialog" );
  config->writeEntry( "Size", size() );

  TQValueList<int> widths;
  widths.push_back(mTreeView->columnWidth(mTreeView->mFolderColumn));
  widths.push_back(mTreeView->columnWidth(mTreeView->mPathColumn));
  config->writeEntry( "ColumnWidths", widths );
}

} // namespace KMail

#include "kmfolderseldlg.moc"
