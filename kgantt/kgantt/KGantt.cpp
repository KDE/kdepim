//
//  file    : KGantt.C
//  date    : 26 oct 2000
//  changed : 10 jan 2001
//  author  : jh
//


#include "KGantt.h"


#include <qcolor.h>
#include <qstylefactory.h>
#include <qscrollview.h> 


KGantt::KGantt(KGanttItem* toplevelitem,
	       QWidget* parent, const char * name, WFlags f)
  : QWidget(parent,name,f)
{ 
#ifdef _DEBUG_
  printf("KGantt::KGantt()\n");
#endif

  if(toplevelitem == 0) {
    _toplevelitem = new KGanttItem(0, "toplevelitem",
			   QDateTime::currentDateTime(),
			   QDateTime::currentDateTime() );
    _toplevelitem->setMode(KGanttItem::Rubberband);
    _deleteItem = true;
  }
  else {
    _toplevelitem = toplevelitem;
    _deleteItem = false;
  }

  setBackgroundColor(QColor(white));

  _splitter = new QSplitter(this);
/*
//  QStyle *cdestyle=QStyleFactory::create("CDE");
//  if(cdestyle)
//	  _splitter->setStyle(cdestyle);
*/
  QPalette pal1(_splitter->palette());
/*  QPalette pal(_splitter->palette());
  QColorGroup cg(pal.active());
  cg.setColor( QColorGroup::Foreground, blue );
  cg.setColor( QColorGroup::Background, white );
  pal.setActive( cg );

  _splitter->setPalette(pal);*/
  
  _ganttlist = new xQGanttListView(_toplevelitem, _splitter); 
  _ganttlist->setMinimumWidth(1);
  _ganttlist->setPalette(pal1);

  _ganttbar = new xQGanttBarView(_toplevelitem, _splitter);
  _ganttbar->setPalette(pal1);

  connect(_ganttbar, SIGNAL(contentsMoving(int,int)),
	  _ganttlist, SLOT(contentsMoved(int,int)));

  _ganttlist->setBarView(_ganttbar);

}



KGantt::~KGantt()
///////////////////
{
  if(_deleteItem)
    delete _toplevelitem;
}




void 
KGantt::dumpItems()
/////////////////////////
{
  QTextOStream cout(stdout);

  cout << "\n<Gantt>\n";
  cout << " start : " << _toplevelitem->getStart().toString() << endl;
  cout << " end :   " << _toplevelitem->getEnd().toString() << endl;

  _toplevelitem->dump(cout, "  ");

  cout << "</Gantt>\n\n";

}



#include "KGantt.moc"
