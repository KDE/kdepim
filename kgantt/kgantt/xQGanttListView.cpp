//
//  file    : xQGanttListView.C
//  date    : 23 nov 2000
//  changed : 
//  author  : jh
//

#include "xQGanttListView.h"

#include <qcolor.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QPaintEvent>
#include <klocale.h>

xQGanttListView::xQGanttListView(KGanttItem* toplevelitem, QWidget* parent, 
				 const char * name, Qt::WFlags f)
  : Q3ScrollView(parent,name,f)
/////////////////////////////////////////////////////////
{ 
  _toplevelitem = toplevelitem;

  setFrameStyle(Q3Frame::Sunken);
  setLineWidth(1);

  _headerBackBrush = QBrush(QColor(230,230,230));

  setMargins( 1, TOPMARGIN , 1, 1 );
  
  setVScrollBarMode( AlwaysOff );
    
  _viewport = new xQGanttListViewPort(toplevelitem,viewport());
  addChild(_viewport);
 
  viewport()->setBackgroundColor(QColor(Qt::white));

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
  p.setPen( QPen(QColor(Qt::black)) );
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
