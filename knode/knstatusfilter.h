/*
    knstatusfilter.h

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

#ifndef KNSTATUSFILTER_H
#define KNSTATUSFILTER_H

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qbitarray.h>

class QCheckBox;
class KSimpleConfig;
class KNRemoteArticle;


class KNStatusFilter {
  
  friend class KNStatusFilterWidget;

  public:
    KNStatusFilter();
    ~KNStatusFilter();
  
    KNStatusFilter& operator=(const KNStatusFilter &sf)
      { for(int i=0; i<8; i++) data.setBit(i, sf.data.at(i)); return (*this); }
    
    void load(KSimpleConfig *conf);
    void save(KSimpleConfig *conf); 
      
    bool doFilter(KNRemoteArticle *a);
    
  protected:  
    QBitArray data;

};


//=================================================================================


class KNStatusFilterWidget : public QButtonGroup  {
  
  Q_OBJECT
  
  public:
    KNStatusFilterWidget(QWidget *parent);
    ~KNStatusFilterWidget();

    KNStatusFilter filter();
    void setFilter(KNStatusFilter &f);
    void clear();   
    
    
  protected:
    
    class TFCombo : public QComboBox {
      
      public:
        TFCombo(QWidget *parent);
        ~TFCombo();
        void setValue(bool b) { if(b) setCurrentItem(0); else setCurrentItem(1); }
        bool value()          { return (currentItem()==0); }
    };
        
    
    QCheckBox *enR, *enN, *enUS, *enNS;
    TFCombo *rCom, *nCom, *usCom, *nsCom;
  
  protected slots:
    void slotEnabled(int c);

};


#define EN_R  0
#define EN_N  1
#define EN_US 2
#define EN_NS 3

#define DAT_R   4
#define DAT_N   5
#define DAT_US  6
#define DAT_NS  7


#endif
