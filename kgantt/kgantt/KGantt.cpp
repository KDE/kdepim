//
//  file    : KGantt.C
//  date    : 26 oct 2000
//  changed : 10 jan 2001
//  author  : jh
//


#include "KGantt.h"


#include <tqcolor.h>
#include <tqstylefactory.h>
#include <tqscrollview.h> 


KGantt::KGantt(KGanttItem* toplevelitem,
	       TQWidget* parent, const char * name, WFlags f)
  : TQWidget(parent,name,f)
{ 
#ifdef _DEBUG_
  printf("KGantt::KGantt()\n");
#endif

  if(toplevelitem == 0) {
    _toplevelitem = new KGanttItem(0, "toplevelitem",
			   TQDateTime::currentDateTime(),
			   TQDateTime::currentDateTime() );
    _toplevelitem->setMode(KGanttItem::Rubberband);
    _deleteItem = true;
  }
  else {
    _toplevelitem = toplevelitem;
    _deleteItem = false;
  }

  setBackgroundColor(TQColor(white));

  _splitter = new TQSplitter(this);
/*
//  TQStyle *cdestyle=TQStyleFactory::create("CDE");
//  if(cdestyle)
//	  _splitter->setStyle(cdestyle);
*/
  TQPalette pal1(_splitter->palette());
/*  TQPalette pal(_splitter->palette());
  TQColorGroup cg(pal.active());
  cg.setColor( TQColorGroup::Foreground, blue );
  cg.setColor( TQColorGroup::Background, white );
  pal.setActive( cg );

  _splitter->setPalette(pal);*/
  
  _ganttlist = new xQGanttListView(_toplevelitem, _splitter); 
  _ganttlist->setMinimumWidth(1);
  _ganttlist->setPalette(pal1);

  _ganttbar = new xQGanttBarView(_toplevelitem, _splitter);
  _ganttbar->setPalette(pal1);

  connect(_ganttbar, TQT_SIGNAL(contentsMoving(int,int)),
	  _ganttlist, TQT_SLOT(contentsMoved(int,int)));

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
  TQTextOStream cout(stdout);

  cout << "\n<Gantt>\n";
  cout << " start : " << _toplevelitem->getStart().toString() << endl;
  cout << " end :   " << _toplevelitem->getEnd().toString() << endl;

  _toplevelitem->dump(cout, "  ");

  cout << "</Gantt>\n\n";

}



#include "KGantt.moc"
