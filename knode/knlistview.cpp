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

#include <qheader.h>
#include <qpixmap.h>

#include <kapp.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "knglobals.h"
#include "knconfigmanager.h"
#include "utilities.h"
#include "knlistview.h"
#include "knmime.h"
#include "knhdrviewitem.h"
#include "kndnd.h"



KNLVItemBase::KNLVItemBase(KNLVItemBase *item)
  : QListViewItem(item), a_ctive(false), h_over(false)
{
}


KNLVItemBase::KNLVItemBase(KNListView *view)
  : QListViewItem(view), a_ctive(false), h_over(false)
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
    base=cg.base();
  }

  p->setPen(pen);
      
  p->fillRect(0,0,width, height(), QBrush(base));
  
  if(column==0) {
    QFont font=p->font();
    font.setBold( this->firstColBold() );
    font.setUnderline(h_over);
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
    if(it.current()->depth()==depth()) break;
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


KNListView::KNListView(QWidget *parent, const char *name, KNDragHandler *dh)
  : QListView(parent,name), s_ortAsc(true), s_ortCol(-1), a_ctiveItem(0), d_handler(dh),
    d_ragMousePressed(false), d_ragHoverItem(0), d_ropError(QString::null)
{
  connect(header(), SIGNAL(clicked(int)),
          this, SLOT(slotSortList(int)));
  disconnect(header(), SIGNAL(sizeChange(int,int,int)));
  connect(header(), SIGNAL(sizeChange(int,int,int)),
          this, SLOT(slotSizeChanged(int,int,int)));

  header()->setMovingEnabled(true);
  setFrameStyle(NoFrame);
  setSelectionMode(QListView::Extended);

  if(dh)
    dh->setWidget(this);
}


KNListView::~KNListView()
{
  delete d_handler;
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


void KNListView::triggerDropError(const QString &e)
{
  /* show a message-box with a little delay, so
     the drop-operation can be completed first */
  d_ropError=e;
  QTimer::singleShot(50, this, SLOT(slotShowDropError()));
}


void KNListView::slotSortList(int col)
{
  if(col==s_ortCol) s_ortAsc=!s_ortAsc;
  else {
    emit sortingChanged(col);
    s_ortCol=col;
  }

  setSorting(col, s_ortAsc);
  
  if(currentItem()!=0) ensureItemVisible(currentItem());  
}


void KNListView::slotSizeChanged(int section, int, int newSize)
{
  viewport()->repaint( header()->sectionPos(section), 0, newSize, visibleHeight(), false);
}


void KNListView::contentsMousePressEvent(QMouseEvent *e)
{
  if (!e) return;

  bool selectMode=(
                    ( ( e->state() & ShiftButton ) || ( e->state() & ControlButton ) ) &&
                    selectionMode()!=Single
                  );

  QPoint vp = contentsToViewport( e->pos() );
  QListViewItem * i = itemAt( vp );

  if ( (e->button() == RightButton) && i && (i->isSelected()) ) {
    emit rightButtonPressed( i, viewport()->mapToGlobal(vp), -1 );
    return;
  }

  if ((e->button() == MidButton) && i)
    emit middleMBClick(i);

  // select item ?
  if(!i || !i->isSelected() || selectMode) {
    QListView::contentsMousePressEvent(e);
    i=currentItem();
    if(i && !selectMode && i->isSelected())
      setActive(i, true);
  }
  // open item ?
  else if(i) {
  if( vp.x() >= header()->sectionPos(0) &&
          vp.x() < ((i->depth()+1)*treeStepSize() + header()->sectionPos(0))
    )
    QListView::contentsMousePressEvent(e);
  }

  if(i && !selectMode && i->isSelected() && e->button()==LeftButton) {
    d_ragStartPos=e->pos();
    d_ragMousePressed=true;
  }
}


void KNListView::contentsMouseMoveEvent(QMouseEvent *e)
{
  if( d_handler && d_ragMousePressed && (e->pos() - d_ragStartPos).manhattanLength() > 8 ) {
    d_ragMousePressed=false;
     kdDebug(5003) << "KNListView::contentsMouseMoveEvent() : start drag" << endl;
    d_handler->startDrag(currentItem());
  }
}


void KNListView::contentsMouseReleaseEvent(QMouseEvent *e)
{
  //kdDebug(5003) << "KNListView::contentsMouseReleaseEvent()" << endl;
  if(e->button()==LeftButton)
    d_ragMousePressed=false;

  bool selectMode=(
                    ( ( e->state() & ShiftButton ) || ( e->state() & ControlButton ) ) &&
                    selectionMode()!=Single
                  );

  QPoint vp = contentsToViewport( e->pos() );
  QListViewItem * i = itemAt( vp );

  if(i && !selectMode && i->isSelected())
    setActive(i, true);

  QListView::contentsMouseReleaseEvent(e);
}


void KNListView::contentsDragEnterEvent(QDragEnterEvent *e)
{
  kdDebug(5003) << "KNListView::contentsDragEnterEvent()" << endl;
  if(!e) return;

  if(d_handler && d_handler->accept(e)) {
    e->accept(rect());
  }
  else {
    e->ignore(rect());
  }
}


void KNListView::contentsDragMoveEvent(QDragMoveEvent *e)
{
  if(!e) return;
  QPoint vp = contentsToViewport( e->pos() );
  QListViewItem *i=itemAt(vp);
  //kdDebug(5003) << "KNListView::contentsDragMoveEvent() : type = " << e->format(0) << endl;

  if(d_ragHoverItem!=i && d_ragHoverItem) {
    d_ragHoverItem->hover(false);
    repaintItem(d_ragHoverItem);
    d_ragHoverItem=0;
  }


  if( d_handler && d_handler->accept(e, i) ) {
    if(i && d_ragHoverItem!=i) {
      d_ragHoverItem=static_cast<KNLVItemBase*>(i);
      d_ragHoverItem->hover(true);
      repaintItem(d_ragHoverItem);
    }
    e->accept(QRect(0,0,0,0));
  }
  else {
    d_ragHoverItem=0;
    e->ignore(QRect(0,0,0,0));
  }
}


void KNListView::contentsDragLeaveEvent(QDragLeaveEvent *)
{
  if(d_ragHoverItem) {
    d_ragHoverItem->hover(false);
    repaintItem(d_ragHoverItem);
  }
  d_ragHoverItem=0;

  d_ragMousePressed=false;
}


void KNListView::contentsDropEvent(QDropEvent *e)
{
  if(!e) return;
  QPoint vp = contentsToViewport( e->pos() );
  QListViewItem *i=itemAt(vp);

  kdDebug(5003) << "KNListView::contentsDropEvent() : type = " << e->format(0) << endl;

  if( d_handler && d_handler->accept(e, i) )
    e->acceptAction(true);
  else
    e->ignore();

  if(d_ragHoverItem) {
    d_ragHoverItem->hover(false);
    repaintItem(d_ragHoverItem);
  }
  d_ragHoverItem=0;

  if(e->isAccepted()) {
    kapp->processEvents();
    emit( dropReceived(e->format(0), i) );
  }

  d_ragMousePressed=false;
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
      ensureItemVisible(i);
    break;

    default:
      QListView::keyPressEvent (e);
  }
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


void KNListView::slotShowDropError()
{
  if(!d_ropError.isEmpty()) {
    KMessageBox::error(knGlobals.topWidget, d_ropError);
    d_ropError=QString::null;
  }
}



//--------------------------------

#include "knlistview.moc"
