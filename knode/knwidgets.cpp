/*
    knwidgets.cpp

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

#include <qpainter.h>
#include <qpixmap.h>

#include "knwidgets.h"


//====================================================================================

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


//====================================================================================

// **** listbox for dialogs **************************************************

KNDialogListBox::KNDialogListBox(bool alwaysIgnore, QWidget * parent, const char * name)
 : QListBox(parent, name), a_lwaysIgnore(alwaysIgnore)
{
}


KNDialogListBox::~KNDialogListBox()
{
}


void KNDialogListBox::keyPressEvent(QKeyEvent *e)
{
  if ((a_lwaysIgnore || !(hasFocus()&&isVisible()))&&((e->key()==Key_Enter)||(e->key()==Key_Return)))
    e->ignore();
  else
    QListBox::keyPressEvent(e);
}


//====================================================================================


KNProgress::KNProgress (int desiredHeight, int maxValue, int value, QWidget *parent, const char *name)
 : KProgress(parent, name), desHeight(desiredHeight)
{
  setTotalSteps(maxValue);
  setValue(value);
  setFixedWidth(110);
  setFrameStyle(QFrame::Box);
  setLineWidth(1);
  setBackgroundMode(QWidget::PaletteBackground);
  disableProgressBar();
}


KNProgress::~KNProgress()
{}


// 0% and no text
void KNProgress::disableProgressBar()
{
  setFormat(QString::null);
  setValue(0);
  repaint(false);
}


// manual operation
void KNProgress::setProgressBar(int value,const QString& text)
{
  setFormat(text);
  if (value>1000) {
    setValue(1000);
    update();       // circumvent the optimization of setValue
  } else
    setValue(value);
}


QSize KNProgress::sizeHint() const
{
  return QSize(KProgress::sizeHint().width(),desHeight);
}


//====================================================================================


KNDockWidgetHeaderDrag::KNDockWidgetHeaderDrag(QWidget *focusWidget, KDockWidgetAbstractHeader* parent, KDockWidget* dock, const char* name )
  : KDockWidgetHeaderDrag(parent, dock, name), f_ocus(false)
{
  connect(focusWidget, SIGNAL(focusChanged(QFocusEvent*)), SLOT(slotFocusChanged(QFocusEvent*)));
}


KNDockWidgetHeaderDrag::~KNDockWidgetHeaderDrag()
{
}


void KNDockWidgetHeaderDrag::slotFocusChanged(QFocusEvent *e)
{
  if(e->gotFocus()) {
    f_ocus = true;
  } else if(e->lostFocus()) {
    f_ocus = false;
  }
  update();
}


void KNDockWidgetHeaderDrag::paintEvent(QPaintEvent* ev)
{
  if (!f_ocus) {
    KDockWidgetHeaderDrag::paintEvent(ev);
    return;
  }

  QPixmap drawBuffer(width(), height());
  QPainter paint;

  paint.begin(&drawBuffer);
  paint.fillRect(drawBuffer.rect(), QBrush(colorGroup().brush(QColorGroup::Background)));

  paint.setPen(palette().active().highlight());
  paint.drawLine(1, 2, width(), 2);
  paint.drawLine(1, 3, width(), 3);
  paint.drawLine(1, 5, width(), 5);
  paint.drawLine(1, 6, width(), 6);

  bitBlt( this,0,0,&drawBuffer,0,0,width(),height());
  paint.end();
}


//====================================================================================

#include "knwidgets.moc"
