/*
    knfilterconfigwidget.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

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

    void setStartFocus();      // useablity hack for the search dialog
        
  protected:
    KNStatusFilterWidget *status;
    KNStringFilterWidget *subject;
    KNStringFilterWidget *from;
    KNRangeFilterWidget *age;
    KNRangeFilterWidget *lines;
    KNRangeFilterWidget *score;
};

#endif
