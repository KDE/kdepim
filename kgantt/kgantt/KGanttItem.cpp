//
//  file    : KGanttItem.C
//  date    : 26 oct 2000
//  changed : 11 jan 2001
//  author  : jh
//


#include "KGanttItem.h"


QBrush KGanttItem::_selectBrush(QColor(255,0,0));


KGanttItem::KGanttItem(KGanttItem* parentItem, const QString& text, 
	 const QDateTime& start, const QDateTime& end)
  : QObject()
////////////////////////////////////////////////////////
{
  init(parentItem,text, start,end);
}



KGanttItem::KGanttItem(KGanttItem* parentItem, const QString& text, 
	 const QDateTime& start, long durationMin)
  : QObject()
////////////////////////////////////////////////////////
{  
  init(parentItem, text, start, start.addSecs( durationMin * 60));
}



void 
KGanttItem::init(KGanttItem* parentItem, const QString& text,
	     const QDateTime& start, const QDateTime& end) 
///////////////////////////////////////////////////////////////
{
  _style = DrawAll - DrawHandle;
  _open = true;
  _selected = false;
  _editable = true;

  _mode = Normal;

  _brush   = QBrush(QColor(140,140,255));
  _pen     = QPen(QColor(100,100,100));
  _textPen = QPen(QColor(black));

  _height = 24;

  _text = text;

  _start = start; _minDateTime = start;
  _end = end; _maxDateTime = end;

  _parentItem = parentItem;

  if(_parentItem)
    _parentItem->registerItem(this);
  
}



KGanttItem::~KGanttItem()
/////////////////
{
#ifdef _DEBUG_
  printf("-> delete %s \n", getText().latin1() );
#endif

  if(_parentItem)
    _parentItem->unregisterItem(this);

  _subitems.setAutoDelete(true);
  _subitems.clear();

  emit destroyed(this);

#ifdef _DEBUG_
  printf("<- delete %s \n", getText().latin1() );
#endif
}



KGanttRelation* 
KGanttItem::addRelation(KGanttItem* from, KGanttItem* to,
			const QString& text)
{
  if(_subitems.containsRef(from) > 0 && _subitems.containsRef(to) >0) {
    KGanttRelation* rel = new KGanttRelation(from,to,text);
    _relations.append(rel);

    connect(rel, SIGNAL(destroyed(KGanttRelation*)),
	    this, SLOT(removeRelation(KGanttRelation*)));

    emit changed(this, RelationAdded);
    return rel;
  }	   
  else
    return NULL;
}



void
KGanttItem::removeRelation(KGanttRelation* rel)
{
  if( _relations.removeRef(rel) )
    emit changed(this, RelationRemoved);
}



void 
KGanttItem::endTransaction()
///////////////////////////
{
  blockSignals(false);
  emit changed(this, Unknown);
}



void 
KGanttItem::registerItem(KGanttItem* item)
{
  _subitems.append(item);
  
  connect(item, SIGNAL(changed(KGanttItem*, KGanttItem::Change)),
	  this, SLOT(subItemChanged(KGanttItem*, KGanttItem::Change)) );

  bool minChanged = false;
  bool maxChanged = false;

  // update min/man

  if(_subitems.count() == 1) {

    _minDateTime = item->getStart();
    _maxDateTime = item->getEnd();
    
    minChanged = true;
    maxChanged = true;

  }
  else {

    if(item->getEnd() > _maxDateTime) {
      _maxDateTime = item->getEnd();
      maxChanged = true;
    }
    
    if(_minDateTime > item->getStart()) {
      _minDateTime = item->getStart();
      minChanged = true;
    }
    
  } // else


  //  increase start/end if necessary
  Change change = adjustStartEnd();

  if(_mode == Rubberband) {
    if(minChanged && !(change & StartChanged))
      change = (Change) (change + StartChanged);
    if(maxChanged && !(change & EndChanged))
      change = (Change) (change + EndChanged);
  }

  if( isOpen() ) {
    if(!(change & TotalHeightChanged))
      change = (Change) (change + TotalHeightChanged);
  }

  if(change != NoChange)
    emit changed(this,change);

}



void 
KGanttItem::unregisterItem(KGanttItem* item)
{
  _subitems.removeRef(item);
  disconnect(item);

  Change change = adjustMinMax();

  if( isOpen() ) {
    if(!(change & TotalHeightChanged))
      change = (Change) (change + TotalHeightChanged);
  }

  if(change != NoChange)
    emit changed(this,change);

}



