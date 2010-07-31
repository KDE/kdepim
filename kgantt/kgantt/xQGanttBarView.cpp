//
//  file    : xQGanttBarView.C
//  date    : 26 oct 2000
//  changed : 28 nov 2000
//  author  : jh
//


#include "xQGanttBarView.h"
#include "KGanttBarConfig.h"



xQGanttBarView::xQGanttBarView(KGanttItem* toplevelitem,
			       TQWidget* parent, 
			       const char * name, WFlags f)
  : TQScrollView(parent,name,f)
{     
  _config = NULL;

  _toplevelitem = toplevelitem;

  setFrameStyle(TQFrame::Sunken);
  setLineWidth(1);

  _headerBackBrush = TQBrush(TQColor(250,250,250));

  setMargins( 1, TOPMARGIN , 1, 1 );

  _viewport = new xQGanttBarViewPort(toplevelitem, this );

  addChild(_viewport);

  _viewport->setMode(xQGanttBarViewPort::Select);

  connect(_viewport, TQT_SIGNAL(scroll(int,int)),
	  this, TQT_SLOT(scrollBy(int,int)) );  

  connect(_viewport, TQT_SIGNAL(recalculated()),
	  this, TQT_SLOT(drawHeader()) );  

  connect(horizontalScrollBar(), TQT_SIGNAL(valueChanged(int)),
	  this, TQT_SLOT(horizontalScrollBarChanged(int)) ); 

}



xQGanttBarView::~xQGanttBarView()
/////////////////////////////////
{
}



void
xQGanttBarView::horizontalScrollBarChanged(int /*x*/)
////////////////////////////////////////////////////
{
  printf("xQGanttBarView::horizontalScrollBarChanged()\n");
  drawHeader();
}



void 
xQGanttBarView::drawHeader()
////////////////////////////////
{
  static TQPen _dotPen( TQColor(35,35,35), 0, DotLine);
  static TQPen _normalPen(TQColor(0,0,0));
  static TQPen _redPen(TQColor(254,0,0));
  static TQPen _greyPen(TQColor(150,150,150));

  TQPainter p(this);
  p.setPen( _normalPen );

  p.fillRect(0,0,width(),TOPMARGIN, _headerBackBrush );

  static int top = 1;
  static int height = 20;
  static int skip = 1;
 
  int a,e,tmp;
  bool drawDays = false;
  double dayWidth = (double) ((_viewport->screenX(144000) - _viewport->screenX(0))/100.);

  int wx = _viewport->worldX(contentsX());

  TQDate startDate = _toplevelitem->getStart().addSecs( wx * 60 ).date();

  wx = _viewport->worldX(contentsX()+width());
  TQDate endDate = _toplevelitem->getStart().addSecs( wx * 60 ).date();
  endDate = endDate.addDays(1);

  int end = (int) startDate.daysTo(endDate);  
  drawDays = (end < 12);

  //  draw week, which first day is not visible

  TQDate t = startDate.addDays(-startDate.dayOfWeek()+1);    
   
  tmp = _toplevelitem->getStart().secsTo(t)/60;
  a = _viewport->screenX(tmp) - contentsX();
  
  p.fillRect(a, top, (int) (5. * dayWidth), height, TQBrush(TQColor(240,240,240))); 
  p.drawRect(a, top, (int) (5. * dayWidth), height );

  //  draw month, which first day is not visible

  t = startDate.addDays(-startDate.day()+1);    
   
  tmp = _toplevelitem->getStart().secsTo(t)/60;
  a = _viewport->screenX(tmp) - contentsX();

  e = t.daysInMonth();
  p.fillRect(a, top + height + skip, (int) (e*dayWidth), height, TQBrush(TQColor(240,240,240))); 
  p.drawRect(a, top + height + skip, (int) (e*dayWidth), height );

  if(a<0) a = 0;
  p.drawText(a+5, int( top + height + skip + (0.8*height) ), 
	     t.shortMonthName(t.month()) + " " + TQString::number(t.year()) );  


  //  draw snapgrid for first month

  tmp = _toplevelitem->getStart().secsTo(startDate)/60;
  a = _viewport->screenX(tmp) - contentsX()+1;
  double dx = (double) 
    ((_viewport->screenX(_viewport->_snapgrid*1000) - _viewport->screenX(0))/1000.);
  double limit = ((1+e-startDate.day()) * 1440)/_viewport->_snapgrid;

  for(double k=1.; k<limit; k++) {	
    p.setPen(_greyPen);
    p.drawLine(a + (int)(k * dx + 0.5), top + skip + 2 * height + 1, 
	       a + (int)(k * dx + 0.5), top + skip + 2 * height + 2);
  }


  //  draw from start to end

  t = startDate;

  for(int i=0; i<end; i++, t = t.addDays(1) ) {          

    tmp = _toplevelitem->getStart().secsTo(t)/60;
    a = _viewport->screenX(tmp) - contentsX();
   
    p.setPen( TQPen(TQColor(black)) );

    if(t.dayOfWeek() == 1) {
	
      p.fillRect(a, top, (int) (5. * dayWidth), height, TQBrush(TQColor(240,240,240))); 
      p.drawRect(a, top, (int) (5. * dayWidth), height );
    
      if(!drawDays)
	p.drawText(a+5, (int) (top + (0.8*height)), TQString::number(t.day()) );      

    }

    if(drawDays) {

      if(a<0) a = 0;

      TQString str = t.shortDayName(t.dayOfWeek()) + " " + TQString::number(t.day());
      TQRect rect = p.boundingRect(a+5, (int)(0.8 * height), 
				  (int) dayWidth, height, AlignLeft, str );

      if(t.dayOfWeek() > 5)
	p.fillRect(rect.x(), rect.y(), rect.width(), -rect.height(), _headerBackBrush );
      else
	p.fillRect(rect.x(), rect.y(), 
		   rect.width(), -rect.height(), TQBrush(TQColor(240,240,240)));

      p.drawText(a+5, (int)(0.8 * height), str );

      if(t.dayOfWeek()>1 && t.dayOfWeek()<6) {
	p.setPen(_dotPen);
	p.drawLine(a, top, a, height);
      }
    
    }

    if(t.day()==1) {

      e = t.daysInMonth();

      p.setPen(_normalPen);

      p.fillRect(a, top + height + skip, 
		 (int) (e * dayWidth), height, TQBrush(TQColor(240,240,240))); 
      p.drawRect(a, top + height + skip, (int) (e * dayWidth), height );
      
      p.drawText(a+5, 
		 top + (int)(1.8 * height) + skip, 
		 t.shortMonthName(t.month()) + " " + TQString::number(t.year()) );  

      //  draw snapgrid

      double limit = (e * 1440)/_viewport->_snapgrid;
      for(double k=1.; k<limit; k++) {	
	p.setPen(_greyPen);
	p.drawLine(a + 1 + (int)(k * dx + 0.5), top + skip + 2 * height + 1, 
		   a + 1 + (int)(k * dx + 0.5), top + skip + 2 * height + 2);
      }
     

    }
    
  } 
  
}



void 
xQGanttBarView::paintEvent(TQPaintEvent * /*e*/)
{      
  drawHeader();
}



KGanttBarConfig* 
xQGanttBarView::getConfig() 
{
  if(_config)
    return _config;
  else {
    _config = new KGanttBarConfig(this);
    return _config;
  }

}



void 
xQGanttBarView::showConfig()
{
  getConfig()->show();
}



void
xQGanttBarView::hideConfig()
{
  if(_config)
    _config->hide();
}
#include "xQGanttBarView.moc"
