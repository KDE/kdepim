/***************************************************************************
                     knlvitembase.cpp - description
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

#include "knlvitembase.h"
#include "utilities.h"
#include <qpixmap.h>
#include <kapp.h>


bool KNLVItemBase::totalExpand=true;
QPixmap* KNLVItemBase::pms[15];

void KNLVItemBase::initIcons()
{
  for(int i=0; i<15; i++) pms[i]=0;
}



void KNLVItemBase::clearIcons()
{
	for(int i=0; i<15; i++) delete pms[i];	
}



QPixmap& KNLVItemBase::icon(pixmapType t)
{
	if(pms[t]==0) {
		pms[t]=new QPixmap();
		switch(t) {
			case PTgreyBall:				*pms[t]=UserIcon("greyball");			break;
			case PTredBall:					*pms[t]=UserIcon("redball");			break;
			case PTgreyBallChkd:	  *pms[t]=UserIcon("greyballchk");	break;
			case PTredBallChkd:			*pms[t]=UserIcon("redballchk");		break;
			case PTnewFups:					*pms[t]=UserIcon("newsubs");			break;
			case PTeyes:						*pms[t]=UserIcon("eyes");					break;
			case PTmail:						*pms[t]=UserIcon("mail");					break;
			case PTposting:					*pms[t]=UserIcon("posting");			break;
			case PTstatusSent:			*pms[t]=UserIcon("stat_sent");			break;
			case PTstatusEdit:			*pms[t]=UserIcon("stat_edit");			break;
			case PTstatusCanceled:	*pms[t]=UserIcon("stat_cncl");			break;
			case PTnntp:            *pms[t]=UserIcon("server");				break;
			case PTgroup:           *pms[t]=UserIcon("group");				break;
			case PTfolder:          *pms[t]=UserIcon("folder");				break;
			case PTnull:						break;
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
	p->drawText(xText, 0, width-xText-5, height(), alignment | AlignVCenter,	text(column));
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



void KNLVItemBase::setOpen(bool o)
{
	QListViewItem *it;
	QListViewItem::setOpen(o);
	
	
	if(o && totalExpand) {
		it=firstChild();
		while(it) {
			if(it->depth()==0) break;
			else {
				it->setOpen(true);
				it=it->nextSibling();
			}
		}
	}		
}



void KNLVItemBase::sortChildItems(int column, bool a)
{
	QListViewItem::sortChildItems(column, true);
}


