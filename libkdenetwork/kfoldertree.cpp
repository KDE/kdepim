#include "kfoldertree.h"
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qheader.h>

//-----------------------------------------------------------------------------
KFolderTreeItem::KFolderTreeItem( KFolderTree *parent, const QString & label,
				  Protocol protocol, Type type )
  : KListViewItem( parent, label ), mProtocol( protocol ), mType( type ),
    mUnread(-1), mTotal(0)
{
}

//-----------------------------------------------------------------------------
KFolderTreeItem::KFolderTreeItem( KFolderTreeItem *parent,
				  const QString & label, Protocol protocol, Type type,
          int unread, int total )
    : KListViewItem( parent, label ), mProtocol( protocol ), mType( type ),
      mUnread( unread ), mTotal( total )
{
}

//-----------------------------------------------------------------------------
QString KFolderTreeItem::key(int column, bool) const
{
  if ( column > 0 ) return text(column);

  // local root-folder
  if ( !parent() && mProtocol == NONE )
    return "\t0";

  QString thiskey;

  /* basic sorting rules: 
     local => imap => news => other
   */
  if (mProtocol == Local)
    thiskey = "\t1";
  else if (mProtocol == CachedImap)
    thiskey = "\t2";
  else if (mProtocol == Imap)
    thiskey = "\t3";
  else if (mProtocol == News)
    thiskey = "\t4";
  else
    thiskey = "\t5";

  // make sure system folders come first when sorting
  if (mType == Inbox)
    thiskey += "\t0";
  else if (mType == Outbox)
    thiskey += "\t1";
  else if (mType == SentMail)
    thiskey += "\t2";
  else if (mType == Trash)
    thiskey += "\t3";
  else if (mType == Drafts)
    thiskey += "\t4";
  else if (mType == Calendar)
    thiskey += "\t5";
  else if (mType == Contacts)
    thiskey += "\t6";
  else if (mType == Notes)
    thiskey += "\t7";
  else if (mType == Tasks)
    thiskey += "\t8";

  // the displayed text
  thiskey += text(0);

  return thiskey;
}

//-----------------------------------------------------------------------------
int KFolderTreeItem::compare( QListViewItem * i, int col, bool ascending ) const 
{
  if (col == 0) 
  {
    // sort by folder
    return key(col, ascending).localeAwareCompare( i->key(col, ascending) );
  }
  else 
  {
    // sort by unread or total-column
    int a = 0, b = 0; 
    if (col == static_cast<KFolderTree*>(listView())->unreadIndex())
    {
      a = mUnread; 
      b = static_cast<KFolderTreeItem*>(i)->unreadCount();
    }
    else if (col == static_cast<KFolderTree*>(listView())->totalIndex())
    {
      a = mTotal; 
      b = static_cast<KFolderTreeItem*>(i)->totalCount();
    }
    
    if ( a == b )
      return 0;
    else 
      return (a < b ? -1 : 1);
  }
}

//-----------------------------------------------------------------------------
void KFolderTreeItem::setUnreadCount( int aUnread )
{
  if ( aUnread < 0 ) return;

  mUnread = aUnread;
  
  QString unread = QString::null;
  if (mUnread == 0)
    unread = "- ";
  else {
    unread.setNum(mUnread);
    unread += " ";
  }

  setText( static_cast<KFolderTree*>(listView())->unreadIndex(), 
      unread );
}

//-----------------------------------------------------------------------------
void KFolderTreeItem::setTotalCount( int aTotal )
{
  if ( aTotal < 0 ) return;

  mTotal = aTotal;

  QString total = QString::null;
  if (mTotal == 0)
    total = "- ";
  else {
    total.setNum(mTotal);
    total += " ";
  }

  setText( static_cast<KFolderTree*>(listView())->totalIndex(), 
      total );
}

//-----------------------------------------------------------------------------
int KFolderTreeItem::countUnreadRecursive()
{
  int count = (mUnread > 0) ? mUnread : 0;

  for ( QListViewItem *item = firstChild() ;
      item ; item = item->nextSibling() )
  {
    count += static_cast<KFolderTreeItem*>(item)->countUnreadRecursive();
  }

  return count;
}

