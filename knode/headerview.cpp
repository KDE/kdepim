/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

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
#include <qtimer.h>

#include <klocale.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kpopupmenu.h>

#include "knglobals.h"
#include "knconfigmanager.h"
#include "headerview.h"
#include "knhdrviewitem.h"
#include "kngroupmanager.h"
#include "knarticle.h"
#include "knarticlemanager.h"
#include "knmainwidget.h"


KNHeaderView::KNHeaderView(QWidget *parent, const char *name) :
  KListView(parent,name),
  mSortCol( -1 ),
  mSortAsc( true ),
  mSortByThreadChangeDate( false ),
  mDelayedCenter( -1 ),
  mActiveItem( 0 ),
  mShowingFolder( false )
{
  mPaintInfo.subCol    = addColumn( i18n("Subject"), 310 );
  mPaintInfo.senderCol = addColumn( i18n("From"), 115 );
  mPaintInfo.scoreCol  = addColumn( i18n("Score"), 42 );
  mPaintInfo.sizeCol   = addColumn( i18n("Lines"), 42 );
  mPaintInfo.dateCol   = addColumn( i18n("Date"), 102 );

  setDropVisualizer( false );
  setDropHighlighter( false );
  setItemsRenameable( false );
  setItemsMovable( false );
  setAcceptDrops( false );
  setDragEnabled( true );
  setAllColumnsShowFocus( true );
  setSelectionMode( QListView::Extended );
  setShowSortIndicator( true );
#if KDE_IS_VERSION( 3, 3, 90 )
  setShadeSortColumn ( true );
#endif
  setRootIsDecorated( true );
  setSorting( mPaintInfo.dateCol );
  header()->setMovingEnabled( true );
  setColumnAlignment( mPaintInfo.sizeCol, Qt::AlignRight );
  setColumnAlignment( mPaintInfo.scoreCol, Qt::AlignRight );

  // due to our own column text squeezing we need to repaint on column resizing
  disconnect( header(), SIGNAL(sizeChange(int, int, int)) );
  connect( header(), SIGNAL(sizeChange(int, int, int)),
           SLOT(slotSizeChanged(int, int, int)) );

  // column selection RMB menu
  mPopup = new KPopupMenu( this );
  mPopup->insertTitle( i18n("View Columns") );
  mPopup->setCheckable( true );
  mPopup->insertItem( i18n("Line Count"),  KPaintInfo::COL_SIZE );
  mPopup->insertItem( i18n("Score"), KPaintInfo::COL_SCORE );

  connect( mPopup, SIGNAL(activated(int)), this, SLOT(toggleColumn(int)) );

  // connect to the article manager
  connect( knGlobals.articleManager(), SIGNAL(aboutToShowGroup()), SLOT(prepareForGroup()) );
  connect( knGlobals.articleManager(), SIGNAL(aboutToShowFolder()), SLOT(prepareForFolder()) );

  new KNHeaderViewToolTip( this );

  installEventFilter( this );
}


KNHeaderView::~KNHeaderView()
{
  // ### crash because KNConfigManager is already deleted here
  // writeConfig();
}


void KNHeaderView::readConfig()
{
  static bool initDone = false;
  if ( !initDone ) {
    initDone = true;
    KConfig *conf = knGlobals.config();
    conf->setGroup( "HeaderView" );
    mSortByThreadChangeDate = conf->readBoolEntry( "sortByThreadChangeDate", false );
    restoreLayout( conf, "HeaderView" );
  }

  KNConfig::ReadNewsGeneral *rngConf = knGlobals.configManager()->readNewsGeneral();
  toggleColumn( KPaintInfo::COL_SIZE, rngConf->showLines() );
  if ( !mShowingFolder ) // score column is always hidden when showing a folder
    toggleColumn( KPaintInfo::COL_SCORE, rngConf->showScore() );

  mDateFormatter.setCustomFormat( rngConf->dateCustomFormat() );
  mDateFormatter.setFormat( rngConf->dateFormat() );

  KNConfig::Appearance *app = knGlobals.configManager()->appearance();
  QPalette p = palette();
  p.setColor( QColorGroup::Base, app->backgroundColor() );
  p.setColor( QColorGroup::Text, app->textColor() );
  setPalette( p );
  setAlternateBackground( app->alternateBackgroundColor() );
  setFont( app->articleListFont() );
}


