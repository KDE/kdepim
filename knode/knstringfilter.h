/***************************************************************************
                          knstringfilter.h  -  description
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
          
    bool doFilter(const QCString &s);
                
  protected:
    QCString data, expanded;
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

