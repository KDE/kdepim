/***************************************************************************
                          knarticlefilter.h  -  description
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


#ifndef KNARTICLEFILTER_H
#define KNARTICLEFILTER_H

#include <qstring.h>

class KNStatusFilter;
class KNRangeFilter;
class KNStringFilter;
class KNFetchArticle;
class KNGroup;

class KNArticleFilter {
  
  friend class KNFilterManager;
  friend class KNFilterDialog;
  friend class KNSearchDialog;  

  public:
    KNArticleFilter(int id=-1);
    ~KNArticleFilter();
    
    bool loadInfo();
    void load();
    void save();
                
    void doFilter(KNGroup *g);
    //bool applyFilter(KNSavedArticle *a);
    int count()                     { return c_ount; }
    int id()                        { return i_d; }
    int applyOn()                   { return static_cast<int>(apon); }
    const QString& name()           { return n_ame; }
    bool isEnabled()                { return e_nabled; }
    bool loaded()                   { return l_oaded; }
    
    
    void setId(int i)               { i_d=i; }
    void setApplyOn(int i)          { apon=(ApOn)i; }
    void setLoaded(bool l)          { l_oaded=l; }
    void setName(const QString &s)  { n_ame=s; }
    void setEnabled(bool l)         { e_nabled=l; }
    
  protected:
    
    enum ApOn { articles=0 , threads=1 };
    bool applyFilter(KNFetchArticle *a);
      
    QString n_ame;
    int i_d, c_ount;
    bool l_oaded, e_nabled;
    ApOn apon;
    
    KNStatusFilter status;
    KNRangeFilter score, age, lines;
    KNStringFilter subject,from;      
};

#endif
