/***************************************************************************
                          knfocuswidget.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
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

#include "knfocuswidget.h"


KNFocusWidget::KNFocusWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  f_ocus=false;
}



KNFocusWidget::~KNFocusWidget()
{
}



void KNFocusWidget::setWidget(QWidget *w)
{
  w_idget=w;
  connect(w_idget, SIGNAL(focusChanged(QFocusEvent*)),
    this, SLOT(slotFocusChanged(QFocusEvent*)));
}



void KNFocusWidget::resizeEvent(QResizeEvent *)
{
  w_idget->setGeometry(0, 2, width(), height()-2);  
}



void KNFocusWidget::paintEvent(QPaintEvent *)
{
  if(f_ocus) {
    QPainter p(this);
    QPen pen(colorGroup().highlight(),2);
    p.setPen(pen);
    p.drawLine(0,1,width(),1);
  }
}



void KNFocusWidget::slotFocusChanged(QFocusEvent *e)
{
  if(e->gotFocus()) {
    setFocus(true);
    //qDebug("focus in");
  }
  else if(e->lostFocus()) {
    setFocus(false);
    //qDebug("focus out");
  }   
}



//--------------------------------

#include "knfocuswidget.moc"

