/*
    knarticlefilter.h

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

#ifndef KNARTICLEFILTER_H
#define KNARTICLEFILTER_H

#include <qstring.h>
#include "knstatusfilter.h"
#include "knrangefilter.h"
#include "knstringfilter.h"

class KNRemoteArticle;
class KNGroup;

class KNArticleFilter {
  
  friend class KNFilterManager;
  friend class KNFilterDialog;
  friend class KNSearchDialog;  

  public:
    KNArticleFilter(int id=-1);
    KNArticleFilter(const KNArticleFilter& org);   // constructs a copy of org
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
    QString translatedName();        // *trys* to translate the name
    bool isEnabled()                { return e_nabled; }
    bool loaded()                   { return l_oaded; }

    void setId(int i)               { i_d=i; }
    void setApplyOn(int i)          { apon=(ApOn)i; }
    void setLoaded(bool l)          { l_oaded=l; }
    void setName(const QString &s)  { n_ame=s; }
    void setTranslatedName(const QString &s);     // *trys* to retranslate the name to english
    void setEnabled(bool l)         { e_nabled=l; }
    
  protected:
    
    enum ApOn { articles=0 , threads=1 };
    bool applyFilter(KNRemoteArticle *a);
      
    QString n_ame;
    int i_d, c_ount;
    bool l_oaded, e_nabled, translateName;
    ApOn apon;
    
    KNStatusFilter status;
    KNRangeFilter score, age, lines;
    KNStringFilter subject,from;      
};

#endif
