//
//  file    : KGanttBarConfig.cpp
//  date    : 16 jan 2001
//  changed : 
//  author  : jh
//


#include "KGanttBarConfig.h"
#include "xQGanttBarView.h"

#include <kcolorbutton.h> 


KGanttBarConfig::KGanttBarConfig(xQGanttBarView* barview,
				 TQWidget* parent,  
				 const char * name, WFlags f)
  : TQWidget(parent,name,f) 
{
  _barview = barview;
  KColorButton* b = new KColorButton(this);

  connect(b, TQT_SIGNAL(changed(const TQColor&)),
	  this, TQT_SLOT(changeBackground(const TQColor&)));

}


void KGanttBarConfig::changeBackground(const TQColor& color) {
  _barview->viewport()->setBackgroundColor(color);
}
#include "KGanttBarConfig.moc"
