//
//  file    : xQGanttListView.C
//  date    : 23 nov 2000
//  changed : 
//  author  : jh
//

#include "xQGanttListView.h"

#include <tqcolor.h>
#include <klocale.h>

xQGanttListView::xQGanttListView(KGanttItem* toplevelitem, TQWidget* parent, 
				 const char * name, WFlags f)
  : TQScrollView(parent,name,f)
/////////////////////////////////////////////////////////
{ 
  _toplevelitem = toplevelitem;

  setFrameStyle(TQFrame::Sunken);
  setLineWidth(1);

  _headerBackBrush = TQBrush(TQColor(230,230,230));

  setMargins( 1, TOPMARGIN , 1, 1 );
  
  setVScrollBarMode( AlwaysOff );
    
  _viewport = new xQGanttListViewPort(toplevelitem,viewport());
  addChild(_viewport);
 
  viewport()->setBackgroundColor(TQColor(white));

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

  TQPainter p(this);
  p.setPen( TQPen(TQColor(black)) );
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
xQGanttListView::paintEvent(TQPaintEvent * /*e*/)
{      
  drawHeader();
}
#include "xQGanttListView.moc"
