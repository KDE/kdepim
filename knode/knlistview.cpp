/***************************************************************************
                     knlistview.cpp - description
 copyright            : (C) 1999 by Christian Thurner
 email                : cthurner@freepage.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qheader.h>
#include <qpixmap.h>

#include <kiconloader.h>

#include "utilities.h"
#include "knlistview.h"


//bool KNLVItemBase::totalExpand=true;
QPixmap* KNLVItemBase::pms[15];

void KNLVItemBase::initIcons()
{
  for(int i=0; i<16; i++) pms[i]=0;
}



void KNLVItemBase::clearIcons()
{
  for(int i=0; i<16; i++) delete pms[i];  
}



QPixmap& KNLVItemBase::icon(pixmapType t)
{
  if(pms[t]==0) {
    pms[t]=new QPixmap();
    switch(t) {
      case PTgreyBall:        *pms[t]=UserIcon("greyball");     break;
      case PTredBall:         *pms[t]=UserIcon("redball");      break;
      case PTgreyBallChkd:    *pms[t]=UserIcon("greyballchk");  break;
      case PTredBallChkd:     *pms[t]=UserIcon("redballchk");   break;
      case PTnewFups:         *pms[t]=UserIcon("newsubs");      break;
      case PTeyes:            *pms[t]=UserIcon("eyes");         break;
      case PTmail:            *pms[t]=UserIcon("mail");         break;
      case PTposting:         *pms[t]=UserIcon("posting");      break;
      case PTcontrol:         *pms[t]=UserIcon("ctlart");       break;      
      case PTstatusSent:      *pms[t]=UserIcon("stat_sent");    break;
      case PTstatusEdit:      *pms[t]=UserIcon("stat_edit");    break;
      case PTstatusCanceled:  *pms[t]=UserIcon("stat_cncl");    break;
      case PTnntp:            *pms[t]=UserIcon("server");       break;
      case PTgroup:           *pms[t]=UserIcon("group");        break;
      case PTfolder:          *pms[t]=UserIcon("folder");       break;
      case PTnull:            break;
    }
  }
  
  return (*pms[t]);     
}

//==================================================================================

KNLVItemBase::KNLVItemBase(KNLVItemBase *item) : QListViewItem(item)
{
}



KNLVItemBase::KNLVItemBase(KNListView *view) : QListViewItem(view)
{
}



KNLVItemBase::~KNLVItemBase()
{
  if ((isSelected())&&(listView()))
    static_cast<KNListView*>(listView())->selectedRemoved();
}



void KNLVItemBase::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  int xText=0, xPM=3, yPM=0;
  QColor base;
  
  if(isSelected()) {
    QPen pen=p->pen();
    pen.setColor(cg.highlightedText());
    p->setPen(pen);
    base=cg.highlight();
  }
  else {
    if(this->greyOut()) {
      QPen pen=p->pen();
      pen.setColor(cg.dark());
      p->setPen(pen);
    }
    base=cg.base();
  }
      
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
  
  //yText=p->fontMetrics().ascent() + p->fontMetrics().leading()/2;
  p->drawText(xText, 0, width-xText-5, height(), alignment | AlignVCenter,  text(column));
}



int KNLVItemBase::width(const QFontMetrics &fm, const QListView *lv, int column)
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



void KNLVItemBase::sortChildItems(int column, bool a)
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
  : QListView(parent,name), sAsc(true), sCol(-1), exclusiveSelectedItem(0)
{
  connect(header(), SIGNAL(sectionClicked(int)),
    this, SLOT(slotSortList(int)));
    
  header()->setMovingEnabled(false);
  setFrameStyle(NoFrame);
  setSelectionMode(QListView::Multi);
  
  //setAllColumnsShowFocus(true);
  
}


KNListView::~KNListView()
{
}



void KNListView::setSelected(QListViewItem *item, bool select)
{
  if ((select) && (item)) {         // ignore unselect, like in single selection mode
    if (exclusiveSelectedItem)
      QListView::setSelected(exclusiveSelectedItem,false);
    QListView::setSelected(item,true);
    exclusiveSelectedItem = item;
    emit(selectionChanged(item));
  }
}



void KNListView::clear()
{
  exclusiveSelectedItem=0;
  QListView::clear();
}



void KNListView::slotSortList(int col)
{         
  if(col==sCol) sAsc=!sAsc;
  else {
    emit sortingChanged(col);
    sCol=col;
    sAsc=true;
  
  }
      
  this->setSorting(sCol, sAsc);
  
  if(currentItem()!=0) ensureItemVisible(currentItem());  
}


void KNListView::keyPressEvent(QKeyEvent *e)
{
  if ( !e )       return; // subclass bug

  switch(e->key()) {
   case Key_Enter:
   case Key_Return:
     if (currentItem()) setSelected(currentItem(),true);
   break;
   default:
     QListView::keyPressEvent(e);
  }
}


/*void KNListView::keyPressEvent(QKeyEvent *e)
{
  if ( !e ) return; // subclass bug

  switch(e->key()) {
    
    case Key_Enter:
    case Key_Return:
      if (currentItem())
        setSelected(currentItem(),true);
    break;
    
    case Key_Up:
      i = i->itemAbove();
      e->accept();
    break;

    case Key_Right:
      if ( i->isOpen()) i = i->itemBelow();
      else setOpen( i, TRUE );
      e->accept();
    break;

    case Key_Left:
      if ( i->isOpen() ) setOpen( i, FALSE );
      else i = i->itemAbove();
      e->accept();
    break;

    case Key_Next:
      i2 = itemAt(QPoint(0,viewport()->height()-1 ));
      if (i2 == i || !r.isValid() || viewport()->height()<=itemRect(i).bottom()) {
        if (i2)
          i = i2;
        int left = viewport()->height();
        while((i2 = i->itemBelow()) != 0 && left > i2->height() ) {
          left -= i2->height();
          i = i2;
        }
      }
      else {
        if (!i2) {     // list is shorter than the view, goto last item
          while( (i2 = i->itemBelow())!=0)
          i = i2;
        }
        else i = i2;
      }
      e->accept();
    
    break;
  
    case Key_Prior:
      i2 = itemAt( QPoint( 0, 0 ) );
      if ( i == i2 || !r.isValid() || r.top() <= 0 ) {
        if ( i2 ) i = i2;
        int left = viewport()->height();
        while( (i2 = i->itemAbove()) != 0 && left > i2->height() ) {
          left -= i2->height();
          i = i2;
        }
      }
      else i = i2;
      e->accept();
    break;
    
    case Key_Home:  
      i = firstChild();
      e->accept();
    break;  

    case Key_End:           // *Hack* , a direct way to the end of the list would be nicer..
      while( (i2 = i->itemBelow())!=0) i = i2;
      e->accept();
    break;                                                            
  
    default:  
      e->ignore();
    break;    
  }
  
  if ( !i ) return;

  setCurrentItem( i );
  ensureItemVisibleSmooth( i );
  //CG end    
}*/



void KNListView::mouseDoubleClickEvent(QMouseEvent *e)
{
//  QListView::mouseDoubleClickEvent(e);
  QListViewItem *it;
  
  if(e->button()==RightButton || e->button()==LeftButton) {
    it=itemAt(e->pos());
    if(it) emit doubleClicked(it);
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

  if (exclusiveSelectedItem) {
    setCurrentItem(exclusiveSelectedItem);
    ensureItemVisible(exclusiveSelectedItem);
  }

  emit focusChanged(e);
}



//--------------------------------

#include "knlistview.moc"
