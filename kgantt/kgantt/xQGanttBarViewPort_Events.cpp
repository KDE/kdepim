//
//  file    : xQGanttBarViewPort_Events.C
//  date    : 11 nov 2000
//  changed : 27 dec 2000
//  author  : jh
//


#include "xQGanttBarViewPort.h"
#include "xQGanttBarView.h"

#include <math.h>


KGanttItem* xQGanttBarViewPort::_currentItem = NULL;


int _currentMButton = 0;
bool _Mousemoved = FALSE;
bool _selectItem = false;

int _timediff;

bool _changeEnd = false, _changeStart = false;
int oldw = -1, oldx = -1;

QDateTime _tmpStartDateTime, _tmpEndDateTime;


void 
xQGanttBarViewPort::mousePressEvent(QMouseEvent* e)
{
  //  set _currentItem to pushed mousebutton
  _currentMButton = e->button();
  _Mousemoved = false;

  _startPoint->setX( e->x() );
  _startPoint->setY( e->y() );

  _endPoint->setX( e->x() );
  _endPoint->setY( e->y() );

  _itemInfo->hide();
  _itemTextEdit->hide();

  //  right mousebutton & control -> popup menu
  if(e->button() == RightButton && e->state() == ControlButton ) {
    _menu->popup(e->globalPos());
    return;
  }


  /*
   *  get clicked item and position
   */
  _currentItem = NULL;
  Position pos = check(&_currentItem, e->x(), e->y());

  if(!_currentItem) {
    unselectAll();
    return;
  }
  

  /*
   *  edit text
   */
  if(e->button() == MidButton && _mode == Select) {

    xQTaskPosition* tp = _gItemList.find(_currentItem);
    QPainter p(this);

    QRect rect = p.boundingRect(tp->_textPosX, 
				tp->_textPosY, 200, 
				tp->_screenH, AlignLeft, _currentItem->getText() );

    _itemTextEdit->setText(_currentItem->getText());
    _itemTextEdit->move(tp->_textPosX, tp->_screenY + _margin + 1);
    _itemTextEdit->setFixedWidth(rect.width()+40);
    _itemTextEdit->setFixedHeight(tp->_screenH - 2 * _margin - 2);
    _itemTextEdit->setFocus();
    
    // if item is not editable, _itemTextEdit should be not editable
    // as well
    _itemTextEdit->setReadOnly(!_currentItem->isEditable());

    _itemTextEdit->show();

  }


  /*
   *  open/close item, move start, end, item
   */
  if(e->button() == LeftButton && _mode == Select) {

    _timediff = 0;

    switch(pos) {

    case Handle:

      _currentItem->open( !_currentItem->isOpen() );
      break;
      
    case Center:
      
      _changeEnd   = true;
      _changeStart = true;
      
      if(e->state() == ShiftButton) {

	QString tmp; tmp.sprintf("%s\n", _currentItem->getText().latin1() );
	
	tmp += _currentItem->getStart().toString();
	tmp += " - ";
	tmp += _currentItem->getEnd().toString();
	
	_itemInfo->setText( tmp );
	_itemInfo->adjustSize();
	
	_itemInfo->move(e->x() + 25, _gItemList.find(_currentItem)->_screenY - 50 );
	_itemInfo->show();
      }
      else
	_selectItem = true;
      
      break;

    
    case East:

      _changeEnd = true;
      _changeStart = false;
      break;

    
    case West:

      _changeStart = true;
      _changeEnd = false;
      break;

    default :
      break;

    }

  
    
  } // if(e->button() == LeftButton && _mode == Select)
  
}



void
xQGanttBarViewPort::mouseReleaseEvent(QMouseEvent* e)
{
  switch(_mode) {

  case Select: {

    if(_Mousemoved == true) {
      
      _itemInfo->hide();

      if(_changeStart == true || _changeEnd == true) {
      
	if(_changeStart == true) {
	  _currentItem->setStart( _tmpStartDateTime );
	}
	if(_changeEnd == true) {
	  _currentItem->setEnd( _tmpEndDateTime );
	}
	
	oldx = -1; oldw = -1;
	
	recalc();
	QWidget::update();
	
      }
    }
    else {
      if(_currentItem && _selectItem) {


	if(e->state() & ControlButton) {
	  _currentItem->select( !_currentItem->isSelected() );
	}
	else {
	  bool state = _currentItem->isSelected();
	  unselectAll();
	  _currentItem->select( !state );
	}

	QWidget::update();
	_selectItem = false;

      }
    }

    _changeEnd   = false;
    _changeStart = false;

  }
  break;
  

  case Zoom:
    
    if(!_Mousemoved) {
      
      if(e->button() ==  LeftButton)
	zoom(1.4, e->x(), e->y() );
      
      
      if(e->button() ==  RightButton)
	zoom(0.7, e->x(), e->y() );
      

      if(e->button() ==  MidButton)
	zoomAll();

    }
    else {

      if(_currentMButton ==  LeftButton) {

	QPainter p(this);
	QPen pen(DashLine);
	pen.setColor(red);
	p.setRasterOp(XorROP);      
	p.setPen( pen );

	p.drawRect(_startPoint->x(),
		   _startPoint->y(), 
		   _endPoint->x()-_startPoint->x(),
		   _endPoint->y() - _startPoint->y());

	double x1 = _startPoint->x();
	double y1 = _startPoint->y();

	double x2 = _endPoint->x();
	double y2 = _endPoint->y();

	double sys_width  = fabs(x2 - x1);

	double mass = (_parent->visibleWidth()/ sys_width);

	zoom(mass, (int) (x1+(x2-x1)/2), (int) (y1+(y2-y1)/2) );


      }
    }

    break;


  default:
    break;

  }

  _Mousemoved = false;
  _currentMButton = 0;

}



