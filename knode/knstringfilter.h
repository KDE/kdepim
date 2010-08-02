/*
    knstringfilter.h

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

#ifndef KNSTRINGFILTER_H
#define KNSTRINGFILTER_H

#include <tqgroupbox.h>

class TQCheckBox;
class TQComboBox;

class KLineEdit;
class KSimpleConfig;

class KNGroup;


class KNStringFilter {
  
  friend class KNStringFilterWidget;  

  public:
    KNStringFilter()  { con=true; regExp=false;}
    ~KNStringFilter() {}

    KNStringFilter& operator=(const KNStringFilter &sf);
    /** replace placeholders */
    void expand(KNGroup *g);
          
    void load(KSimpleConfig *conf);
    void save(KSimpleConfig *conf);     
          
    bool doFilter(const TQString &s);
                
  protected:
    TQString data, expanded;
    bool con, regExp;
    
};


//===============================================================================


class KNStringFilterWidget : public TQGroupBox  {
  
  Q_OBJECT

  public:
    KNStringFilterWidget(const TQString& title, TQWidget *parent);
    ~KNStringFilterWidget();
    
    KNStringFilter filter();
    void setFilter(KNStringFilter &f);
    void clear();

    /** usablity hack for the search dialog */
    void setStartFocus();
    
  protected:
    TQCheckBox *regExp;
    TQComboBox *fType;
    KLineEdit *fString;

};


#endif

