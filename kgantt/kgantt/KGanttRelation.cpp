//
//  file    : KGanttRelation.cpp
//  date    : 17 feb 2001
//  changed : 
//  author  : jh
//


#include "KGanttRelation.h"
#include "KGanttItem.h"


QPen KGanttRelation::_selectPen(QColor(255,0,0));


KGanttRelation::KGanttRelation(KGanttItem* from, KGanttItem* to,
			       const QString& text )
  : QObject()
////////////////////////////////////////////////////////
{
  _from = from;
  _to = to;
  _text = text;
  _pen = QPen(QColor(20,20,20),1);

  connect(from, SIGNAL(destroyed(KGanttItem*)),
	  this, SLOT(itemDestroyed(KGanttItem*)));

  connect(to, SIGNAL(destroyed(KGanttItem*)),
	  this, SLOT(itemDestroyed(KGanttItem*)));

}




KGanttRelation::~KGanttRelation()
/////////////////
{
#ifdef _DEBUG_
  printf("-> delete Relation %s \n", getText().latin1() );
#endif

  emit destroyed(this);

#ifdef _DEBUG_
  printf("<- delete Relation %s \n", getText().latin1() );
#endif
}



KGanttItem* 
KGanttRelation::getFrom()
{ 
  return _from;
}




KGanttItem*
KGanttRelation::getTo()
/////////////////
{
  return _to; 
}



void
KGanttRelation::itemDestroyed(KGanttItem* /*item*/)
{
  delete this;
}



void 
KGanttRelation::setText(const QString& text) 
///////////////////////////////////////
{ 
  if(!_editable) return;
  if(text != _text) {
    _text = text; 
    emit changed(this,TextChanged);
  }
}



void
KGanttRelation::select(bool f)
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
KGanttRelation::setPen(const QPen& pen)
{
  _pen = pen;
}




void 
KGanttRelation::dump(QTextOStream& cout, const QString& pre) 
{
  cout << pre << "<Relation. text = [" << _text << "]>\n";

  cout << pre << "|  from : " << getFrom()->getText().latin1() << endl;
  cout << pre << "|  to   : " << getTo()->getText().latin1() << endl;

  if(_editable)
    cout << pre << "|    - editable " << endl;
  else
    cout << pre << "|    - not editable " << endl;

  if(_selected)
    cout << pre << "|    - selected " << endl;
  else
    cout << pre << "|    - not selected " << endl;

  cout << pre << "</Relation>\n";

}


QString
KGanttRelation::ChangeAsString(Change c)
//////////////////////////////////
{
  QString ret;

  /*
  if(c & StartChanged)       ret += "StartChanged, ";
  if(c & EndChanged)         ret += "EndChanged,  ";
  if(c & HeightChanged)      ret += "HeightChanged,  ";
  if(c & TotalHeightChanged) ret += "TotalHeightChanged,  ";
  if(c & StyleChanged)       ret += "StyleChanged,  ";
  */
  if(c & TextChanged)        ret += "TextChanged,  ";
  /*
  if(c & ModeChanged)        ret += "ModeChanged,  ";
  if(c & MinChanged)         ret += "MinChanged,  ";
  if(c & MaxChanged)         ret += "MaxChanged,  ";
  if(c & Opened)             ret += "Opened,  ";
  if(c & Closed)             ret += "Closed,  ";
  if(c & Selected)           ret += "Selected, ";
  if(c & Unselected)         ret += "Unselected, ";
  if(c & Unknown)            ret += "Unknown, ";
  */
  return ret;

}
#include "KGanttRelation.moc"