QDateTime 
KGanttItem::getStart()
{ 
  if(_mode == Rubberband && _subitems.count()>0)
    return _minDateTime;
  else
    return _start; 
}




QDateTime 
KGanttItem::getEnd()
/////////////////
{
  if(_mode == Rubberband && _subitems.count()>0)
    return _maxDateTime;
  else
    return _end; 
}



void 
KGanttItem::setStart(const QDateTime& start) 
{
  if(!_editable) return;

  //  if there are no subitems, just set _start and _minDateTime
  if(_subitems.count()==0) {

    if(_start != start) {
      _start = start;
      _minDateTime = _start;
      emit changed(this,StartChanged);
    }
    
  }
  else {

    //  if there are subitems, just change start if
    //  mode is not 'rubberband' and start is less than _minDateTime

    if(_mode != Rubberband) {

      if(start < _minDateTime)
	_start = start;
      else
	_start = _minDateTime;
    
      emit changed(this,StartChanged);

    }
      
  }
  
}



void 
KGanttItem::setEnd(const QDateTime& end) 
{ 
  if(!_editable) return;

  //  if there are no subitems, just set _end and _maxDateTime
  if(_subitems.count()==0) {

    if(_end != end) {
      _end = end;
      _maxDateTime = _end;
      emit changed(this,EndChanged);
    }

  }
  else {

    //  if there are subitems, just change end if
    //  mode is not 'rubberband' and end is greater than _maxDateTime

    if(_mode != Rubberband) {

      if(end > _maxDateTime)
	_end = end;
      else
	_end = _maxDateTime;

      emit changed(this,EndChanged);

    }
    
  }
  
}




KGanttItem::Change
KGanttItem::adjustStartEnd()
//////////////////////////
{
  //  first update _min and _max of subitems

  Change c = adjustMinMax();

  if(_start > _minDateTime) {
    _start = _minDateTime;
    if(!(c & StartChanged))
      c = (Change) (c + StartChanged);
  }
  
  if(_end < _maxDateTime) {
    _end = _maxDateTime;
    if(!(c & EndChanged))
      c = (Change) (c + EndChanged);
  }  
  
  return c;
  
}



KGanttItem::Change 
KGanttItem::adjustMinMax()
//////////////////////////
{
  //
  //  calculate _min and _max by 
  //  traversing the subitems. if there are no subitems
  //  _min = start and _max = end.
  //

  QDateTime min = _minDateTime;
  QDateTime max = _maxDateTime;
  Change c = NoChange;

  if(_subitems.count()==0) {

    _minDateTime = _start;
    _maxDateTime = _end;

    if(min != _minDateTime) c  = MinChanged;
    if(max != _maxDateTime) c = (Change) (c + MaxChanged);

  }
  else {
    
    // get min/max date and time
    
    KGanttItem* item = _subitems.first();
    
    _minDateTime = item->getStart();
    _maxDateTime = item->getEnd();
    
    item = _subitems.next();
    
    for(; item != 0; item = _subitems.next() ) {
      
      if(_minDateTime > item->getStart()) {
	_minDateTime = item->getStart();
      }
      
      if(item->getEnd() > _maxDateTime) {
	_maxDateTime = item->getEnd();
      }
      
    } // for()
    
    
    if(min != _minDateTime) c  = MinChanged;
    if(max != _maxDateTime) c = (Change) (c + MaxChanged);
  
  }

  return c;

}



void 
KGanttItem::subItemChanged(KGanttItem* /*item*/, Change change)
/////////////////////////////////////////////////////
{  
  if(change & StyleChanged)
    emit changed(this, change);
  
  if( (change & Opened) || (change & Closed) || 
      (change & TotalHeightChanged) || (change & HeightChanged) )
    emit changed(this, TotalHeightChanged);

  if( (change & StartChanged) || 
      (change & EndChanged) ) {

    Change c = adjustStartEnd();
    
    if(_mode == Rubberband) {
      if(c & MinChanged && !(c & StartChanged)) 
	c = (Change) (c + StartChanged);
      if(c & MaxChanged && !(c & EndChanged))
	c = (Change) ( c +EndChanged);
    }

    if(c != NoChange)
      emit changed(this, c);
   
  }
}



void 
KGanttItem::setText(const QString& text) 
///////////////////////////////////////
{ 
  if(!_editable) return;
  if(text != _text) {
    _text = text; 
    emit changed(this,TextChanged);
  }
}