//-----------------------------------------------------------------------------
void KFolderTreeItem::paintCell( QPainter * p, const QColorGroup & cg,
                                  int column, int width, int align )
{
  QListView *lv = listView();
  QString oldText = text(column);

  // set an empty text so that we can have our own implementation (see further down)
  // but still benefit from KListView::paintCell
  setText( column, "" );
  
  KListViewItem::paintCell( p, cg, column, width, align );

  KFolderTree *ft = static_cast<KFolderTree*>(listView());
  int r = lv ? lv->itemMargin() : 1;
  const QPixmap *icon = pixmap( column );
  int marg = lv ? lv->itemMargin() : 1;

  QString t;
  QRect br;
  setText( column, oldText );
  if ( isSelected() )
    p->setPen( cg.highlightedText() );
  else
    p->setPen( ft->paintInfo().colFore );

  if ( icon ) {
    r += icon->width() + lv->itemMargin();
  }
  t = text( column );
  if ( !t.isEmpty() ) 
  {
    // use a bold-font for the folder- and the unread-columns
    if ( countUnreadRecursive() > 0 &&
        (column == 0 || column == ft->unreadIndex()) ) 
    {
      QFont f = p->font();
      f.setWeight(QFont::Bold);
      p->setFont(f);
    }
    p->drawText( r, 0, width-marg-r, height(),
        align | AlignVCenter, t, -1, &br );
    if (!isSelected())
      p->setPen( ft->paintInfo().colUnread );
    if (column == 0) {
      // draw the unread-count if the unread-column is not active
      QString unread = QString::null;
      if ( !ft->isUnreadActive() && mUnread > 0 ) 
        unread = " (" + QString::number(mUnread) + ")";
      p->drawText( br.right(), 0, width-marg-br.right(), height(),
          align | AlignVCenter, unread );
    }
  } // end !t.isEmpty()
}


//=============================================================================


KFolderTree::KFolderTree( QWidget *parent, const char* name )
  : KListView( parent, name ), mUnreadIndex(-1), mTotalIndex(-1)
{
  // GUI-options
  setLineWidth(0);
  setAcceptDrops(true);
  setDropVisualizer(false);  
  setAllColumnsShowFocus(true);
  setShowSortIndicator(true);
  setUpdatesEnabled(true);
  setItemsRenameable(false);
  setRootIsDecorated(true);
  setSelectionModeExt(Extended);
  setAlternateBackground(QColor());
  setFullWidth(true);
}

//-----------------------------------------------------------------------------
void KFolderTree::drawContentsOffset( QPainter * p, int ox, int oy,
                                       int cx, int cy, int cw, int ch )
{
  bool oldUpdatesEnabled = isUpdatesEnabled();
  setUpdatesEnabled(false);
  KListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );
  setUpdatesEnabled(oldUpdatesEnabled);
}

//-----------------------------------------------------------------------------
void KFolderTree::contentsMousePressEvent( QMouseEvent *e )
{
    setSelectionModeExt(Single);
    KListView::contentsMousePressEvent(e);
}

//-----------------------------------------------------------------------------
void KFolderTree::contentsMouseReleaseEvent( QMouseEvent *e )
{
    KListView::contentsMouseReleaseEvent(e);
    setSelectionModeExt(Extended);
}

//-----------------------------------------------------------------------------
void KFolderTree::addAcceptableDropMimetype( const char *mimeType, bool outsideOk )
{
  int oldSize = mAcceptableDropMimetypes.size();
  mAcceptableDropMimetypes.resize(oldSize+1);
  mAcceptOutside.resize(oldSize+1);

  mAcceptableDropMimetypes.at(oldSize) =  mimeType;
  mAcceptOutside.setBit(oldSize, outsideOk);
}

//-----------------------------------------------------------------------------
bool KFolderTree::acceptDrag( QDropEvent* event ) const
{
  QListViewItem* item = itemAt(contentsToViewport(event->pos()));

  for (uint i = 0; i < mAcceptableDropMimetypes.size(); i++) 
  {
    if (event->provides(mAcceptableDropMimetypes[i])) 
    {
      if (item) 
        return (static_cast<KFolderTreeItem*>(item))->acceptDrag(event);
      else
        return mAcceptOutside[i];
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
void KFolderTree::addUnreadColumn( const QString & name, int width )
{
  mUnreadIndex = addColumn( name, width );
  setColumnAlignment( mUnreadIndex, qApp->reverseLayout() ? Qt::AlignLeft : Qt::AlignRight );
}

//-----------------------------------------------------------------------------
void KFolderTree::addTotalColumn( const QString & name, int width )
{
  mTotalIndex = addColumn( name, width );
  setColumnAlignment( mTotalIndex, qApp->reverseLayout() ? Qt::AlignLeft : Qt::AlignRight );
}

//-----------------------------------------------------------------------------
void KFolderTree::removeUnreadColumn()
{
  if ( !isUnreadActive() ) return;
  removeColumn( mUnreadIndex );
  if ( isTotalActive() && mTotalIndex > mUnreadIndex )
    mTotalIndex--;
  mUnreadIndex = -1;
  header()->adjustHeaderSize();
}

//-----------------------------------------------------------------------------
void KFolderTree::removeTotalColumn()
{
  if ( !isTotalActive() ) return;
  removeColumn( mTotalIndex );
  if ( isUnreadActive() && mTotalIndex < mUnreadIndex )
    mUnreadIndex--;
  mTotalIndex = -1;
  header()->adjustHeaderSize();
}

//-----------------------------------------------------------------------------
void KFolderTree::setFullWidth( bool fullWidth )
{
  if (fullWidth)
    header()->setStretchEnabled( true, 0 );
}

#include "kfoldertree.moc"
