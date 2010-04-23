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

#include "knmemorymanager.h"

#include "knfolder.h"
#include "knglobals.h"
#include "knarticlemanager.h"
#include "kngroupmanager.h"
#include "knfoldermanager.h"
#include "settings.h"

#include <KDebug>


KNMemoryManager::KNMemoryManager()
  : c_ollCacheSize(0), a_rtCacheSize(0)
{
}


KNMemoryManager::~KNMemoryManager()
{
  qDeleteAll( mColList );
  qDeleteAll( mArtList );
}


void KNMemoryManager::updateCacheEntry( KNArticleCollection::Ptr c )
{
  CollectionItem *ci;
  int oldSize=0;

  if( (ci=findCacheEntry(c, true)) ) { // item is taken from the list
    oldSize=ci->storageSize;
    ci->sync();
    kDebug(5003) <<"KNMemoryManager::updateCacheEntry() : collection (" << c->name() <<") updated";
  }
  else {
    ci=new CollectionItem(c);
    kDebug(5003) <<"KNMemoryManager::updateCacheEntry() : collection (" << c->name() <<") added";
  }

  mColList.append(ci);
  c_ollCacheSize += (ci->storageSize - oldSize);
  checkMemoryUsageCollections();
}


void KNMemoryManager::removeCacheEntry( KNArticleCollection::Ptr c )
{
  CollectionItem *ci;
  ci=findCacheEntry(c, true);

  if(ci) {
    c_ollCacheSize -= ci->storageSize;
    delete ci;

    kDebug(5003) <<"KNMemoryManager::removeCacheEntry() : collection removed (" << c->name() <<"),"
                  << mColList.count() << "collections left in cache";
  }
}


void KNMemoryManager::prepareLoad( KNArticleCollection::Ptr c )
{
  CollectionItem ci(c);

  c_ollCacheSize += ci.storageSize;
  checkMemoryUsageCollections();
  c_ollCacheSize -= ci.storageSize;
}


void KNMemoryManager::updateCacheEntry( KNArticle::Ptr a )
{
  ArticleItem *ai;
  int oldSize=0;

  if( (ai=findCacheEntry(a, true)) ) {
    oldSize=ai->storageSize;
    ai->sync();
    kDebug(5003) <<"KNMemoryManager::updateCacheEntry() : article updated";
  }
  else {
    ai=new ArticleItem(a);
    kDebug(5003) <<"KNMemoryManager::updateCacheEntry() : article added";
  }

  mArtList.append(ai);
  a_rtCacheSize += (ai->storageSize - oldSize);
  checkMemoryUsageArticles();
}


void KNMemoryManager::removeCacheEntry( KNArticle::Ptr a )
{
  ArticleItem *ai;

  if( (ai=findCacheEntry(a, true)) ) {
    a_rtCacheSize -= ai->storageSize;
    delete ai;

    kDebug(5003) <<"KNMemoryManager::removeCacheEntry() : article removed,"
                  << mArtList.count() << "articles left in cache";

  }
}


KNMemoryManager::CollectionItem * KNMemoryManager::findCacheEntry( KNArticleCollection::Ptr c, bool take )
{
  for ( CollectionItem::List::Iterator it = mColList.begin(); it != mColList.end(); ++it ) {
    if ( (*it)->col == c ) {
      CollectionItem *ret = (*it);
      if ( take )
        mColList.erase( it );
      return ret;
    }
  }

  return 0;
}


KNMemoryManager::ArticleItem* KNMemoryManager::findCacheEntry( KNArticle::Ptr a, bool take )
{
  for ( ArticleItem::List::Iterator it = mArtList.begin(); it != mArtList.end(); ++it ) {
    if ( (*it)->art == a ) {
      ArticleItem *ret = (*it);
      if ( take )
        mArtList.erase( it );
      return ret;
    }
  }

  return 0;
}


void KNMemoryManager::checkMemoryUsageCollections()
{
  int maxSize = knGlobals.settings()->collCacheSize() * 1024;
  KNArticleCollection::Ptr c;

  if (c_ollCacheSize > maxSize) {
    CollectionItem::List tempList( mColList ); // work on a copy, KNGroup-/Foldermanager will
                                                      // modify the original list

    for ( CollectionItem::List::Iterator it = tempList.begin(); it != tempList.end(); ) {
      if ( c_ollCacheSize <= maxSize )
        break;
      // unloadHeaders() will remove the cache entry and thus invalidate the iterator!
      c = (*it)->col;
      ++it;

      if (c->type() == KNCollection::CTgroup)
        knGlobals.groupManager()->unloadHeaders( boost::static_pointer_cast<KNGroup>( c ), false );   // *try* to unload
      else
        if (c->type() == KNCollection::CTfolder)
          knGlobals.folderManager()->unloadHeaders( boost::static_pointer_cast<KNFolder>( c ), false );   // *try* to unload
    }
  }

  kDebug(5003) <<"KNMemoryManager::checkMemoryUsageCollections() :"
                << mColList.count() << "collections in cache => Usage :"
                << ( c_ollCacheSize*100.0 / maxSize ) << "%";
}


void KNMemoryManager::checkMemoryUsageArticles()
{
  int maxSize = knGlobals.settings()->artCacheSize() * 1024;

  if (a_rtCacheSize > maxSize) {
    ArticleItem::List tempList( mArtList ); // work on a copy, KNArticlemanager will
                                                   // modify the original list

    for ( ArticleItem::List::Iterator it = mArtList.begin(); it != mArtList.end(); ) {
      if ( a_rtCacheSize <= maxSize )
        break;
      // unloadArticle() will remove the cache entry and thus invalidate the iterator!
      KNArticle::Ptr art = (*it)->art;
      ++it;
      knGlobals.articleManager()->unloadArticle( art, false );   // *try* to unload
    }
  }

  kDebug(5003) <<"KNMemoryManager::checkMemoryUsageArticles() :"
                << mArtList.count() << "articles in cache => Usage :"
                << ( a_rtCacheSize*100.0 / maxSize ) << "%";
}


void KNMemoryManager::ArticleItem::sync()
{
  storageSize=art->storageSize();
}


void KNMemoryManager::CollectionItem::sync()
{
  storageSize=col->length()*1024; // rule of thumb : ~1k per header
}

