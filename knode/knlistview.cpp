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



KNLVItemBase::KNLVItemBase(KNLVItemBase *item) : QListViewItem(item)
{
}


KNLVItemBase::KNLVItemBase(KNListView *view) : QListViewItem(view)
{
}


KNLVItemBase::~KNLVItemBase()
{
  if(isSelected()) {
    QListView *lv=listView();
    if(lv && lv->isA("KNListView"))
      (static_cast<KNListView*>(lv))->selectedRemoved();
  }
}


void KNLVItemBase::setSelected(bool select)
{
  QListViewItem::setSelected(select);
  QListView *lv=listView();
  if(lv && lv->isA("KNListView"))
    (static_cast<KNListView*>(lv))->itemToggled(this, select);
}


void KNLVItemBase::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  int xText=0, xPM=3, yPM=0;
  QColor base;
  KNConfig::Appearance *app=knGlobals.cfgManager->appearance();

  QPen pen=p->pen();
  if (isSelected()) {
    pen.setColor(cg.highlightedText());
    base=cg.highlight();
  } else {
    if (this->greyOut())
      pen.setColor(app->readArticleColor());
    else
      pen.setColor(app->textColor());
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
    // making the string shorter when the column is to narrow
    QFontMetrics fm( p->fontMetrics() );
    QString t(text(column));
    int ew = fm.width("...");
    if (fm.width(t) > width-xText-5) {
      for (int i=t.length();i>0;i--)
        if (fm.width(t)+ew > width-xText-5)
          t.truncate(i);
      t += "...";
    }

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
  QListView* lv = listView();
  if ((lv)&&(lv->hasFocus())) {
    p->setPen(QPen(cg.foreground(),1, DotLine));
    p->drawRect(r.x(), r.y(), r.width()-3, r.height());
  }
}


void KNLVItemBase::sortChildItems(int column, bool)
{
  QListViewItem::sortChildItems(column, true);
}


void KNLVItemBase::expandChildren()
{
  QListViewItemIterator it(firstChild());
  
  for( ; it.current(); ++it) {
    if(it.current()->depth()==depth()) break;
    it.current()->setOpen(true);
  }
}


//==============================================================================


KNListView::KNListView(QWidget *parent, const char *name)
  : QListView(parent,name), sAsc(true), sCol(-1), s_elCount(0)
{
  connect(header(), SIGNAL(sectionClicked(int)),
    this, SLOT(slotSortList(int)));
    
  header()->setMovingEnabled(false);
  setFrameStyle(NoFrame);
  setSelectionMode(QListView::Extended);
}


KNListView::~KNListView()
{
}


/*void KNListView::setSelected(QListViewItem *i, bool select)
{
  kdDebug(5003) << "KNListView::setSelected(QListViewItem *i, bool select)" << endl;
  QListView::setSelected(i, select);
  if(i && select)
    emit( itemSelected(i) );
}*/


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


void KNListView::itemToggled(QListViewItem *i, bool selected)
{
  if(selected) {
    if(++s_elCount==1)
      emit( itemSelected(i) );
  }
  else
    s_elCount--;
}


void KNListView::keyPressEvent(QKeyEvent *e)
{
  if ( !e )       return; // subclass bug

  /*if (e->state() & ShiftButton) {  // lame workaround to avoid multiselection in multiselection mode ;-)
    e->ignore();
    return;
  }*/
  QListViewItem *i=currentItem();
  switch(e->key()) {

    case Key_PageDown:
      verticalScrollBar()->addPage();
    break;

    case Key_PageUp:
      verticalScrollBar()->subtractPage();
    break;

    case Key_Up:
      if(i)
        i=i->itemAbove();
      if(i) {
        if(e->state() & ShiftButton)
          setSelected(i, true);
        setCurrentItem(i);
        ensureItemVisible(i);
      }
    break;

    case Key_Down:
      if(i)
        i=i->itemBelow();
      if(i) {
        if(e->state() & ShiftButton)
          setSelected(i, true);
        setCurrentItem(i);
        ensureItemVisible(i);
      }
    break;

    case Key_Right:
      if(i && !i->isOpen() && i->isExpandable())
        i->setOpen(true);
    break;

    case Key_Left:
      if(i && i->isOpen())
        i->setOpen(false);
    break;

    case Key_Enter:
    case Key_Return:
      if( !(e->state() & ControlButton) )
        clearSelection();
      setSelected(i, true);
    break;

    case Key_Escape:
      clearSelection();
    break;

    default:
      e->ignore();
  }
}


void KNListView::focusInEvent(QFocusEvent *e)
{
  if ( currentItem() ) repaintItem(currentItem());   // show cursor marker

  emit focusChanged(e);
}


void KNListView::focusOutEvent(QFocusEvent *e)
{
  if ( currentItem() ) repaintItem(currentItem());   // hide cursor marker

  /*if (exclusiveSelectedItem) {
    setCurrentItem(exclusiveSelectedItem);
    ensureItemVisible(exclusiveSelectedItem);
  } */

  emit focusChanged(e);
}


//--------------------------------

#include "knlistview.moc"
