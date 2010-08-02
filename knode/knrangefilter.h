/*
    knrangefilter.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNRANGEFILTER_H
#define KNRANGEFILTER_H

#include <tqgroupbox.h>

class TQLabel;
class KIntSpinBox;
class TQComboBox;
class TQCheckBox;

class KSimpleConfig;


class KNRangeFilter {
  
  friend class KNRangeFilterWidget;

  public:
    KNRangeFilter()   { op1=eq; op2=dis; val1=0; val2=0; enabled=false; }
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
    enum Op { gt=0, gtoeq=1, eq=2, ltoeq=3, lt=4, dis=5 };
    bool matchesOp(int v1, Op o, int v2);
    
    int val1, val2;
    Op op1, op2;
    bool enabled;
      
};


//==================================================================================
      

class KNRangeFilterWidget : public TQGroupBox {
      
  Q_OBJECT
    
  public:
    KNRangeFilterWidget(const TQString& value, int min, int max, TQWidget* parent, const TQString &unit=TQString::null);
    ~KNRangeFilterWidget();
        
    KNRangeFilter filter();
    void setFilter(KNRangeFilter &f);
    void clear();
              
  protected:
    TQCheckBox *enabled;
    TQLabel *des;
    KIntSpinBox *val1, *val2;
    TQComboBox *op1, *op2;
  
  protected slots:
    void slotEnabled(bool e);                 
    void slotOp1Changed(int id);
    void slotOp2Changed(int id);
    
};

#endif