void KNHeaderView::writeConfig()
{
  KConfig *conf = knGlobals.config();
  conf->setGroup( "HeaderView" );
  conf->writeEntry( "sortByThreadChangeDate", mSortByThreadChangeDate );
  saveLayout( conf, "HeaderView" );

  KNConfig::ReadNewsGeneral *rngConf = knGlobals.configManager()->readNewsGeneral();
  rngConf->setShowLines( mPaintInfo.showSize );
  if ( !mShowingFolder ) // score column is always hidden when showing a folder
    rngConf->setShowScore( mPaintInfo.showScore );
}


void KNHeaderView::setActive( QListViewItem *i )
{
  KNHdrViewItem *item = static_cast<KNHdrViewItem*>( i );

  if ( !item || item->isActive() )
    return;

  if ( mActiveItem ) {
    mActiveItem->setActive( false );
    repaintItem( mActiveItem );
    mActiveItem = 0;
  }

  item->setActive( true );
  setSelected( item, true );
  setCurrentItem( i );
  ensureItemVisibleWithMargin( i );
  mActiveItem = item;
  emit( itemSelected(item) );
}


void KNHeaderView::clear()
{
  mActiveItem = 0;
  QListView::clear();
}


void KNHeaderView::ensureItemVisibleWithMargin( const QListViewItem *i )
{
  if ( !i )
    return;

 QListViewItem *parent = i->parent();
  while ( parent ) {
    if ( !parent->isOpen() )
      parent->setOpen( true );
    parent = parent->parent();
  }

  mDelayedCenter = -1;
  int y = itemPos( i );
  int h = i->height();

  if ( knGlobals.configManager()->readNewsGeneral()->smartScrolling() &&
      ((y + h + 5) >= (contentsY() + visibleHeight()) ||
       (y - 5 < contentsY())) )
  {
    ensureVisible( contentsX(), y + h/2, 0, h/2 );
    mDelayedCenter = y + h/2;
    QTimer::singleShot( 300, this, SLOT(slotCenterDelayed()) );
  } else {
    ensureVisible( contentsX(), y + h/2, 0, h/2 );
  }
}


void KNHeaderView::slotCenterDelayed()
{
  if ( mDelayedCenter != -1 )
    ensureVisible( contentsX(), mDelayedCenter, 0, visibleHeight() / 2 );
}


void KNHeaderView::setSorting( int column, bool ascending )
{
  if ( column == mSortCol ) {
    mSortAsc = ascending;
    if ( column == mPaintInfo.dateCol && ascending )
      mSortByThreadChangeDate = !mSortByThreadChangeDate;
  } else {
    mSortCol = column;
    emit sortingChanged( column );
  }

  KListView::setSorting( column, ascending );

  if ( currentItem() )
    ensureItemVisible( currentItem() );

  if ( mSortByThreadChangeDate )
    setColumnText( mPaintInfo.dateCol , i18n("Date (thread changed)") );
  else
    setColumnText( mPaintInfo.dateCol, i18n("Date") );
}


void KNHeaderView::nextArticle()
{
  KNHdrViewItem *it = static_cast<KNHdrViewItem*>( currentItem() );

  if (it) {
    if (it->isActive()) {  // take current article, if not selected
      if (it->isExpandable())
        it->setOpen(true);
      it = static_cast<KNHdrViewItem*>(it->itemBelow());
    }
  } else
    it = static_cast<KNHdrViewItem*>( firstChild() );

  if(it) {
    clearSelection();
    setActive( it );
    setSelectionAnchor( currentItem() );
  }
}


void KNHeaderView::prevArticle()
{
  KNHdrViewItem *it = static_cast<KNHdrViewItem*>( currentItem() );

  if (it && it->isActive()) {  // take current article, if not selected
    if (it)
      it = static_cast<KNHdrViewItem*>(it->itemAbove());
    else
      it = static_cast<KNHdrViewItem*>( firstChild() );
  }

  if (it) {
    clearSelection();
    setActive( it );
    setSelectionAnchor( currentItem() );
  }
}


void KNHeaderView::incCurrentArticle()
{
  QListViewItem *lvi = currentItem();
  if ( lvi && lvi->isExpandable() )
    lvi->setOpen( true );
  if ( lvi && lvi->itemBelow() ) {
    setCurrentItem( lvi->itemBelow() );
    ensureItemVisible( currentItem() );
    setFocus();
  }
}

void KNHeaderView::decCurrentArticle()
{
  QListViewItem *lvi = currentItem();
  if ( lvi && lvi->itemAbove() ) {
    if ( lvi->itemAbove()->isExpandable() )
      lvi->itemAbove()->setOpen( true );
    setCurrentItem( lvi->itemAbove() );
    ensureItemVisible( currentItem() );
    setFocus();
  }
}


