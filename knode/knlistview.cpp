/*
    knlistview.cpp

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
#include <qpainter.h>

#include <kdeversion.h>
#include <klocale.h>
#include <kstringhandler.h>

#include "knglobals.h"
#include "knconfigmanager.h"
#include "knlistview.h"

KNLVItemBase::KNLVItemBase(KNLVItemBase *item)
  : KListViewItem(item), a_ctive(false)
{
}


KNLVItemBase::KNLVItemBase(KNListView *view)
  : KListViewItem(view), a_ctive(false)
{
}


KNLVItemBase::~KNLVItemBase()
{
  if(a_ctive) {
    QListView *lv=listView();
    // no need to check necessary, a KNLVItemBase objects can only be added to KNListViews
    if(lv) (static_cast<KNListView*>(lv))->activeRemoved();
  }
}


void KNLVItemBase::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  int xText=0, xPM=3, yPM=0;
  QColor base;

  QPen pen=p->pen();
  if (isSelected() || a_ctive) {
    pen.setColor(cg.highlightedText());
    base = cg.highlight();
  } else {
    if(this->greyOut())
      pen.setColor(greyColor());
    else
      pen.setColor(normalColor());
#if KDE_IS_VERSION(3,3,90)
    base = backgroundColor( column );
#else
    base = backgroundColor();
#endif
  }

  p->setPen(pen);

  p->fillRect(0,0,width, height(), QBrush(base));

  if(column==0) {
    QFont font=p->font();
    font.setBold( this->firstColBold() );
    p->setFont(font);
    const QPixmap *pm;

    for(int i=0; i<4; i++) {
       pm=pixmap(i);
        if(pm && !pm->isNull()) {
          yPM = (height() - pm->height())/2;
          p->drawPixmap(xPM, yPM, *pm);
          xPM+=pm->width()+3;
        }
    }

    xText=xPM;
  }

  if (width-xText-5 > 0) {
    int cntWidth = 0;
    QString t2;
    QFont f2;
    if (countUnreadInThread() > 0 && column==0 && !isOpen()) {
      t2 = QString("   (%1)").arg(countUnreadInThread());
      f2 = p->font();
      f2.setBold (true);
      cntWidth = QFontMetrics(f2).width(t2, -1);
    }
    QString t = KStringHandler::rPixelSqueeze( text(column), p->fontMetrics(), width - xText - cntWidth - 5 );
    p->drawText(xText, 0, width-xText-5, height(), alignment | AlignVCenter,  t);
    if (cntWidth) {
      QFont orig = p->font();
      p->setFont( f2 );
      QPen pen = p->pen();
      if (isSelected() || a_ctive) {
        pen.setColor(cg.highlightedText());
      } else {
        pen.setColor(cg.link());
      }
      p->setPen(pen);
      p->drawText(xText + QFontMetrics(orig).width(t, -1), 0, width-xText-5, height(), alignment | AlignVCenter,  t2);
    }
  }
}


int KNLVItemBase::width(const QFontMetrics &fm, const QListView *, int column)
{
  int ret = fm.boundingRect( text(column) ).width();

  // all pixmaps are drawn in the first column
  if(column == 0) {
    const QPixmap *pm;
    for(int i = 0; i < 4; ++i) {
      pm = pixmap(i);
      if(pm && !pm->isNull())
        ret += pm->width() + 3;
    }
  }

  return ret;
}


void KNLVItemBase::sortChildItems(int column, bool b)
{
  QListViewItem::sortChildItems(column, b);
}


void KNLVItemBase::expandChildren()
{
  QListViewItemIterator it(firstChild());
  for( ; it.current(); ++it) {
    if(it.current()->depth()<=depth()) break;
    it.current()->setOpen(true);
  }
}


QColor KNLVItemBase::normalColor()
{
  return knGlobals.configManager()->appearance()->textColor();
}


QColor KNLVItemBase::greyColor()
{
  return knGlobals.configManager()->appearance()->textColor();
}


//==============================================================================


KNListView::KNListView(QWidget *parent, const char *name)
  : KListView(parent,name), s_ortAsc(true), s_ortByThreadChangeDate(false), s_ortCol(-1),
    d_elayedCenter(-1), a_ctiveItem(0), k_eepSelection(false)
{
  connect(header(), SIGNAL(clicked(int)),
          this, SLOT(slotSortList(int)));
  disconnect(header(), SIGNAL(sizeChange(int,int,int)));
  connect(header(), SIGNAL(sizeChange(int,int,int)),
          this, SLOT(slotSizeChanged(int,int,int)));

  header()->setMovingEnabled(true);
  header()->setStretchEnabled(true, 0);

  setDropVisualizer(false);
  setDropHighlighter(false);
  setItemsRenameable(false);
  setItemsMovable(false);
  setAcceptDrops( false );
  setDragEnabled( true );
  setAllColumnsShowFocus( true );
  setSelectionMode( QListView::Extended );

  installEventFilter(this);
}


KNListView::~KNListView()
{
}


void KNListView::setActive(QListViewItem *i, bool activate)
{
  KNLVItemBase *item = static_cast<KNLVItemBase*>(i);

  if (!item || (item->isActive() == activate))
    return;

  if (a_ctiveItem) {
    a_ctiveItem->setActive(false);
    repaintItem(a_ctiveItem);
    a_ctiveItem=0;
  }

  item->setActive(activate);

  if (activate) {
    clearSelection();
    setSelected(item,true);
    setCurrentItem(i);
    ensureItemVisibleWithMargin(i);
    a_ctiveItem = item;
    emit(itemSelected(item));
  } else
    repaintItem(item);
}


void KNListView::clear()
{
  a_ctiveItem=0;
  QListView::clear();
}


void KNListView::clearSelection()
{
  if (!k_eepSelection)
    KListView::clearSelection();
}


void KNListView::ensureItemVisibleWithMargin(const QListViewItem *i)
{
  if (!i)
  	return;

 QListViewItem *parent = i->parent();
  while (parent) {
    if (!parent->isOpen())
      parent->setOpen(true);
    parent = parent->parent();
  }

  d_elayedCenter = -1;
  int y = itemPos(i);
  int h = i->height();

  if (knGlobals.configManager()->readNewsGeneral()->smartScrolling() &&
      ((y+h+5) >= (contentsY()+visibleHeight()) ||
       (y-5 < contentsY())))
  {
    ensureVisible(contentsX(), y+h/2, 0, h/2);
    d_elayedCenter = y+h/2;
    QTimer::singleShot(300, this, SLOT(slotCenterDelayed()));
  } else {
    ensureVisible(contentsX(), y+h/2, 0, h/2);
  }
}


void KNListView::slotSortList(int col)
{
  if(col==s_ortCol) {
    s_ortAsc=!s_ortAsc;
    if (col==4 && !s_ortAsc)
      s_ortByThreadChangeDate = !s_ortByThreadChangeDate;
  } else {
    s_ortCol=col;
    emit sortingChanged(col);
  }

  setSorting(col, s_ortAsc);
  if(currentItem()!=0) ensureItemVisible(currentItem());

  if (s_ortByThreadChangeDate)
    setColumnText(4, i18n("Date (thread changed)"));
  else
    setColumnText(4, i18n("Date"));
}


void KNListView::slotSizeChanged(int section, int, int newSize)
{
  viewport()->repaint( header()->sectionPos(section), 0, newSize, visibleHeight(), false);
}

bool KNListView::event( QEvent *e )
{
  // we don't want to have the alternate list background restored
  // to the system defaults!
  if (e->type() == QEvent::ApplicationPaletteChange)
    return QListView::event(e);
  else
    return KListView::event(e);
}

void KNListView::contentsMousePressEvent(QMouseEvent *e)
{
  if (!e) return;

  bool selectMode=(( e->state() & ShiftButton ) || ( e->state() & ControlButton ));

  QPoint vp = contentsToViewport(e->pos());
  QListViewItem *i = itemAt(vp);

  // hack: block clearSelection()...
  if (i && i->isSelected() && !selectMode &&
      ((vp.x() < header()->sectionPos(0)) ||
       (vp.x() >= ((i->depth()+1)*treeStepSize() + header()->sectionPos(0)))))
    k_eepSelection = true;

  KListView::contentsMousePressEvent(e);

  int decoLeft = header()->sectionPos( 0 ) +
      treeStepSize() * ( (i->depth() - 1) + ( rootIsDecorated() ? 1 : 0) );
  int decoRight = kMin( decoLeft + treeStepSize() + itemMargin(),
      header()->sectionPos( 0 ) + header()->sectionSize( 0 ) );
  bool rootDecoClicked = i && vp.x() > decoLeft && vp.x() < decoRight;

  if(i && !selectMode && i->isSelected() && !rootDecoClicked)
    setActive(i, true);

  k_eepSelection = false;
}


void KNListView::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
  if (!e) return;

  QListViewItem *i = itemAt( contentsToViewport(e->pos()) );
  if (i) {
    emit doubleClick(i);
    return;
  }

  KListView::contentsMouseDoubleClickEvent(e);
}


void KNListView::keyPressEvent(QKeyEvent *e)
{
  if (!e) return;

  QListViewItem *i=currentItem();

  switch(e->key()) {
    case Key_Space:
    case Key_Backspace:
    case Key_Delete:
      e->ignore(); // don't eat them
    break;
    case Key_Enter:
    case Key_Return:
      setActive(i, true);
    break;

    default:
      KListView::keyPressEvent (e);
  }
}


bool KNListView::eventFilter(QObject *o, QEvent *e)
{
  if ((e->type() == QEvent::KeyPress) && (static_cast<QKeyEvent*>(e)->key() == Key_Tab)) {
    emit(focusChangeRequest(this));
    if (!hasFocus())  // focusChangeRequest was successful
      return true;
  }
  return KListView::eventFilter(o, e);
}


void KNListView::focusInEvent(QFocusEvent *e)
{
  QListView::focusInEvent(e);
  emit focusChanged(e);
}


void KNListView::focusOutEvent(QFocusEvent *e)
{
  QListView::focusOutEvent(e);
  emit focusChanged(e);
}


QDragObject* KNListView::dragObject()
{
  KNLVItemBase *item = static_cast<KNLVItemBase*>(itemAt(viewport()->mapFromGlobal(QCursor::pos())));
  if (item)
    return item->dragObject();
  else
    return 0;
}


void KNListView::slotCenterDelayed()
{
  if (d_elayedCenter != -1)
    ensureVisible(contentsX(), d_elayedCenter, 0, visibleHeight()/2);
}


void KNListView::reparent(QWidget *parent, WFlags f, const QPoint &p, bool showIt)
{
  KListView::reparent(parent, f, p, showIt);
  emit reparented();
}

//--------------------------------
#include "knlistview.moc"
