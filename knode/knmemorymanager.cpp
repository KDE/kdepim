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

#include <kdebug.h>

#include "knmemorymanager.h"
#include "knfolder.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "knarticlemanager.h"
#include "kngroupmanager.h"
#include "knfoldermanager.h"


KNMemoryManager::KNMemoryManager()
  : c_ollCacheSize(0), a_rtCacheSize(0)
{
}


KNMemoryManager::~KNMemoryManager()
{
  for ( QValueList<CollectionItem*>::Iterator it = mColList.begin(); it != mColList.end(); ++it )
    delete (*it);
  for ( QValueList<ArticleItem*>::Iterator it = mArtList.begin(); it != mArtList.end(); ++it )
    delete (*it);
}


void KNMemoryManager::updateCacheEntry(KNArticleCollection *c)
{
  CollectionItem *ci;
  int oldSize=0;

  if( (ci=findCacheEntry(c, true)) ) { // item is taken from the list
    oldSize=ci->storageSize;
    ci->sync();
    kdDebug(5003) << "KNMemoryManager::updateCacheEntry() : collection (" << c->name() << ") updated" << endl;
  }
  else {
    ci=new CollectionItem(c);
    kdDebug(5003) << "KNMemoryManager::updateCacheEntry() : collection (" << c->name() << ") added" << endl;
  }

  mColList.append(ci);
  c_ollCacheSize += (ci->storageSize - oldSize);
  checkMemoryUsageCollections();
}


void KNMemoryManager::removeCacheEntry(KNArticleCollection *c)
{
  CollectionItem *ci;
  ci=findCacheEntry(c, true);

  if(ci) {
    c_ollCacheSize -= ci->storageSize;
    delete ci;

    kdDebug(5003) << "KNMemoryManager::removeCacheEntry() : collection removed (" << c->name() << "), "
                  << mColList.count() << " collections left in cache" << endl;
  }
}


void KNMemoryManager::prepareLoad(KNArticleCollection *c)
{
  CollectionItem ci(c);

  c_ollCacheSize += ci.storageSize;
  checkMemoryUsageCollections();
  c_ollCacheSize -= ci.storageSize;
}


void KNMemoryManager::updateCacheEntry(KNArticle *a)
{
  ArticleItem *ai;
  int oldSize=0;

  if( (ai=findCacheEntry(a, true)) ) {
    oldSize=ai->storageSize;
    ai->sync();
    kdDebug(5003) << "KNMemoryManager::updateCacheEntry() : article updated" << endl;
  }
  else {
    ai=new ArticleItem(a);
    kdDebug(5003) << "KNMemoryManager::updateCacheEntry() : article added" << endl;
  }

  mArtList.append(ai);
  a_rtCacheSize += (ai->storageSize - oldSize);
  checkMemoryUsageArticles();
}


void KNMemoryManager::removeCacheEntry(KNArticle *a)
{
  ArticleItem *ai;

  if( (ai=findCacheEntry(a, true)) ) {
    a_rtCacheSize -= ai->storageSize;
    delete ai;

    kdDebug(5003) << "KNMemoryManager::removeCacheEntry() : article removed, "
                  << mArtList.count() << " articles left in cache" << endl;

  }
}


KNMemoryManager::CollectionItem* KNMemoryManager::findCacheEntry(KNArticleCollection *c, bool take)
{
  for ( QValueList<CollectionItem*>::Iterator it = mColList.begin(); it != mColList.end(); ++it ) {
    if ( (*it)->col == c ) {
      CollectionItem *ret = (*it);
      if ( take )
        mColList.remove( it );
      return ret;
    }
  }

  return 0;
}


KNMemoryManager::ArticleItem* KNMemoryManager::findCacheEntry(KNArticle *a, bool take)
{
  for ( QValueList<ArticleItem*>::Iterator it = mArtList.begin(); it != mArtList.end(); ++it ) {
    if ( (*it)->art == a ) {
      ArticleItem *ret = (*it);
      if ( take )
        mArtList.remove( it );
      return ret;
    }
  }

  return 0;
}


void KNMemoryManager::checkMemoryUsageCollections()
{
  int maxSize = knGlobals.configManager()->readNewsGeneral()->collCacheSize() * 1024;
  KNArticleCollection *c=0;

  if (c_ollCacheSize > maxSize) {
    QValueList<CollectionItem*> tempList( mColList ); // work on a copy, KNGroup-/Foldermanager will
                                                      // modify the original list

    for ( QValueList<CollectionItem*>::Iterator it = tempList.begin(); it != tempList.end(); ) {
      if ( c_ollCacheSize <= maxSize )
        break;
      // unloadHeaders() will remove the cache entry and thus invalidate the iterator!
      c = (*it)->col;
      ++it;

      if (c->type() == KNCollection::CTgroup)
        knGlobals.groupManager()->unloadHeaders(static_cast<KNGroup*>(c), false);   // *try* to unload
      else
        if (c->type() == KNCollection::CTfolder)
          knGlobals.folderManager()->unloadHeaders(static_cast<KNFolder*>(c), false);   // *try* to unload
    }
  }

  kdDebug(5003) << "KNMemoryManager::checkMemoryUsageCollections() : "
                << mColList.count() << " collections in cache => Usage : "
                << ( c_ollCacheSize*100.0 / maxSize ) << "%" << endl;
}


void KNMemoryManager::checkMemoryUsageArticles()
{
  int maxSize = knGlobals.configManager()->readNewsGeneral()->artCacheSize() * 1024;

  if (a_rtCacheSize > maxSize) {
    QValueList<ArticleItem*> tempList( mArtList ); // work on a copy, KNArticlemanager will
                                                   // modify the original list

    for ( QValueList<ArticleItem*>::Iterator it = mArtList.begin(); it != mArtList.end(); ) {
      if ( a_rtCacheSize <= maxSize )
        break;
      // unloadArticle() will remove the cache entry and thus invalidate the iterator!
      KNArticle *art = (*it)->art;
      ++it;
      knGlobals.articleManager()->unloadArticle( art, false );   // *try* to unload
    }
  }

  kdDebug(5003) << "KNMemoryManager::checkMemoryUsageArticles() : "
                << mArtList.count() << " articles in cache => Usage : "
                << ( a_rtCacheSize*100.0 / maxSize ) << "%" << endl;
}


void KNMemoryManager::ArticleItem::sync()
{
  storageSize=art->storageSize();
}


void KNMemoryManager::CollectionItem::sync()
{
  storageSize=col->length()*1024; // rule of thumb : ~1k per header
}

