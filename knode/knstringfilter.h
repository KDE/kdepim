/*
    knstringfilter.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNSTRINGFILTER_H
#define KNSTRINGFILTER_H

#include <qgroupbox.h>

class QCheckBox;
class QComboBox;
class QLineEdit;

class KSimpleConfig;

class KNGroup;

class KNStringFilter {
  
  friend class KNStringFilterWidget;  

  public:
    KNStringFilter()  { enabled=false; con=true; regExp=false;}
    ~KNStringFilter() {}

    KNStringFilter& operator=(const KNStringFilter &sf);

    void expand(KNGroup *g);  // replace placeholders
          
    void load(KSimpleConfig *conf);
    void save(KSimpleConfig *conf);     
          
    bool doFilter(const QString &s);
                
  protected:
    QString data, expanded;
    bool con, enabled, regExp;  
    
};


//===============================================================================


class KNStringFilterWidget : public QGroupBox  {
  
  Q_OBJECT

  public:
    KNStringFilterWidget(const QString& title, QWidget *parent);
    ~KNStringFilterWidget();
    
    KNStringFilter filter();
    void setFilter(KNStringFilter &f);
    void clear();
    
  protected:
    QCheckBox *enabled, *regExp;
    QComboBox *fType;
    QLineEdit *fString;
  
  protected slots:
    void slotEnabled(bool e);
    
};


#endif