void KNHeaderView::selectCurrentArticle()
{
  clearSelection();
  setActive( currentItem() );
}


bool KNHeaderView::nextUnreadArticle()
{
  if ( !knGlobals.groupManager()->currentGroup() )
    return false;

  KNHdrViewItem *next, *current;
  KNRemoteArticle *art;

  current = static_cast<KNHdrViewItem*>( currentItem() );
  if ( !current )
    current = static_cast<KNHdrViewItem*>( firstChild() );

  if(!current)
    return false;

  art = static_cast<KNRemoteArticle*>( current->art );

  if ( !current->isActive() && !art->isRead() ) // take current article, if unread & not selected
    next = current;
  else {
    if ( current->isExpandable() && art->hasUnreadFollowUps() && !current->isOpen() )
      setOpen( current, true );
    next = static_cast<KNHdrViewItem*>( current->itemBelow() );
  }

  while ( next ) {
    art = static_cast<KNRemoteArticle*>( next->art );
    if ( !art->isRead() )
      break;
    else {
      if ( next->isExpandable() && art->hasUnreadFollowUps() && !next->isOpen() )
        setOpen( next, true );
      next = static_cast<KNHdrViewItem*>( next->itemBelow() );
    }
  }

  if ( next ) {
    clearSelection();
    setActive( next );
    setSelectionAnchor( currentItem() );
    return true;
  }
  return false;
}


bool KNHeaderView::nextUnreadThread()
{
  KNHdrViewItem *next, *current;
  KNRemoteArticle *art;

  if ( !knGlobals.groupManager()->currentGroup() )
    return false;

  current = static_cast<KNHdrViewItem*>( currentItem() );
  if ( !current )
    current = static_cast<KNHdrViewItem*>( firstChild() );

  if ( !current )
    return false;

  art = static_cast<KNRemoteArticle*>( current->art );

  if ( current->depth() == 0 && !current->isActive() && (!art->isRead() || art->hasUnreadFollowUps()) )
    next = current; // take current article, if unread & not selected
  else
    next = static_cast<KNHdrViewItem*>( current->itemBelow() );

  while ( next ) {
    art = static_cast<KNRemoteArticle*>( next->art );

    if ( next->depth() == 0 ) {
      if ( !art->isRead() || art->hasUnreadFollowUps() )
        break;
    }
    next = static_cast<KNHdrViewItem*>( next->itemBelow() );
  }

  if ( next ) {
    setCurrentItem( next );
    if ( art->isRead() )
      nextUnreadArticle();
    else {
      clearSelection();
      setActive( next );
      setSelectionAnchor( currentItem() );
    }
    return true;
  }
  return false;
}


void KNHeaderView::toggleColumn( int column, int mode )
{
  bool *show = 0;
  int  *col  = 0;
  int  width = 0;

  switch ( static_cast<KPaintInfo::ColumnIds>( column ) )
  {
    case KPaintInfo::COL_SIZE:
      show  = &mPaintInfo.showSize;
      col   = &mPaintInfo.sizeCol;
      width = 42;
      break;
    case KPaintInfo::COL_SCORE:
      show  = &mPaintInfo.showScore;
      col   = &mPaintInfo.scoreCol;
      width = 42;
      break;
    default:
      return;
  }

  if ( mode == -1 )
    *show = !*show;
  else
    *show = mode;

  mPopup->setItemChecked( column, *show );

  if (*show) {
    header()->setResizeEnabled( true, *col );
    setColumnWidth( *col, width );
  }
  else {
    header()->setResizeEnabled( false, *col );
    header()->setStretchEnabled( false, *col );
    hideColumn( *col );
  }

  if ( mode == -1 ) // save config when toggled
    writeConfig();
}


void KNHeaderView::prepareForGroup()
{
  mShowingFolder = false;
  header()->setLabel( mPaintInfo.senderCol, i18n("From") );
  KNConfig::ReadNewsGeneral *rngConf = knGlobals.configManager()->readNewsGeneral();
  toggleColumn( KPaintInfo::COL_SCORE, rngConf->showScore() );
}


void KNHeaderView::prepareForFolder()
{
  mShowingFolder = true;
  header()->setLabel( mPaintInfo.senderCol, i18n("Newsgroups / To") );
  toggleColumn( KPaintInfo::COL_SCORE, false ); // local folders have no score
}


