/***************************************************************************
                          knrangefilter.h  -  description
                             -------------------

    copyright            : (C) 1999 by Christian Thurner
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

#ifndef KNRANGEFILTER_H
#define KNRANGEFILTER_H

#include <qgroupbox.h>

class QLabel;
class KIntSpinBox;
class QComboBox;
class QCheckBox;

class KSimpleConfig;


class KNRangeFilter {
  
  friend class KNRangeFilterWidget;

  public:
    KNRangeFilter()   { op1=eq; op2=gt; val1=0; val2=0; enabled=false; }
    ~KNRangeFilter()  {}
    
    KNRangeFilter& operator=(const KNRangeFilter &nr)
      { val1=nr.val1; val2=nr.val2;
        op1=nr.op1; op2=nr.op2;
        enabled=nr.enabled;
        return (*this); }
    
    void load(KSimpleConfig *conf);
    void save(KSimpleConfig *conf);     
        
    bool doFilter(int a);
    
  protected:
    enum Op { gt=0, gtoeq=1, eq=2 };
    bool matchesOp(int v1, Op o, int v2);
    
    int val1, val2;
    Op op1, op2;
    bool enabled;
      
};


//==================================================================================
      

class KNRangeFilterWidget : public QGroupBox {
      
  Q_OBJECT
    
  public:
    KNRangeFilterWidget(const QString& value, int min, int max, QWidget* parent, const QString &unit=QString::null);
    ~KNRangeFilterWidget();
        
    KNRangeFilter filter();
    void setFilter(KNRangeFilter &f);
    void clear();
              
  protected:
    QCheckBox *enabled;
    QLabel *des;
    KIntSpinBox *val1, *val2;
    QComboBox *op1, *op2;
  
  protected slots:
    void slotEnabled(bool e);                 
    void slotOp1Changed(int id);
    
};

#endif