void
xQGanttBarViewPort::mouseMoveEvent(QMouseEvent* e)
{
  if(fabs(_startPoint->x() - e->x()) < 2 &&
     fabs(_startPoint->y() - e->y()) < 2 )
    return;

  static QPen _dashPen(QColor(255,0,0),DashLine);
  static QPen _solidPen(QColor(200,200,200));

  _Mousemoved = true;

  switch(_mode) {
    
  case Select: {
    
    if(_currentMButton == LeftButton && _currentItem) {

      QPainter p(this);
      p.setRasterOp(XorROP);
      // QPen pen(DashLine);
      // pen.setColor(red);
      p.setPen( _dashPen );

      QString stmp;
      stmp.sprintf("%s\n", _currentItem->getText().latin1() );

      int pixeldiff = e->x() - _startPoint->x();
      _timediff = (int) ((double) pixeldiff / _scaleX + 0.5 );
      
      xQTaskPosition* tpos = _gItemList[_currentItem];

      int x = tpos->_screenX; int w = tpos->_screenW;
      
      if(_changeStart && _changeEnd) {
	double tmp = (double) _timediff/(double) _snapgrid;
	_timediff = ((int) (tmp + sgn(tmp) * 0.5)) * _snapgrid;
	stmp += _currentItem->getStart().addSecs(_timediff*60).toString();
	stmp += " - ";
	stmp += _currentItem->getEnd().addSecs(_timediff*60).toString();	
	x += (int) (_timediff * _scaleX);

	_tmpStartDateTime = _currentItem->getStart().addSecs(_timediff*60);
	_tmpEndDateTime = _currentItem->getEnd().addSecs(_timediff*60);

      }
      else {

	if(_changeStart) {

	  QDateTime movedStart( _currentItem->getStart().addSecs(_timediff*60) );

	  _tmpStartDateTime.setDate( movedStart.date() );
	  _tmpStartDateTime.setTime(QTime(0,0,0,0));

	  double diff = _tmpStartDateTime.secsTo(movedStart)/60;

	  double tmp = diff/(double) _snapgrid;
	  _timediff = ((int) (tmp + sgn(tmp) * 0.5)) * _snapgrid;
	  
	  _tmpStartDateTime = _tmpStartDateTime.addSecs(_timediff*60);
	  _timediff = _currentItem->getStart().secsTo(_tmpStartDateTime)/60;

	  stmp += _tmpStartDateTime.toString().latin1();
	  stmp += " - ";
	  stmp += _currentItem->getEnd().toString();

	  x += (int) (_timediff * _scaleX);
	  w -= (int) (_timediff * _scaleX);
	}
	
	if(_changeEnd) {

	  QDateTime movedEnd( _currentItem->getEnd().addSecs(_timediff*60) );

	  _tmpEndDateTime.setDate( movedEnd.date() );
	  _tmpEndDateTime.setTime(QTime(0,0,0,0));

	  double diff = _tmpEndDateTime.secsTo(movedEnd)/60;

	  double tmp = diff/(double) _snapgrid;
	  _timediff = ((int) (tmp + sgn(tmp) * 0.5)) * _snapgrid;

	  _tmpEndDateTime = _tmpEndDateTime.addSecs(_timediff*60);
	  _timediff = _currentItem->getEnd().secsTo(_tmpEndDateTime)/60;

	  stmp += _currentItem->getStart().toString();
	  stmp += " - ";
	  stmp += _tmpEndDateTime.toString().latin1();

	  w += (int) (_timediff * _scaleX);

	}

      }

      _itemInfo->setText( stmp );
      _itemInfo->adjustSize();
      _itemInfo->move(e->x() + 25, _gItemList.find(_currentItem)->_screenY - 50);
      _itemInfo->show();

      if(oldx > 0) {
	p.fillRect(oldx, _gItemList.find(_currentItem)->_screenY, 
		   oldw, _gItemList.find(_currentItem)->_screenH,
		   QBrush(QColor(50,50,50), Dense4Pattern));
	p.drawRect(oldx, _gItemList.find(_currentItem)->_screenY, 
		   oldw, _gItemList.find(_currentItem)->_screenH);

	p.setPen(_solidPen);
	if(_changeStart)
	  p.drawLine(oldx, 0, oldx, height());
	if(oldw > 2)
	  if(_changeEnd)
	    p.drawLine(oldx + oldw, 0, oldx + oldw, height());

      }

      p.setPen(_dashPen);
      p.fillRect(x, _gItemList.find(_currentItem)->_screenY, 
		 w, _gItemList.find(_currentItem)->_screenH,
		 QBrush(QColor(50,50,50), Dense4Pattern) );
      p.drawRect(x, _gItemList.find(_currentItem)->_screenY, 
		 w, _gItemList.find(_currentItem)->_screenH);

      p.setPen(_solidPen);
      if(_changeStart)
	p.drawLine(x, 0, x, height());

      if(w>2)
      if(_changeEnd)
	p.drawLine(x + w, 0, x + w, height());

      oldx = x; oldw = w;

    }
    else {
      
      static Position _pos = Outside;
      
      KGanttItem* item = NULL;
      
      Position pos = check(&item, e->x(), e->y());
      
      if(_pos != pos) {
	
	_pos = pos;
	
	if(pos == West || pos == East) {
	  setCursor( splitHCursor );
	  break;
	}
	if(pos == North || pos == South) {
	  setCursor( splitVCursor );
	  break;
	}
	if(pos == Center) {
	  setCursor( upArrowCursor);
	  break;
	}
	if(pos == Handle) {
	  setCursor(pointingHandCursor);
	  break;
	}
	
	setCursor(arrowCursor);
	
      }
    }
  }
  break;


  case Zoom: {

    if(_currentMButton == LeftButton) {

      static QString strpos;

      strpos = "";

      int s = worldX(_startPoint->x());
      QDateTime d1 = _toplevelitem->getStart().addSecs(s*60);

      s = worldX(e->x());
      QDateTime d2 = _toplevelitem->getStart().addSecs(s*60);

      strpos += d1.date().toString();
      strpos += " - ";
      strpos += d2.date().toString();

      emit message(strpos);

      QPainter p(this);
      QPen pen(DashLine);
      pen.setColor(red);

      p.setRasterOp(XorROP);

      p.setPen( pen );
            
      p.drawRect(_startPoint->x(),
		 _startPoint->y(),
		 _endPoint->x()-_startPoint->x(),
		 _endPoint->y() - _startPoint->y());    

      QBrush _selectedbrush( QColor(50,50,50), Dense4Pattern );

      p.fillRect( _startPoint->x(), _startPoint->y(), 
		  _endPoint->x()-_startPoint->x(), _endPoint->y() - _startPoint->y(),
		  _selectedbrush );

      _endPoint->setX( e->x() );
      _endPoint->setY( e->y() );


      p.drawRect(_startPoint->x(), _startPoint->y(), 
		 _endPoint->x()-_startPoint->x(), _endPoint->y() - _startPoint->y());

      p.fillRect( _startPoint->x(), _startPoint->y(),
		  _endPoint->x()-_startPoint->x(), _endPoint->y() - _startPoint->y(),
		  _selectedbrush );
    }

  }

  break;

  case Move: {
    emit scroll(_startPoint->x() - e->x(), _startPoint->y() - e->y() );
  }
  break;


  default :
    break;

  }
}


void 
xQGanttBarViewPort::keyPressEvent(QKeyEvent* e)
{

  printf("xQGanttBarViewPort::keyPressEvent() key = %d \n", e->key() );

  int dx = 15;
  
  if(e->state() == ControlButton)
    dx *= 10;
  
  switch(e->key()) {
    
  case Key_Left:
    
    emit scroll(-dx,0);
    break;
    
  case Key_Right:
    
    emit scroll(dx,0);
    break;
    
  case Key_Up:
    
    emit scroll(0,-dx);
    break;
    
  case Key_Down:
    
    emit scroll(0, dx);
    break;

  case 43:  // +

    zoom(1.4);
    break;

  case 45: // -

    zoom(0.7);
    break;

  case 4103:  // del

    deleteSelectedItems();
    break;

  case 4102:  // einfg

    insertIntoSelectedItem();
    break;

  case 4119:  // bild v

    emit scroll(0, dx*15);
    break;

  case 4118:  // bild ^

    emit scroll(0,-dx*15);
    break;

  }
  
} 


void 
xQGanttBarViewPort::paintEvent(QPaintEvent * e) 
/////////////////////////////////////////////////
{
  update(e->rect().left(), e->rect().top(),
	 e->rect().right(), e->rect().bottom() );
}