bool KNHeaderView::event( QEvent *e )
{
  // we don't want to have the alternate list background restored
  // to the system defaults!
  if (e->type() == QEvent::ApplicationPaletteChange)
    return QListView::event(e);
  else
    return KListView::event(e);
}

void KNHeaderView::contentsMousePressEvent( QMouseEvent *e )
{
  if (!e) return;

  bool selectMode=(( e->state() & ShiftButton ) || ( e->state() & ControlButton ));

  QPoint vp = contentsToViewport(e->pos());
  QListViewItem *i = itemAt(vp);

  KListView::contentsMousePressEvent( e );

  if ( i ) {
    int decoLeft = header()->sectionPos( 0 ) +
        treeStepSize() * ( (i->depth() - 1) + ( rootIsDecorated() ? 1 : 0) );
    int decoRight = kMin( decoLeft + treeStepSize() + itemMargin(),
        header()->sectionPos( 0 ) + header()->sectionSize( 0 ) );
    bool rootDecoClicked = vp.x() > decoLeft && vp.x() < decoRight;

    if( !selectMode && i->isSelected() && !rootDecoClicked )
      setActive( i );
  }
}


void KNHeaderView::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
  if (!e) return;

  QListViewItem *i = itemAt( contentsToViewport(e->pos()) );
  if (i) {
    emit doubleClick( i );
    return;
  }

  KListView::contentsMouseDoubleClickEvent( e );
}


void KNHeaderView::keyPressEvent(QKeyEvent *e)
{
  if (!e) return;

  QListViewItem *i = currentItem();

  switch(e->key()) {
    case Key_Space:
    case Key_Backspace:
    case Key_Delete:
      e->ignore(); // don't eat them
    break;
    case Key_Enter:
    case Key_Return:
      setActive( i );
    break;

    default:
      KListView::keyPressEvent (e);
  }
}


QDragObject* KNHeaderView::dragObject()
{
  KNHdrViewItem *item = static_cast<KNHdrViewItem*>( itemAt(viewport()->mapFromGlobal(QCursor::pos())) );
  if (item)
    return item->dragObject();
  else
    return 0;
}


void KNHeaderView::slotSizeChanged( int section, int, int newSize )
{
  viewport()->repaint( header()->sectionPos(section), 0, newSize, visibleHeight(), false);
}


bool KNHeaderView::eventFilter(QObject *o, QEvent *e)
{
  if ((e->type() == QEvent::KeyPress) && (static_cast<QKeyEvent*>(e)->key() == Key_Tab)) {
    emit(focusChangeRequest(this));
    if (!hasFocus())  // focusChangeRequest was successful
      return true;
  }

  // right click on header
  if ( e->type() == QEvent::MouseButtonPress &&
       static_cast<QMouseEvent*>(e)->button() == RightButton &&
       o->isA("QHeader") )
  {
    mPopup->popup( static_cast<QMouseEvent*>(e)->globalPos() );
    return true;
  }

  return KListView::eventFilter(o, e);
}


void KNHeaderView::focusInEvent(QFocusEvent *e)
{
  QListView::focusInEvent(e);
  emit focusChanged(e);
}


void KNHeaderView::focusOutEvent(QFocusEvent *e)
{
  QListView::focusOutEvent(e);
  emit focusChanged(e);
}


void KNHeaderView::resetCurrentTime()
{
  mDateFormatter.reset();
  QTimer::singleShot( 1000, this, SLOT(resetCurrentTime()) );
}


//BEGIN: KNHeaderViewToolTip ==================================================

KNHeaderViewToolTip::KNHeaderViewToolTip( KNHeaderView *parent ) :
  QToolTip( parent->viewport() ),
  listView( parent )
{
}


void KNHeaderViewToolTip::maybeTip( const QPoint &p )
{
  const KNHdrViewItem *item = static_cast<KNHdrViewItem*>( listView->itemAt( p ) );
  if ( !item )
    return;
  const int column = listView->header()->sectionAt( p.x() );
  if ( column == -1 )
    return;

  if ( !item->showToolTip( column ) )
    return;

  const QRect itemRect = listView->itemRect( item );
  if ( !itemRect.isValid() )
    return;
  const QRect headerRect = listView->header()->sectionRect( column );
  if ( !headerRect.isValid() )
    return;

  tip( QRect( headerRect.left(), itemRect.top(), headerRect.width(), itemRect.height() ), item->text( column ) );
}

//END: KNHeaderViewToolTip ====================================================

#include "headerview.moc"
