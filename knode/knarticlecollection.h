/*
    knarticlecollection.h

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

#ifndef KNARTICLECOLLECTION_H
#define KNARTICLECOLLECTION_H

#include "kncollection.h"

class KNArticle;


class KNArticleVector {

  public:
    enum SortingType { STid, STmsgId, STunsorted };

    KNArticleVector(KNArticleVector *master=0, SortingType sorting=STunsorted);
    virtual ~KNArticleVector();

    // list-info
    KNArticleVector* master()            { return m_aster; }
    void setMaster(KNArticleVector *m)   { m_aster=m; }
    bool isMaster()   { return (m_aster==0); }

    bool isEmpty()    { return ( (l_ist==0) || (l_en==0) ); }
    int length()      { return l_en; }
    int size()        { return s_ize; }

    // list-handling
    bool resize(int s=0);
    bool append(KNArticle *a, bool autoSort=false);
    void remove(int pos, bool autoDel=false, bool autoCompact=false);
    void clear();
    void compact();
    void syncWithMaster();

    // sorting
    SortingType sortMode()            { return s_ortType; }
    void setSortMode(SortingType s)   { s_ortType=s; }
    void sort();
    static int compareById(const void *a1, const void *a2);
    static int compareByMsgId(const void *a1, const void *a2);

    // article access
    KNArticle* at(int i)  { return ( (i>=0 && i<l_en) ? l_ist[i] : 0 ); }
    KNArticle* bsearch(int id);
    KNArticle* bsearch(const QCString &id);

    int indexForId(int id);
    int indexForMsgId(const QCString &id);

  protected:
    KNArticleVector *m_aster;
    int l_en,
        s_ize;
    KNArticle **l_ist;
    SortingType s_ortType;
};


class KNArticleCollection : public KNCollection {

  public:
    KNArticleCollection(KNCollection *p=0);
    ~KNArticleCollection();

    // info
    bool isEmpty()                { return a_rticles.isEmpty(); }
    bool isLoaded()               { return (c_ount==0 || a_rticles.length()>0); }
    int size()                    { return a_rticles.size(); }
    int length()                  { return a_rticles.length(); }

    // cache behavior
    bool isNotUnloadable()               { return n_otUnloadable; }
    void setNotUnloadable(bool b=true)   { n_otUnloadable = b; }

    // locking
    unsigned int lockedArticles() { return l_ockedArticles; }
    void articleLocked()          { l_ockedArticles++; }
    void articleUnlocked()        { l_ockedArticles--; }

    // list-handling
    bool resize(int s=0);
    bool append(KNArticle *a, bool autoSync=false);
    void clear();
    void compact();
    void setLastID();

    // article access
    KNArticle* at(int i)          { return a_rticles.at(i); }
    KNArticle* byId(int id);
    KNArticle* byMessageId(const QCString &mid);

    // search index
    void syncSearchIndex();
    void clearSearchIndex();

  protected:
    int l_astID;
    unsigned int l_ockedArticles;
    bool n_otUnloadable;
    KNArticleVector a_rticles;
    KNArticleVector m_idIndex;
};


#endif