void 
KGanttItem::open(bool f)
//////////////////////
{
  if(f != _open) {
    _open = f;
    if(_open)
      emit changed(this, Opened);
    else
      emit changed(this, Closed);
  }
}



void
KGanttItem::select(bool f)
///////////////////////
{
  if(!_editable) return;
  if(f != _selected) {
    _selected = f;
    if(_selected)
      emit changed(this, Selected);
    else
      emit changed(this, Unselected);
  }
}



void 
KGanttItem::setMode(Mode flag) 
////////////////////////////
{
  if(!_editable) return;
  if(_mode != flag) {
    _mode = flag;
    emit changed(this,ModeChanged);
  }
  
}



void
KGanttItem::setStyle(int flag, bool includeSubItems) 
///////////////////////////////////////////////
{
  if(!_editable) return;
  if(_style != flag) {

    _style = flag;
 
    if(includeSubItems)
      for(KGanttItem* item = _subitems.first(); 
	  item != 0; 
	  item = _subitems.next() )
	item->setStyle(flag,true);

    emit changed(this,StyleChanged);

  }

}



void
KGanttItem::setBrush(const QBrush& brush)
///////////////////////////////////////
{
  _brush = brush;
}



void
KGanttItem::setPen(const QPen& pen)
///////////////////////////////
{
  _pen = pen;
}



void
KGanttItem::setHeight(int h)
/////////////////////////
{
  if(!_editable) return;
  if(_height != h) {
    _height = h;
    emit changed(this,HeightChanged);
  }
}



int 
KGanttItem::getTotalHeight()
////////////////////////////////////////
{
  int h = _height;

  if( isOpen() ) {
    for(KGanttItem* item = _subitems.first(); item != 0; item = _subitems.next() ) {
      h += item->getTotalHeight();
    }
  }
  return h;
}



int
KGanttItem::getWidth()
//////////////////
{
  //  int width = _start.secsTo(_end)/60;

  int width = getStart().secsTo(getEnd())/60;
 
  // printf("width[%s] = %d \n", (const char*) getID(), width );

  return width;
}



void 
KGanttItem::dump(QTextOStream& cout, const QString& pre) 
////////////////////////////////////////////////////
{
  cout << pre << "<Item. text = [" << _text << "]>\n";
  cout << pre << "|  start : " << getStart().toString() << "  (" 
       <<_start.toString() << ")" << endl;
  cout << pre << "|  end :   " << getEnd().toString() << "  (" 
       <<_end.toString() << ")" << endl;
  if(_editable)
    cout << pre << "|    - editable " << endl;
  else
    cout << pre << "|    - not editable " << endl;
  if(_mode == Rubberband)
    cout << pre << "|  mode = 'rubberband'" << endl;
  else
    cout << pre << "|  mode = 'normal'" << endl;

  cout << pre << "|  min date/time : " << _minDateTime.toString() << endl;
  cout << pre << "|  max date/time : " << _maxDateTime.toString() << endl;
  
  for(KGanttItem* item = _subitems.first(); item != 0; item = _subitems.next() )
    item->dump(cout, pre + "|   ");
  
  for(KGanttRelation* rel = _relations.first(); 
      rel != 0; 
      rel = _relations.next() )
    rel->dump(cout, pre + "|   ");

  cout << pre << "</Item>\n";

}


QString
KGanttItem::ChangeAsString(Change c)
//////////////////////////////////
{
  QString ret;

  if(c & StartChanged)       ret += "StartChanged, ";
  if(c & EndChanged)         ret += "EndChanged,  ";
  if(c & HeightChanged)      ret += "HeightChanged,  ";
  if(c & TotalHeightChanged) ret += "TotalHeightChanged,  ";
  if(c & StyleChanged)       ret += "StyleChanged,  ";
  if(c & TextChanged)        ret += "TextChanged,  ";
  if(c & ModeChanged)        ret += "ModeChanged,  ";
  if(c & MinChanged)         ret += "MinChanged,  ";
  if(c & MaxChanged)         ret += "MaxChanged,  ";
  if(c & Opened)             ret += "Opened,  ";
  if(c & Closed)             ret += "Closed,  ";
  if(c & Selected)           ret += "Selected, ";
  if(c & Unselected)         ret += "Unselected, ";
  if(c & Unknown)            ret += "Unknown, ";
  return ret;

}
#include "KGanttItem.moc"
