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

#include <kapp.h>
#include <kiconloader.h>

#include "knglobals.h"
#include "knappmanager.h"
#include "utilities.h"
#include "knlistview.h"


QPixmap* KNLVItemBase::pms[15];
QColor KNLVItemBase::normal, KNLVItemBase::grey;


void KNLVItemBase::updateAppearance()
{
  if (knGlobals.appManager->useColors()) {
    normal = knGlobals.appManager->color(KNAppManager::normalText);
    grey = knGlobals.appManager->color(KNAppManager::readArticle);
  } else {
    normal = kapp->palette().active().text();
    grey = kapp->palette().disabled().text();
  }
}



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

  QPen pen=p->pen();
  if (isSelected()) {
    pen.setColor(cg.highlightedText());
    base=cg.highlight();
  } else {
    if (this->greyOut())
      pen.setColor(grey);
    else
      pen.setColor(normal);
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
  }
      
  setSorting(sCol, sAsc);
  
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
