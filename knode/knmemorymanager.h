/*
    knmemorymanager.h

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

#ifndef KNMEMORYMANAGER_H
#define KNMEMORYMANAGER_H

#include <qglobal.h>
#if QT_VERSION >= 290
#  include <qptrlist.h>
#else
#  include <qlist.h>
#  define QPtrList QList
#  define QPtrListIterator QListIterator
#endif

class KNArticle;
class KNArticleCollection;


class KNMemoryManager {

  public:
    KNMemoryManager();
    ~KNMemoryManager();

    /** Collection-Handling */
    void updateCacheEntry(KNArticleCollection *c);
    void removeCacheEntry(KNArticleCollection *c);
    /** try to free enough memory for this collection */
    void prepareLoad(KNArticleCollection *c);

    /** Article-Handling */
    void updateCacheEntry(KNArticle *a);
    void removeCacheEntry(KNArticle *a);

  protected:

    class ArticleItem {
    public:
      ArticleItem(KNArticle *a) { art=a; sync(); }
      ~ArticleItem()            {}
      void sync();

      KNArticle *art;
      int storageSize;
    };

    class CollectionItem {
    public:
      CollectionItem(KNArticleCollection *c) { col=c; sync(); }
      ~CollectionItem()                      { }
      void sync();

      KNArticleCollection *col;
      int storageSize;
    };

    CollectionItem* findCacheEntry(KNArticleCollection *c, bool take=false);
    ArticleItem* findCacheEntry(KNArticle *a, bool take=false);
    void checkMemoryUsageCollections();
    void checkMemoryUsageArticles();

    QPtrList<CollectionItem> c_olList;
    QPtrList<ArticleItem> a_rtList;
    int c_ollCacheSize, a_rtCacheSize;
};


#endif
