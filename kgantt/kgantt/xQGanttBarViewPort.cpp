//
//  file    : xQGanttBarViewPort.C
//  date    : 26 oct 2000
//  changed : 09 jan 2001
//  author  : jh
//

#include "xQGanttBarViewPort.h"
#include "xQGanttBarView.h"

#include <qcolor.h>
#include <qtoolbutton.h>

#include "lupe.xpm"
#include "open.xpm"
#include "close.xpm"

#include <ktoolbarbutton.h> 
#include <kiconloader.h>
#include <klocale.h> 
  


xQGanttBarViewPort::xQGanttBarViewPort(KGanttItem* toplevelitem, 
				       xQGanttBarView* parent,
				       const char * name, WFlags f)
  : QFrame(parent,name,f)
/////////////////////////////////////////////////////////////////////////////
{
  _parent = parent;

  closedIcon =  QPixmap(open_xpm);
  openedIcon =  QPixmap(close_xpm);

  _observedList = NULL;
  _toolbar = NULL;

  _gItemList = QPtrDict<xQTaskPosition>(449);
  _gItemList.setAutoDelete(true);

  _toplevelitem = toplevelitem;

  _itemInfo = new QLabel(this);
  _itemInfo->setBackgroundColor(QColor(235,235,255));
  _itemInfo->setFrameStyle( Panel | Sunken ); 
  _itemInfo->setMargin( 5 );
  _itemInfo->setLineWidth(1);
  _itemInfo->hide();
 
  
  _itemTextEdit = new QLineEdit(this);
  _itemTextEdit->hide();
  _itemTextEdit->setFrame(false);
  
  connect(_itemTextEdit, SIGNAL(returnPressed ()),
	  this, SLOT(textEdited()));

  _iconloader = new KIconLoader();

  initMenu();

  setBackgroundColor(QColor(white));        
  /*
    QPixmap back("background.png");
    setBackgroundPixmap ( back );
  */

  _grid = 1440;
  _snapgrid = 360;

  _drawGrid = true;
  _drawHeader = false;
  
  _marginX = 10 * 1440;
  _marginY = 50;

  _scaleX = 0.1;
  _scaleY = 1;

  _margin = 4; // margin item in pixel

  _startPoint = new QPoint();  _endPoint = new QPoint();

  _cursor_lupe = new QCursor( QPixmap(lupe) );
 
  connect(_toplevelitem, SIGNAL(changed(KGanttItem*, KGanttItem::Change)),
	  this, SLOT(toplevelitemChanged(KGanttItem*, KGanttItem::Change)) );

  recalc(); adjustSize();

  setFocusPolicy(QWidget::StrongFocus);
  _mode = Init;

}



xQGanttBarViewPort::~xQGanttBarViewPort()
/////////////////////////////////////////
{
}



