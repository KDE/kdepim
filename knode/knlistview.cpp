/*
    knlistview.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
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

#include "knglobals.h"
#include "knconfigmanager.h"
#include "utilities.h"
#include "knlistview.h"
#include "knmime.h"
#include "knhdrviewitem.h"



KNLVItemBase::KNLVItemBase(KNLVItemBase *item) : QListViewItem(item), active(false)
{
}


KNLVItemBase::KNLVItemBase(KNListView *view) : QListViewItem(view), active(false)
{
}


KNLVItemBase::~KNLVItemBase()
{
  if(active) {
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

  QPen pen=p->pen();
  if (isSelected()||active) {
    pen.setColor(cg.highlightedText());
    if (active)
      base=app->activeItemColor();
    else
      base=app->selectedItemColor();
  } else {
    if (this->greyOut())
      pen.setColor(greyColor());
    else
      pen.setColor(normalColor());
    base=cg.base();
  }
  p->setPen(pen);
      
  p->fillRect(0,0,width, height(), QBrush(base));
  
  if(column==0) {
    
    if(this->firstColBold()) {
      QFont font=p->font();
      font.setBold(true); 
      p->setFont(font);
    }
  
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


KNListView::KNListView(QWidget *parent, const char *name)
  : QListView(parent,name), sAsc(true), sCol(-1), activeItem(0)
{
  connect(header(), SIGNAL(sectionClicked(int)),
          this, SLOT(slotSortList(int)));
  disconnect(header(), SIGNAL(sizeChange(int,int,int)));
  connect(header(), SIGNAL(sizeChange(int,int,int)),
          this, SLOT(slotSizeChanged(int,int,int)));

  header()->setMovingEnabled(false);
  setFrameStyle(NoFrame);
  setSelectionMode(QListView::Extended);
}


KNListView::~KNListView()
{
}


void KNListView::setActive(QListViewItem *i, bool activate)
{
  KNLVItemBase *item = static_cast<KNLVItemBase*>(i);

  if (!item || (item->isActive() == activate))
  	return;
  	
  if (activeItem) {
    activeItem->setActive(false);
    repaintItem(activeItem);
    activeItem=0;
  }

  item->setActive(activate);

  if (activate) {
    clearSelection();
    setSelected(item,true);
    activeItem = item;
    emit(itemSelected(item));
  } else
    repaintItem(item);
}
	

void KNListView::clear()
{
  activeItem=0;
  QListView::clear();
}


void KNListView::slotSortList(int col)
{         
  if(col==sCol) sAsc=!sAsc;
  else {
    emit sortingChanged(col);
    sCol=col;
  }
      
  setSorting(sCol, sAsc);
  
  if(currentItem()!=0) ensureItemVisible(currentItem());  
}


void KNListView::slotSizeChanged(int section, int, int newSize)
{
  viewport()->repaint( header()->sectionPos(section), 0, newSize, visibleHeight(), false);
}


void KNListView::contentsMousePressEvent(QMouseEvent *e)
{
  if (!e) return;

  bool selectMode=(( e->state() & ShiftButton ) || ( e->state() & ControlButton ));

  QListView::contentsMousePressEvent(e);
  QListViewItem *i=currentItem();
  if (i && !selectMode && i->isSelected())
    setActive(i, true);
}



void KNListView::keyPressEvent(QKeyEvent *e)
{
  if (!e) return;

  QListViewItem *i=currentItem();

  switch(e->key()) {
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


//--------------------------------

#include "knlistview.moc"
