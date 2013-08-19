/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNARTICLEFILTER_H
#define KNARTICLEFILTER_H

#include "kngroup.h"
#include "knfolder.h"
#include "knstatusfilter.h"
#include "knrangefilter.h"
#include "knstringfilter.h"

class KNRemoteArticle;
class KNLocalArticle;

namespace KNode {
  class SearchDialog;
}


/** Article filter. */
class KNArticleFilter {

  friend class KNFilterManager;
  friend class KNFilterDialog;
  friend class KNode::SearchDialog;

  public:
    explicit KNArticleFilter(int id=-1);
    /// Copy constructor
    KNArticleFilter(const KNArticleFilter& org);
    ~KNArticleFilter();

    bool loadInfo();
    void load();
    void save();

    void doFilter( KNGroup::Ptr g );
    void doFilter( KNFolder::Ptr f );
    int count()const                     { return c_ount; }
    int id()const                        { return i_d; }
    int applyOn()                   { return static_cast<int>(apon); }
    const QString& name()           { return n_ame; }
    /// *tries* to translate the name
    QString translatedName();
    bool isEnabled()const                { return e_nabled; }
    bool loaded()const                   { return l_oaded; }
    bool isSearchFilter()const           { return s_earchFilter; }

    void setId(int i)               { i_d=i; }
    void setApplyOn(int i)          { apon=(ApOn)i; }
    void setLoaded(bool l)          { l_oaded=l; }
    void setName(const QString &s)  { n_ame=s; }
    /// *tries* to retranslate the name to english
    void setTranslatedName(const QString &s);
    void setEnabled(bool l)         { e_nabled=l; }
    void setSearchFilter(bool b)    { s_earchFilter = b; }

  protected:

    enum ApOn { articles=0 , threads=1 };
    bool applyFilter( KNRemoteArticle::Ptr a );
    bool applyFilter( KNLocalArticle::Ptr a );

    QString n_ame;
    int i_d, c_ount;
    bool l_oaded, e_nabled, translateName, s_earchFilter;
    ApOn apon;

    KNode::StatusFilter status;
    KNode::RangeFilter score, age, lines;
    KNode::StringFilter subject, from, messageId, references;
};

#endif
