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


KNListBoxItem::KNListBoxItem(const QString& text, QPixmap *pm)
{
  p_m=pm;
  setText(text);
}


KNListBoxItem::~KNListBoxItem()
{
}


void KNListBoxItem::paint(QPainter *p)
{
  
  QFontMetrics fm = p->fontMetrics();
  
  int tYPos=0, tXPos=3, pYPos=0;
  
  tYPos = fm.ascent() + fm.leading()/2; // vertical text position
  
  if(p_m) {
    
    tXPos=p_m->width() + 6;
  
    if ( p_m->height() < fm.height() )  {
      //tYPos = fm.ascent() + fm.leading()/2;
      pYPos = (fm.height() - p_m->height())/2;}
    else {
      tYPos = p_m->height()/2 - fm.height()/2 + fm.ascent();
      pYPos = 0;
    }
    p->drawPixmap( 3, pYPos ,  *p_m );
  }
  

  p->drawText( tXPos, tYPos, text() );
}


int KNListBoxItem::height(const QListBox *lb) const
{
  if(p_m)
    return QMAX( p_m->height(), lb->fontMetrics().lineSpacing() + 1 );
  else
    return (lb->fontMetrics().lineSpacing() + 1);
}


int KNListBoxItem::width(const QListBox *lb) const
{
  if(p_m)
    return (p_m->width() + lb->fontMetrics().width( text() ) + 6);
  else
    return (lb->fontMetrics().width( text() ) + 6);
}
