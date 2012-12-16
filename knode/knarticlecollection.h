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

#include "knarticle.h"
#include "kncollection.h"

#include <QByteArray>


/** Article storage used by KNArticleCollection.
 */
class KNArticleVector {

  public:
    enum SortingType { STid, STmsgId, STunsorted };

    explicit KNArticleVector(KNArticleVector *master=0, SortingType sorting=STunsorted);
    virtual ~KNArticleVector();

    // list-info
    void setMaster(KNArticleVector *m)   { m_aster=m; }

    bool isEmpty()    { return mList.isEmpty(); }
    int size()        { return mList.size(); }

    // list-handling
    /**
      Appends an article to this store.
    */
    void append( KNArticle::Ptr a );
    /**
      Remove the element at position @p pos in this store.
    */
    void remove( int pos );
    void clear();
    void syncWithMaster();

    // sorting
    void setSortMode(SortingType s)   { s_ortType=s; }
    static bool compareById( KNArticle::Ptr a1, KNArticle::Ptr a2 );
    static bool compareByMsgId( KNArticle::Ptr a1, KNArticle::Ptr a2 );

    // article access
    KNArticle::Ptr at( int i )  { return mList.value( i ); }
    KNArticle::Ptr bsearch( int id );
    KNArticle::Ptr bsearch( const QByteArray &id );

    int indexForId(int id);
    int indexForMsgId( const QByteArray &id );

  private:
    void sort();

    KNArticleVector *m_aster;
    QList<KNArticle::Ptr> mList;
    SortingType s_ortType;
};


/** Abstract base class for article collections, ie. news groups and folders.
 */
class KNArticleCollection : public KNCollection {

  public:
    /**
     * Shared pointer to a KNArticle. To be used instead of raw KNArticleCollection*.
     */
    typedef boost::shared_ptr<KNArticleCollection> Ptr;
    /**
     * List of KNArticleCollection.
     */
    typedef QList<KNArticleCollection::Ptr> List;


    explicit KNArticleCollection( KNCollection::Ptr p = KNCollection::Ptr() );
    ~KNArticleCollection();

    /** Returns true if this collection doesn't contain any article. */
    bool isEmpty()                { return a_rticles.isEmpty(); }
    bool isLoaded()               { return ( c_ount==0 || !a_rticles.isEmpty() ); }
    int length()                  { return a_rticles.size(); }

    // cache behavior
    bool isNotUnloadable()               { return n_otUnloadable; }
    void setNotUnloadable(bool b=true)   { n_otUnloadable = b; }

    // locking
    unsigned int lockedArticles() { return l_ockedArticles; }
    void articleLocked()          { l_ockedArticles++; }
    void articleUnlocked()        { l_ockedArticles--; }

    // list-handling
    /**
      Appends an article to this collection.
    */
    void append( KNArticle::Ptr a );
    /**
     * Remove the article @p art from this collection.
     */
    void remove( const KNArticle::Ptr &art );
    void clear();
    void compact();
    void setLastID();

    // article access
    /**
      Returns the article at index @p i in this collection, or an empty KNArticle::Ptr if it is not found.
    */
    KNArticle::Ptr at( int i )    { return a_rticles.at(i); }
    /**
      Returns the article whose id is @p id, or an empty KNArticle::Ptr if it is not found.
    */
    KNArticle::Ptr byId( int id );
    /**
      Returns the article whose message-id is @p mid, or an empty KNArticle::Ptr if it is not found.
    */
    KNArticle::Ptr byMessageId( const QByteArray &mid );

    // search index
    void syncSearchIndex();

  private:
    int l_astID;
    unsigned int l_ockedArticles;
    bool n_otUnloadable;
    KNArticleVector a_rticles;
    KNArticleVector m_idIndex;
};


#endif