KToolBar*
xQGanttBarViewPort::toolbar(QMainWindow* mw)
{
  if(_toolbar || mw == 0) return _toolbar;

  _toolbar = new KToolBar(mw,QMainWindow::DockTop);

  mw->addToolBar(_toolbar);

   
  // KIconLoader* iconloader = new KIconLoader("kgantt");
 

  _toolbar->insertButton("ganttSelect.png", 0, 
			 SIGNAL(clicked()),
			 this, SLOT(setSelect()),
			 true, i18n("Select") );

  KPopupMenu *selectMenu = new KPopupMenu(_toolbar);


  /*
    select all items
  */
  QPixmap pix = _iconloader->loadIcon("ganttSelecttask.png", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("ganttSelecttask.png not found !\n");
  selectMenu->insertItem(pix, i18n("Select All"), this, SLOT(selectAll()) );  


  /*
    unselect all items
  */
  pix = _iconloader->loadIcon("ganttUnselecttask", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("ganttUnselecttask.png not found !\n");
  selectMenu->insertItem(pix, i18n("Unselect All"), this, SLOT(unselectAll()) );


  KToolBarButton* b = _toolbar->getButton(0);
  b->setDelayedPopup(selectMenu);


  _toolbar->insertButton("viewmag.png", 1, 
			 SIGNAL(clicked()),
			 this, SLOT(setZoom()),
			 true, i18n("Zoom") );
  
  KPopupMenu* zoomMenu = new KPopupMenu(_toolbar);
 
  pix = _iconloader->loadIcon("viewmag.png", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("viewmag.png not found !\n");
  zoomMenu->insertItem(pix, i18n("Zoom All"), this, SLOT(zoomAll()) );
  zoomMenu->insertSeparator();

  pix = _iconloader->loadIcon("viewmag+.png", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("viewmag+.png not found !\n");
  zoomMenu->insertItem(pix, i18n("Zoom In +"), this, SLOT(zoomIn()) );

  pix = _iconloader->loadIcon("viewmag-.png", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("viewmag-.png not found !\n");
  zoomMenu->insertItem(pix, i18n("Zoom Out -"), this, SLOT(zoomOut()) );
  
  b = _toolbar->getButton(1);
  b->setDelayedPopup(zoomMenu);

  _toolbar->insertButton("move.png", 2, 
			  SIGNAL(clicked()),
			  this, SLOT(setMove()),
			  true, i18n("Move") );

   return _toolbar;

}



void
xQGanttBarViewPort::initMenu()
/////////////////////////////////
{
  _menu = new KPopupMenu(this);

  /*
      select
  */

  _selectMenu = new KPopupMenu(_menu);

  QPixmap pix = _iconloader->loadIcon("ganttSelect.png", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("ganttSelect.png not found !\n");
  _selectMenu->insertItem(pix, i18n("Select Mode"), this, SLOT(setSelect()));

  _selectMenu->insertSeparator();

  pix = _iconloader->loadIcon("ganttSelecttask.png", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("ganttSelecttask.png not found !\n");
  _selectMenu->insertItem(pix, i18n("Select All"), this, SLOT(selectAll()) );  

  pix = _iconloader->loadIcon("ganttUnselecttask", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("ganttUnselecttask.png not found !\n");
  _selectMenu->insertItem(pix, i18n("Unselect All"), this, SLOT(unselectAll()) );
  
  _menu->insertItem( i18n("Select"), _selectMenu);


  /*
      zoom
  */

  KPopupMenu* _zoomMenu = new KPopupMenu(_menu);

  pix = _iconloader->loadIcon("viewmag.png", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("viewmag.png not found !\n");
  _zoomMenu->insertItem(i18n("Zoom Mode"), this, SLOT(setZoom()) );

  _zoomMenu->insertSeparator();

  _zoomMenu->insertItem(pix, i18n("Zoom All"), this, SLOT(zoomAll()) );
  _zoomMenu->insertSeparator();

  pix = _iconloader->loadIcon("viewmag+.png", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("viewmag+.png not found !\n");
  _zoomMenu->insertItem(pix, i18n("Zoom In +"), this, SLOT(zoomIn()) );

  pix = _iconloader->loadIcon("viewmag-.png", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("viewmag-.png not found !\n");
  _zoomMenu->insertItem(pix, i18n("Zoom Out -"), this, SLOT(zoomOut()) );

  _menu->insertItem( "Zoom", _zoomMenu);

  pix = _iconloader->loadIcon("move.png", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("move.png not found !\n");
  _menu->insertItem(pix, i18n("Move Mode"), this, SLOT(setMove()) );

  _menu->insertSeparator();

  pix = _iconloader->loadIcon("configure.png", KIcon::Toolbar , 16 );
  if(pix.isNull()) printf("configure.png not found !\n");
  _menu->insertItem(pix, i18n("Configure Gantt..."), _parent, SLOT(showConfig()));

}



void
xQGanttBarViewPort::toplevelitemChanged(KGanttItem* /*item*/, KGanttItem::Change /*c*/)
///////////////////////////////////////////////////////////////////
{
  recalc();
  adjustSize();
}



void 
xQGanttBarViewPort::adjustSize()
//////////////////////////////////
{
  // printf("xQGanttBarViewPort::adjustSize()\n");

  static int sw = 0;
  static int sh = 0;

  int w = screenX(_toplevelitem->getWidth() + _marginX);
  int h = screenY(_toplevelitem->getTotalHeight() + _marginY);

  if(sw != w || sh !=h) {

    sw = w;
    sh = h;

    resize(w,h);

    emit resized();

  }

}



void 
xQGanttBarViewPort::update(int x1, int y1, int x2, int y2)
//////////////////////////////////////////////////////////
{
  QPainter p(this);

  // QTime time1 = QTime::currentTime();
  
  if(_drawGrid)
    drawGrid(&p, x1, y1, x2, y2);

  // QTime time2 = QTime::currentTime();  
  // printf("%d msec for drawing grid.\n", time1.msecsTo( time2 ) );

  // drawContents(&p, x1, y1, x2, y2);
  drawItem(_toplevelitem, &p, QRect(x1, y1, x2-x1, y2-y1) );

  // time1 = QTime::currentTime();  
  // printf("%d msec for drawing contents.\n", time2.msecsTo( time1 ) );
  
  if(_drawHeader)
    drawHeader(&p, x1, y1, x2, y2);
  
  // time2 = QTime::currentTime();<  
  // printf("%d msec for drawing header.\n", time1.msecsTo( time2 ) );
  
}



void
xQGanttBarViewPort::drawGrid(QPainter* p, int x1, int y1, int x2, int y2)
////////////////////////////////////////////////////////////////
{
  y2 += 5; // avoid white lines at bottom of redrawn region

  static int a, w, end, tmp;
  static QBrush _sat( QColor(200,200,200));
  static QBrush _sun( QColor(255,110,110));
  static QBrush _hol( QColor(200,200,250));
  static QPen penDay( QColor(235,235,235), 0, DotLine);
  static QPen penMonth( QColor(0,150,0), 3, DashDotLine);
  static QPen penHour( QColor(0,0,150), 0, DashDotLine);

  QDate start( _toplevelitem->getStart().addSecs(worldX(x1)*60).date() );

  end = (int) ((x2-x1)/(1440.*_scaleX))+1;
  w = (int) (1440. * _scaleX + 0.5);

  //  draw holydays

  QDate* ptrDate;
  QDate cmp(start.addDays(-1));

  for(ptrDate = _holidays.first(); ptrDate != 0; ptrDate = _holidays.next() ) {
    if(*ptrDate > cmp) {
      tmp = _toplevelitem->getStart().secsTo(*ptrDate)/60;
      a = screenX( tmp );
      p->fillRect( a, y1, w, y2, _hol );
    }
    
  }

  //  draw grid
 
  for(int i=0; i<=end; i++, start = start.addDays(1) ) {

    int dayOfWeek = start.dayOfWeek();
    tmp = _toplevelitem->getStart().secsTo(start)/60;
    a = screenX( tmp );
   
    //  draw saturday
    if(dayOfWeek == 6) {

      p->fillRect( a, y1, w, y2, _sat );
      
      if(start.day() == 1) {
	p->setPen( penMonth );
	p->drawLine( a, y1, a, y2);
      }

      // continue;
    }

    //  sunday
    if(dayOfWeek == 7) {

      p->fillRect( a, y1, w, y2, _sun );

      if(start.day() == 1) {
	p->setPen( penMonth );
	p->drawLine( a, y1, a, y2);
      }

      // continue;
    }

    if(start.day() == 1) 
      p->setPen( penMonth );
    else {
      if(dayOfWeek == 1 || dayOfWeek == 6 || dayOfWeek == 7) 
	continue;
      p->setPen( penDay );
    }

    p->drawLine( a, y1, a, y2);

  }
}



void
xQGanttBarViewPort::recalc()
{
  // printf("xQGanttBarViewPort::recalc()\n");  
  _gItemList.clear();
  recalc(_toplevelitem, screenX(0), screenY(0), 0, 0 );
  emit recalculated();
}



void 
xQGanttBarViewPort::recalc(KGanttItem* item, int xPos, int yPos, 
			   int depth, int nr)
{
  int tmpTotalHeight = item->getTotalHeight();
  int tmpHeight      = item->getHeight();

  int dd = (int) (0.25 * (double) tmpHeight * _scaleY);

  int _screenW  = (int) ((double) item->getWidth() * _scaleX);
  int _screenHS = (int) ((double) tmpTotalHeight * _scaleY);
  int _screenH  = (int) (tmpHeight * _scaleY);
  int _textPosY = yPos + (int) (0.7 * (double) tmpHeight * _scaleY);
  int _textPosX = xPos + dd + 18;

  xQTaskPosition* tpos =  
    new xQTaskPosition(nr, xPos, yPos, _screenW, _screenH, _screenHS,
		       _textPosX, _textPosY, depth);
  
  _gItemList.replace(item, tpos );

  tpos->_screenHandleX = xPos + dd;
  tpos->_screenHandleW = 2 * dd;
  tpos->_screenHandleY = yPos + dd;
  tpos->_screenHandleH = 2 * dd;
  

  //  recalc subitems
  
  if(item->isOpen()) {

    int h = tmpHeight;

    for(KGanttItem* subitem = item->getSubItems().first(); 
	subitem != 0; 
	subitem = item->getSubItems().next() ) {
      
      recalc(subitem, 
	     xPos + (int)(item->getStart().secsTo(subitem->getStart())/60 * _scaleX), 
	     yPos + (int)( h * _scaleY ), depth + 1, ++nr );

      h += subitem->getTotalHeight();
      
    }
  }         
 
}



void
xQGanttBarViewPort::drawItem(KGanttItem* item, QPainter* p, 
			     const QRect& rect )
{
  xQTaskPosition* tpos = _gItemList[item];

  if(!tpos) return;

  if(tpos->_screenX > (rect.x() + rect.width())) return;
  if((tpos->_screenX + tpos->_screenW) < rect.x()) return;
  if(tpos->_screenY > (rect.y() + rect.height()) ) return; 
  if((tpos->_screenY + tpos->_screenHS) < rect.y()) return;

  p->setPen(item->getPen());
  p->setBrush(item->getBrush());

  int style = item->getStyle();

  if(item->getWidth()==0) {

    p->drawLine(tpos->_screenX, tpos->_screenY,
		tpos->_screenX, tpos->_screenY + tpos->_screenH );

    QPointArray a(4);
    a.setPoint(0, tpos->_screenX, tpos->_screenY + _margin );
    a.setPoint(1, tpos->_screenX - tpos->_screenH / 2 + _margin, 
	          tpos->_screenY + tpos->_screenH / 2 );
    a.setPoint(2, tpos->_screenX, tpos->_screenY + tpos->_screenH - _margin );
    a.setPoint(3, tpos->_screenX + tpos->_screenH / 2 - _margin, 
	          tpos->_screenY + tpos->_screenH / 2 );
    p->drawPolygon(a);

  }
  else {

    if(style & KGanttItem::DrawFilled ) {
      
      p->fillRect(tpos->_screenX, tpos->_screenY + _margin,
		  tpos->_screenW, tpos->_screenHS - 2 * _margin,
		  item->getBrush() );
      
    }
    
    if(style & KGanttItem::DrawBorder ) {
      
      p->setBrush(NoBrush);
      p->drawRect(tpos->_screenX, tpos->_screenY + _margin,
		  tpos->_screenW, tpos->_screenHS - 2 * _margin );
      
    }
    
    if(item->isOpen()) {

      //  draw relations
      for(KGanttRelation* rel = item->getRelations().first(); 
	  rel != 0; 
	  rel = item->getRelations().next() ) {

	drawRelation(p, rel);

      }

      //  draw subitems
      for(KGanttItem* subitem = item->getSubItems().first(); 
	  subitem != 0; 
	  subitem = item->getSubItems().next() ) {
	
	drawItem(subitem, p, rect );
	
      }
    }      
    
    p->setPen(item->getPen());
    p->setBrush(item->getBrush());

    if(style & KGanttItem::DrawHandle || 
       ((style & KGanttItem::DrawHandleWSubitems) && item->getSubItems().count()>0) ) {

      /*
	p->setBrush(QColor("steelblue"));
	p->drawRect(tpos->_screenHandleX, tpos->_screenHandleY, 
	tpos->_screenHandleW, tpos->_screenHandleH);
      */
      if(item->isOpen())
	p->drawPixmap(tpos->_screenHandleX, tpos->_screenHandleY, openedIcon );
      else
	p->drawPixmap(tpos->_screenHandleX, tpos->_screenHandleY, closedIcon );
      
    }
  }
  
  if(style & KGanttItem::DrawText ) {
    p->setPen(item->getTextPen());
    p->drawText(tpos->_textPosX, tpos->_textPosY, item->getText() );
  }

  if(item->isSelected()) {

    p->setPen( QPen(QColor(red),1));

    p->setBrush(NoBrush);
    p->drawRect(tpos->_screenX - 2, tpos->_screenY,
		tpos->_screenW + 4, tpos->_screenHS );

    p->fillRect(tpos->_screenX, tpos->_screenY, 6, 6,
		item->getSelectBrush() );

    p->fillRect(tpos->_screenX + tpos->_screenW - 6, 
		tpos->_screenY, 6, 6,
		item->getSelectBrush() );

    p->fillRect(tpos->_screenX + tpos->_screenW - 6, 
		tpos->_screenY + tpos->_screenHS - 6, 6, 6,
		item->getSelectBrush() );

    p->fillRect(tpos->_screenX, 
		tpos->_screenY + tpos->_screenHS - 6, 6, 6,
		item->getSelectBrush() );
  }

}



void
xQGanttBarViewPort::drawRelation(QPainter* p,
				 KGanttRelation* rel)
{
  static int hw = 20;
  static int margin = 2;

  KGanttItem* from = rel->getFrom();
  KGanttItem* to = rel->getTo();

  xQTaskPosition* tpos_from = _gItemList[from];
  xQTaskPosition* tpos_to = _gItemList[to];

  p->setPen(rel->getPen());

  QPointArray a(6);

  int x,y;
  int i=0;

  // 1
  x = tpos_from->_screenX + tpos_from->_screenW + margin;
  y = tpos_from->_screenY + tpos_from->_screenH / 2;
  a.setPoint(i++, x, y ); 


  // 2
  x = x + hw;
  a.setPoint(i++, x, y);


  // 3   
  y = (int)( (tpos_from->_screenY + tpos_from->_screenH/2) * 0.8 +  
    (tpos_to->_screenY + tpos_to->_screenH/2) * 0.2 );
  a.setPoint(i++, x, y);


  // 4
  x = tpos_to->_screenX - hw;
  y = (int)( (tpos_from->_screenY + tpos_from->_screenH/2) * 0.2 +  
    (tpos_to->_screenY + tpos_to->_screenH/2) * 0.8 );

  a.setPoint(i++, x, y);


  // 5
  y = tpos_to->_screenY + tpos_to->_screenH / 2;
  a.setPoint(i++, x, y);


  // 6
  x = tpos_to->_screenX - margin;
  a.setPoint(i++, x, y);

  p->drawPolyline(a);

  p->drawChord( a.point(0).x()-3, a.point(0).y()-3, 6, 6, 0, 5760 );


  QPointArray b(3);

  b.setPoint(0, x,y);
  b.setPoint(1, x -5, y - 5);
  b.setPoint(2, x - 5, y + 5);

  p->drawPolygon(b);

}



void 
xQGanttBarViewPort::drawHeader(QPainter* p, int /*x1*/, int /*y1*/, int /*x2*/, int /*y2*/)
//////////////////////////////////////////////////////////////////////////
{
  bool drawDays = false;
  int a,e,tmp;

  QDate start( _toplevelitem->getStart().addSecs(-_marginX * 60 ).date() );
  
  // subtract 1 month to draw first month
  QDate t(start.year(), start.month()-1, start.day() ); 

  QDateTime itemstart = _toplevelitem->getStart();

  int end = (int) (width()/(1440*_scaleX));

  if(end < 12) drawDays = true;

  end += 30; // add 30 days to draw last month

  p->setPen( QPen(QColor(black)) );

  for(int i=0; i<=end; i++, t = t.addDays(1) ) {

    tmp = itemstart.secsTo(t)/60;
    a = screenX( tmp );

    if(t.dayOfWeek() == 1) {
	
      p->fillRect(a, 0, (int)( 1440*5*_scaleX ), 20, QBrush(QColor(240,240,240)));
      p->drawRect(a, 0, (int)( 1440*5*_scaleX ), 20 );

      if(!drawDays)
	p->drawText(a+5, 15, QString::number(t.day()) );
    }

    if(drawDays) {
      p->drawText(a+5, 15, t.shortDayName(t.dayOfWeek()) + " " + QString::number(t.day()) );
    }

    if(t.day()==1) {

      e = t.daysInMonth();

      p->fillRect(a, 21, (int)( 1440*e*_scaleX ), 20, QBrush(QColor(240,240,240))); 
      p->drawRect(a, 21, (int)( 1440*e*_scaleX ), 20 );

      if(a<0) a = 0;
      p->drawText(a+5, 36, t.shortMonthName(t.month()) );        

    }

  } 
}



void
xQGanttBarViewPort::setMode(int mode) 
/////////////////////////////
{
  if(_mode == (Mode) mode) {
    return;
  }

  switch(_mode) {

  case Select:

    setSelect();
    break;


  case Zoom:
    
    setZoom();
    break;


  case Move:

    setMove();
    break;


  default:

    setCursor(arrowCursor);
    setMouseTracking(false);
    break;

  }

  emit modeChanged(_mode);

}



void
xQGanttBarViewPort::setSelect()
////////////////////////////////
{
  _mode = Select;
  setCursor(arrowCursor);
  setMouseTracking(true);
}



void
xQGanttBarViewPort::setZoom()
/////////////////////////////
{
  _mode = Zoom;
  setCursor( *_cursor_lupe );
  setMouseTracking(false); 
}



void
xQGanttBarViewPort::setMove()
//////////////////////////////
{
  _mode = Move;
  setCursor( sizeAllCursor );
  setMouseTracking(false);
}


void
xQGanttBarViewPort::zoomIn()
{
  zoom(1.2);
}


void
xQGanttBarViewPort::zoomOut()
{
  zoom(0.7);
}


void 
xQGanttBarViewPort::popup(int index)
///////////////////////////////////
{

  switch(index) {

  case Select:
  case Zoom:
  case Move:

    setMode(index);
    break;

  case 10: // configure
      
    // setConfigDialog();
    // _config->show();

    break;

  }


}



void
xQGanttBarViewPort::zoom(double sfactor, int sx, int sy)
///////////////////////////////////////////////////////
{
  printf("zoom %f, (%d,%d) \n", sfactor, sx, sy );

  int wx = worldX(sx);
  int wy = worldY(sy);

  _scaleX *= sfactor;

  printf("recalc ... \n");

  recalc();
  adjustSize();

  _parent->center(screenX(wx), screenY(wy) );

  QWidget::update();

  printf("zoom ok.\n");

}



void
xQGanttBarViewPort::zoom(double sfactor)
{
  printf("zoom %f \n", sfactor );

  int x = (int) (_parent->visibleWidth()/2 + 0.5);
  int y = (int) (_parent->visibleHeight()/2 + 0.5);

  printf("dx/2 = %d, dy/2 = %d \n", x,y);

  zoom(sfactor, x + _parent->contentsX(), y + _parent->contentsY() );

}



void 
xQGanttBarViewPort::zoomAll()
{  
#ifdef _DEBUG_
  printf("zoom all. scaleX = %f\n", _scaleX );
#endif

  _scaleX = ((double) _parent->visibleWidth()*60)/
    ((double) (_toplevelitem->getStart().secsTo(_toplevelitem->getEnd()) + _marginX*120));

  recalc();
  adjustSize();

}



void 
xQGanttBarViewPort::addHoliday(int y, int m, int d) 
{
  QDate* date = new QDate(y,m,d);
  
  QDate* ptrDate; 
  int i=0;
  
  for(ptrDate = _holidays.first(); 
      ptrDate != 0; 
      ptrDate = _holidays.next() ) {
    
    if(*ptrDate > *date)	
      break;      
    
    i++;
    
  }
  
  _holidays.insert(i,date);

}



xQGanttBarViewPort::Position 
xQGanttBarViewPort::check(KGanttItem** founditem, int x, int y)
{
  QPtrDictIterator<xQTaskPosition> it(_gItemList); 

  static int ty, ty2, tx, tx2, hx, hx2, hy, hy2;
  bool increased;

  while(it.current()) {    

    ty  = it.current()->_screenY;
    ty2 = ty + it.current()->_screenH;
    tx  = it.current()->_screenX;
    tx2 = tx + it.current()->_screenW;

    hx  = it.current()->_screenHandleX;
    hx2 = hx + it.current()->_screenHandleW;
    hy  = it.current()->_screenHandleY;
    hy2 = hy + it.current()->_screenHandleH;
    
    increased = false;

    if(tx2-tx<12) {
      tx -= 12;
      tx2 += 12;
      increased = true;
    }

    if(x>tx && x < tx2) {
      if(y > ty && y < ty2) {

	*founditem = (KGanttItem*) it.currentKey();

	if(!increased)
	  if(x > hx && x < hx2 &&
	     y > hy && y < hy2 )
	    return Handle;

	 if(x < (tx + 5)) 
	   return West;

	 if(x > (tx2 - 5))
	   return East;

	 return Center;
      }
            
    }

    ++it;

  }

  return Outside;

}



void
xQGanttBarViewPort::unselectAll()
{
  selectItem(_toplevelitem, false);
  QWidget::update();
}



void
xQGanttBarViewPort::selectAll()
{
  selectItem(_toplevelitem, true);
  QWidget::update();
}



void
xQGanttBarViewPort::selectItem(KGanttItem* item, bool f)
{
  item->select(f);

  for(KGanttItem* subitem = item->getSubItems().first(); 
      subitem != 0; 
      subitem = item->getSubItems().next() ) {
    selectItem(subitem, f);
  }

}



void 
xQGanttBarViewPort::deleteSelectedItems()
{
#ifdef _DEBUG_
  printf("-> xQGanttBarViewPort::deleteSelectedItems()\n");
#endif

  QPtrList<KGanttItem> list;
  observeList(&list);
  
  getSelectedItems(_toplevelitem,list);

  for(KGanttItem* subitem = list.first(); 
      subitem != 0; 
      subitem =list.next() ) {
#ifdef _DEBUG_
    printf(" : %s \n", subitem->getText().latin1() );
#endif
    connect(subitem, SIGNAL(destroyed(KGanttItem*)),
	    this, SLOT(itemDestroyed(KGanttItem*)));
  }

  list.remove(_toplevelitem);

  while(list.count()>0) {
    KGanttItem* item = list.getFirst();
    delete item;
  }

#ifdef _DEBUG_
  printf("<- xQGanttBarViewPort::deleteSelectedItems()\n");
#endif
}



void
xQGanttBarViewPort::observeList(QPtrList<KGanttItem> *list)
{
  _observedList = list;
}



void 
xQGanttBarViewPort::itemDestroyed(KGanttItem* item)
{
  _observedList->remove(item);
}



void
xQGanttBarViewPort::getSelectedItems (KGanttItem* item, 
				      QPtrList<KGanttItem>& list)
{
  if(item->isSelected()) list.append(item);

  for(KGanttItem* subitem = item->getSubItems().first(); 
      subitem != 0; 
      subitem = item->getSubItems().next() ) {
    
    getSelectedItems(subitem,list);
    
  }
  
}


void 
xQGanttBarViewPort::insertIntoSelectedItem()
{
  QPtrList<KGanttItem> list;

  getSelectedItems(_toplevelitem,list);

  for(KGanttItem* subitem = list.first(); 
      subitem != 0; 
      subitem =list.next() ) {
#ifdef _DEBUG_
    printf(" : %s \n", subitem->getText().latin1() );
#endif 
    new KGanttItem(subitem, subitem->getText() + "_subitem",
	       subitem->getStart(), subitem->getEnd());
  }

}



void
xQGanttBarViewPort::textEdited()
{
  if(_currentItem) {
    _currentItem->setText(_itemTextEdit->text());
    setFocus();
    _itemTextEdit->hide();
  }
}
#include "xQGanttBarViewPort.moc"
