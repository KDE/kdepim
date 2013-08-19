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

#ifndef KNMEMORYMANAGER_H
#define KNMEMORYMANAGER_H

#include "knarticle.h"
#include "knarticlecollection.h"

#include <QList>

class KNArticleCollection;

/** Memory manager. */
class KNMemoryManager {

  public:
    KNMemoryManager();
    ~KNMemoryManager();

    /** Collection-Handling */
    void updateCacheEntry( KNArticleCollection::Ptr c );
    void removeCacheEntry( KNArticleCollection::Ptr c );
    /** try to free enough memory for this collection */
    void prepareLoad( KNArticleCollection::Ptr c );

    /** Article-Handling */
    void updateCacheEntry( KNArticle::Ptr a );
    void removeCacheEntry( KNArticle::Ptr a );

  protected:

    /** Article cache item. */
    class ArticleItem {
    public:
      explicit ArticleItem( KNArticle::Ptr a ) { art=a; sync(); }
      ~ArticleItem()            {}
      void sync();

      KNArticle::Ptr art;
      int storageSize;

      /// List of article cache items.
      typedef QList<KNMemoryManager::ArticleItem*> List;
    };

    /** Group/folder cache item. */
    class CollectionItem {
    public:
      explicit CollectionItem( KNArticleCollection::Ptr c )
      {
        col = c;
        sync();
      }
      ~CollectionItem()                      { }
      void sync();

      KNArticleCollection::Ptr col;
      int storageSize;

      /// List of collection cache items.
      typedef QList<KNMemoryManager::CollectionItem*> List;
    };

    CollectionItem * findCacheEntry( KNArticleCollection::Ptr c, bool take = false );
    ArticleItem * findCacheEntry( KNArticle::Ptr a, bool take = false );
    void checkMemoryUsageCollections();
    void checkMemoryUsageArticles();

    CollectionItem::List mColList;
    ArticleItem::List mArtList;
    int c_ollCacheSize, a_rtCacheSize;
};


#endif
