/*
    knmemorymanager.cpp

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

#include <kdebug.h>

#include "knmemorymanager.h"
#include "kngroup.h"
#include "knfolder.h"
#include "knmime.h"
#include "knglobals.h"
#include "knconfig.h"
#include "knconfigmanager.h"
#include "knarticlemanager.h"
#include "kngroupmanager.h"
#include "knfoldermanager.h"


KNMemoryManager::KNMemoryManager()
  : c_ollCacheSize(0), a_rtCacheSize(0)
{
  c_olList.setAutoDelete(true);
  a_rtList.setAutoDelete(true);
}


KNMemoryManager::~KNMemoryManager()
{
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

  c_olList.append(ci);
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
                  << c_olList.count() << " collections left in cache" << endl;
  }
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

  a_rtList.append(ai);
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
                  << a_rtList.count() << " articles left in cache" << endl;

  }
}


KNMemoryManager::CollectionItem* KNMemoryManager::findCacheEntry(KNArticleCollection *c, bool take)
{
  for(CollectionItem *i=c_olList.first(); i; i=c_olList.next()) {
    if(i->col==c) {
      if(take)
        c_olList.take();
      return i;
    }
  }

  return 0;
}


KNMemoryManager::ArticleItem* KNMemoryManager::findCacheEntry(KNArticle *a, bool take)
{
  for(ArticleItem *i=a_rtList.first(); i; i=a_rtList.next()) {
    if(i->art==a) {
      if(take)
        a_rtList.take();
      return i;
    }
  }

  return 0;
}


void KNMemoryManager::checkMemoryUsageCollections()
{
  int maxSize = knGlobals.cfgManager->readNewsGeneral()->collCacheSize() * 1024;
  KNArticleCollection *c=0;

  if (c_ollCacheSize > maxSize) {
    QList<CollectionItem> tempList(c_olList);  // work on a copy, KNGroup-/Foldermanager will
                                               // modify the original list

    for( CollectionItem *ci = tempList.first(); ci && (c_ollCacheSize > maxSize); ci = tempList.next() ) {
      c=ci->col;

      if (c->type() == KNCollection::CTgroup)
        knGlobals.grpManager->unloadHeaders(static_cast<KNGroup*>(c), false);   // *try* to unload
      else
        if (c->type() == KNCollection::CTfolder)
          knGlobals.folManager->unloadHeaders(static_cast<KNFolder*>(c), false);   // *try* to unload
    }
  }

  kdDebug(5003) << "KNMemoryManager::checkMemoryUsageCollections() : "
                << c_olList.count() << " collections in cache => Usage : "
                << ( c_ollCacheSize*100.0 / maxSize ) << "%" << endl;
}


void KNMemoryManager::checkMemoryUsageArticles()
{
  int maxSize = knGlobals.cfgManager->readNewsGeneral()->artCacheSize() * 1024;

  if (a_rtCacheSize > maxSize) {
    QList<ArticleItem> tempList(a_rtList);  // work on a copy, KNArticlemanager will
                                            // modify the original list

    for( ArticleItem *ci = tempList.first(); ci && (a_rtCacheSize > maxSize); ci = tempList.next() )
      knGlobals.artManager->unloadArticle(ci->art, false);   // *try* to unload
  }

  kdDebug(5003) << "KNMemoryManager::checkMemoryUsageArticles() : "
                << a_rtList.count() << " articles in cache => Usage : "
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

