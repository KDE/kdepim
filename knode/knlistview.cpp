/*
    knlistview.cpp

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

#include <qcursor.h>
#include <qheader.h>
#include <qtimer.h>
#include <qpainter.h>
#include <klocale.h>

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
  KNConfig::Appearance *app=knGlobals.cfgManager->appearance();

  p->setFont(fontForColumn(column, p->font()));

  QPen pen=p->pen();
  if (isSelected()||a_ctive) {
    pen.setColor(cg.highlightedText());
    if (a_ctive)
      base=app->activeItemColor();
    else
      base=app->selectedItemColor();
  } else {
    if(this->greyOut())
      pen.setColor(greyColor());
    else
      pen.setColor(normalColor());
    base=backgroundColor();
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
       if(pm)
        if(!pm->isNull()) {
          yPM = (height() - pm->height())/2;  
          p->drawPixmap(xPM, yPM, *pm);
          xPM+=pm->width()+3;
        }
    }
        
    xText=xPM;
  }

  if (width-xText-5 > 0) {
    QString t = shortString(text(column),column,width-xText-5,p->fontMetrics());
    p->drawText(xText, 0, width-xText-5, height(), alignment | AlignVCenter,  t);
    if (countUnreadInThread() > 0 && column==0 && !isOpen()) {
      QString t2 = QString("   (%1)").arg(countUnreadInThread());
      QFont orig=p->font();
      QFont font=p->font();
      font.setBold( true );
      p->setFont(font);
      QPen pen=p->pen();
      if (isSelected()||a_ctive) {
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
  
  if(column==0) {
    
    int i=0;
    const QPixmap *pm=pixmap(i);
    while(pm) {
      ret+=pixmap(i)->width()+3;
      i++;
    }
  }

  return ret;
}


void KNLVItemBase::paintFocus(QPainter *p, const QColorGroup & cg, const QRect & r)
{
  p->setPen(QPen(cg.foreground(),1, DotLine));
  p->drawRect(r.x(), r.y(), r.width()-3, r.height());
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
  return knGlobals.cfgManager->appearance()->textColor();
}


QColor KNLVItemBase::greyColor()
{
  return knGlobals.cfgManager->appearance()->textColor();
}


QString KNLVItemBase::shortString(QString text, int, int width, QFontMetrics fm)
{
  QString t(text);
  int ew = fm.width("...");
  if (fm.width(t) > width) {
    for (int i=t.length();i>0;i--)
      if (fm.width(t)+ew > width)
        t.truncate(i);
    t += "...";
  }
  return t;
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
  setFrameStyle(NoFrame);

  setDropVisualizer(false);
  setDropHighlighter(true);
  setItemsRenameable(false);
  setItemsMovable(false);

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

  if (knGlobals.cfgManager->readNewsGeneral()->smartScrolling() &&
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


void KNListView::addAcceptableDropMimetype(const char *mimeType, bool outsideOk)
{
  int oldSize = a_cceptableDropMimetypes.size();
  a_cceptableDropMimetypes.resize(oldSize+1);
  a_cceptOutside.resize(oldSize+1);

  a_cceptableDropMimetypes.at(oldSize) =  mimeType;
  a_cceptOutside.setBit(oldSize, outsideOk);
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

  if ((e->button() == RightButton) && i && (i->isSelected())) {
    emit rightButtonPressed(i, viewport()->mapToGlobal(vp), -1);
    return;
  }

  if ((e->button() == MidButton) && i) {
    emit middleMBClick(i);
    return;
  }

  // hack: block clearSelection()...
  if (i && i->isSelected() && !selectMode &&
      ((vp.x() < header()->sectionPos(0)) ||
       (vp.x() >= ((i->depth()+1)*treeStepSize() + header()->sectionPos(0)))))
    k_eepSelection = true;

  KListView::contentsMousePressEvent(e);
  bool rootDecoClicked = i
           && ( vp.x() <= header()->cellPos( header()->mapToActual( 0 ) ) +
                treeStepSize() * ( i->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() )
           && ( vp.x() >= header()->cellPos( header()->mapToActual( 0 ) ) );

  if(i && !selectMode && i->isSelected() && !rootDecoClicked)
    setActive(i, true);

  k_eepSelection = false;
}


void KNListView::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
  QListView::contentsMouseDoubleClickEvent(e);
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
    case Key_Left:
      if (knGlobals.cfgManager->readNewsNavigation()->emulateKMail())
        emit(keyLeftPressed());
      else
        KListView::keyPressEvent(e);
      break;
    case Key_Right:
      if (knGlobals.cfgManager->readNewsNavigation()->emulateKMail())
        emit(keyRightPressed());
      else
        KListView::keyPressEvent(e);
      break;
    case Key_Up:
      if (knGlobals.cfgManager->readNewsNavigation()->emulateKMail())
        emit(keyUpPressed());
      else
        KListView::keyPressEvent(e);
      break;
    case Key_Down:
      if (knGlobals.cfgManager->readNewsNavigation()->emulateKMail())
        emit(keyDownPressed());
      else
        KListView::keyPressEvent(e);
      break;
    case Key_Next:
      if (knGlobals.cfgManager->readNewsNavigation()->emulateKMail())
        emit(keyNextPressed());
      else
        KListView::keyPressEvent(e);
      break;
    case Key_Prior:
      if (knGlobals.cfgManager->readNewsNavigation()->emulateKMail())
        emit(keyPriorPressed());
      else
        KListView::keyPressEvent(e);
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


bool KNListView::acceptDrag(QDropEvent* event) const
{
  QListViewItem *after=0, *parent=0;
  // why isn't ::findDrop const??? grrr...
  const_cast<KNListView*>(this)->findDrop(event->pos(), parent, after);

  for (uint i=0; i < a_cceptableDropMimetypes.size(); i++) {
    if (event->provides(a_cceptableDropMimetypes[i])) {
      if (after)
        return (static_cast<KNLVItemBase*>(after))->acceptDrag(event);
      else
        return a_cceptOutside[i];
    }
  }
  return false;
}


void KNListView::slotCenterDelayed()
{
  if (d_elayedCenter != -1)
    ensureVisible(contentsX(), d_elayedCenter, 0, visibleHeight()/2);
}



//--------------------------------

#include "knlistview.moc"
