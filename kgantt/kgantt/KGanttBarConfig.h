#ifndef _KGANTTBARCONFIG_H_
#define _KGANTTBARCONFIG_H_
  
/*

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    author  : jh, jochen@ifb.bv.tu-berlin.de

    file    : KGanttBarConfig.h
    date    : 16 jan 2001


    changelog :

*/


#include <qwidget.h>

class xQGanttBarView;


class KGanttBarConfig : public QWidget
{

  Q_OBJECT

public:

  KGanttBarConfig(xQGanttBarView* barview,
		  QWidget* parent = 0, const char* name=0, WFlags f=0);
   

protected slots:

  void changeBackground(const QColor& color);
  
protected:

  xQGanttBarView* _barview;

};

#endif
