//
//  file    : xQGanttListView.C
//  date    : 23 nov 2000
//  changed : 
//  author  : jh
//

#include "xQGanttListView.h"

#include <qcolor.h>
#include <klocale.h>

xQGanttListView::xQGanttListView(KGanttItem* toplevelitem, QWidget* parent, 
				 const char * name, WFlags f)
  : QScrollView(parent,name,f)
/////////////////////////////////////////////////////////
{ 
  _toplevelitem = toplevelitem;

  setFrameStyle(QFrame::Sunken);
  setLineWidth(1);

  _headerBackBrush = QBrush(QColor(230,230,230));

  setMargins( 1, TOPMARGIN , 1, 1 );
  
  setVScrollBarMode( AlwaysOff );
    
  _viewport = new xQGanttListViewPort(toplevelitem,viewport());
  addChild(_viewport);
 
  viewport()->setBackgroundColor(QColor(white));

}



xQGanttListView::~xQGanttListView()
///////////////////////////////////
{
}


void
xQGanttListView::drawHeader()
///////////////////////////////
{
  // printf("xQGanttListView::drawHeader()\n");

  QPainter p(this);
  p.setPen( QPen(QColor(black)) );
  p.fillRect(0,0,width(),TOPMARGIN, _headerBackBrush );

  p.drawText(5, (int)(0.8 * TOPMARGIN), i18n("Items"));

}



void
xQGanttListView::contentsMoved(int x, int y)
////////////////////////////////////////////
{
  printf("xQGanttListView::contentsMoved(%d,%d)\n", x, y);
  setContentsPos( 0, y );
}



void 
xQGanttListView::paintEvent(QPaintEvent * /*e*/)
{      
  drawHeader();
}
#include "xQGanttListView.moc"
