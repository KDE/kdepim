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

#include "knlistview.h"
#include "utilities.h"
#include <qheader.h>
#include <qkeycode.h>

KNListView::KNListView(QWidget *parent, const char *name=0)
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
		emit sortingChanged(sCol, col);
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
	if ( !e )	return; // subclass bug

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
