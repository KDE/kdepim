/***************************************************************************
                          knfilterconfigwidget.h  -  description
                             -------------------
    
    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KNFILTERCONFIGWIDGET_H
#define KNFILTERCONFIGWIDGET_H

#include <qtabwidget.h>

class KNStatusFilterWidget;
class KNStringFilterWidget;
class KNRangeFilterWidget;


class KNFilterConfigWidget : public QTabWidget  {

  Q_OBJECT
  
  friend class KNFilterDialog;
  friend class KNSearchDialog;

  public:
    KNFilterConfigWidget(QWidget *parent=0, const char *name=0);
    ~KNFilterConfigWidget();
    
    void reset();
        
  protected:
    KNStatusFilterWidget *status;
    KNStringFilterWidget *subject;
    KNStringFilterWidget *from;
    KNRangeFilterWidget *age;
    KNRangeFilterWidget *lines;
    KNRangeFilterWidget *score;
};

#endif
