//
//  file    : KGanttBarConfig.cpp
//  date    : 16 jan 2001
//  changed : 
//  author  : jh
//


#include "KGanttBarConfig.h"
#include "xQGanttBarView.h"

#include <kcolorbtn.h> 


KGanttBarConfig::KGanttBarConfig(xQGanttBarView* barview,
				 QWidget* parent,  
				 const char * name, WFlags f)
  : QWidget(parent,name,f) 
{
  _barview = barview;
  KColorButton* b = new KColorButton(this);

  connect(b, SIGNAL(changed(const QColor&)),
	  this, SLOT(changeBackground(const QColor&)));

}


void KGanttBarConfig::changeBackground(const QColor& color) {
  _barview->viewport()->setBackgroundColor(color);
}
#include "KGanttBarConfig.moc"
