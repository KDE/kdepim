/***************************************************************************
                     knlistbox.cpp - description
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

#include <qpainter.h>
#include "knlistbox.h"


KNLBoxItem::KNLBoxItem(const QString& text, void *d=0, QPixmap *_pm=0)
{
	if(_pm) pm=*_pm;
	setText(text);
	mData=d;	
}


KNLBoxItem::~KNLBoxItem()
{
}


void KNLBoxItem::paint( QPainter *p )
{
	
	QFontMetrics fm = p->fontMetrics();
	
	int tYPos=0, tXPos=3, pYPos=0;
	
	tYPos = fm.ascent() + fm.leading()/2; // vertical text position
	
	if(!pm.isNull()) {	
		
		tXPos=pm.width() + 6;	
	
		if ( pm.height() < fm.height() )  {
			//tYPos = fm.ascent() + fm.leading()/2;
			pYPos = (fm.height() - pm.height())/2;}
		else {
			tYPos = pm.height()/2 - fm.height()/2 + fm.ascent();
			pYPos = 0;}
	}
	
	p->drawPixmap( 3, pYPos ,  pm );	
	p->drawText( tXPos, tYPos, text() );
}


int KNLBoxItem::height(const QListBox *lb ) const
{
	return QMAX( pm.height(), lb->fontMetrics().lineSpacing() + 1 );	
}


int KNLBoxItem::width(const QListBox *lb ) const
{
	return pm.width() + lb->fontMetrics().width( text() ) + 6;
}


//==============================================================================


KNListBox::KNListBox(QWidget *parent, const char *name) : QListBox(parent,name)
{
}

KNListBox::~KNListBox()
{
}