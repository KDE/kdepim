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

#ifndef KNARTICLECOLLECTION_H
#define KNARTICLECOLLECTION_H

#include "kncollection.h"

#include <QByteArray>

class KNArticle;


/** Article storage used by KNArticleCollection.
 */
class KNArticleVector {

  public:
    enum SortingType { STid, STmsgId, STunsorted };

    KNArticleVector(KNArticleVector *master=0, SortingType sorting=STunsorted);
    virtual ~KNArticleVector();

    // list-info
    void setMaster(KNArticleVector *m)   { m_aster=m; }

    bool isEmpty()    { return ( (l_ist==0) || (l_en==0) ); }
    int length()      { return l_en; }
    int size()        { return s_ize; }

    // list-handling
    bool resize(int s=0);
    /**
      Appends an article to this store.
    */
    bool append( KNArticle *a );
    /**
      Remove the element at position @p pos in this store.
    */
    void remove( int pos, bool autoDel=false );
    void clear();
    void compact();
    void syncWithMaster();

    // sorting
    void setSortMode(SortingType s)   { s_ortType=s; }
    static int compareById(const void *a1, const void *a2);
    static int compareByMsgId(const void *a1, const void *a2);

    // article access
    KNArticle* at(int i)  { return ( (i>=0 && i<l_en) ? l_ist[i] : 0 ); }
    KNArticle* bsearch(int id);
    KNArticle* bsearch( const QByteArray &id );

    int indexForId(int id);
    int indexForMsgId( const QByteArray &id );

  protected:
    void sort();

    KNArticleVector *m_aster;
    int l_en,
        s_ize;
    KNArticle **l_ist;
    SortingType s_ortType;
};


/** Abstract base class for article collections, ie. news groups and folders.
 */
class KNArticleCollection : public KNCollection {

  public:
    KNArticleCollection(KNCollection *p=0);
    ~KNArticleCollection();

    /** Returns true if this collection doesn't contain any article. */
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
    /**
      Appends an article to this collection.
    */
    bool append( KNArticle *a );
    void clear();
    void compact();
    void setLastID();

    // article access
    KNArticle* at(int i)          { return a_rticles.at(i); }
    KNArticle* byId(int id);
    KNArticle* byMessageId( const QByteArray &mid );

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
