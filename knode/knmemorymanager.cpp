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
#include "knmime.h"
#include "knglobals.h"
#include "knconfig.h"
#include "knconfigmanager.h"
#include "knarticlemanager.h"



KNMemoryManager::KNMemoryManager()
{
  c_olList.setAutoDelete(true);
  a_rtList.setAutoDelete(true);
  m_emCacheSize=0;
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
  m_emCacheSize += (ci->storageSize - oldSize);
  checkMemoryUsage();
}


void KNMemoryManager::removeCacheEntry(KNArticleCollection *c, bool freeMem)
{
  CollectionItem *ci;
  ci=findCacheEntry(c, true);

  if(ci) {
    m_emCacheSize -= ci->storageSize;
    ArticleItem *ai=a_rtList.first();
    while(ai) {
      if( ai->art->collection()==c ) {
        m_emCacheSize -= ai->storageSize;
        a_rtList.remove();
        ai=a_rtList.current();
      }
      else
        ai=a_rtList.next();
    }


    if( !c->isEmpty() && c->type() == KNCollection::CTgroup )
      static_cast<KNGroup*>(c)->syncDynamicData(); // remeber "Read" - flag

    if(freeMem)
      c->clear();


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
  m_emCacheSize += (ai->storageSize - oldSize);
  checkMemoryUsage();
}


void KNMemoryManager::removeCacheEntry(KNArticle *a, bool freeMem)
{
  ArticleItem *ai;

  if( (ai=findCacheEntry(a, true)) ) {
    m_emCacheSize -= ai->storageSize;
    if(freeMem)
      ai->art->KNMimeContent::clear();
    delete ai;
    a->updateListItem();
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
  for(ArticleItem *i=a_rtList.first(); i; i=a_rtList.next())
    if(i->art==a) {
      if(take)
        a_rtList.take();
      return i;
    }


  return 0;
}


void KNMemoryManager::checkMemoryUsage()
{
  int max_size = knGlobals.cfgManager->readNewsGeneral()->memCacheSize() * 1024;
  KNArticleCollection *c=0;
  KNArticle *a=0;

  while( (m_emCacheSize > max_size) && (c_olList.count() > 1) ) {

    // find first removable item
    for( CollectionItem *ci = c_olList.first(); ci; ci = c_olList.next() ) {
      c=ci->col;

      if( c != knGlobals.artManager->collection() && c->lockedArticles() == 0 &&
          ( c->type() == KNCollection::CTfolder || !( static_cast<KNGroup*>(ci->col)->isLocked() ) )
        )
        break;
      else
        c=0;
    }

    if(c)
      removeCacheEntry(c);
    else {
      kdDebug(5003) << "KNMemoryManager::checkMemoryUsage() : "
                    << "no removable collection found !!" << endl;
      break;
    }
  }

  while( (m_emCacheSize > max_size) && (a_rtList.count() > 1) ) {

    // find first removable item
    for( ArticleItem *ai = a_rtList.first(); ai; ai = a_rtList.next() ) {
      a = ai->art;
      if( !a->isLocked() )
        break;
      else
        a=0;
    }

    if(a)
      removeCacheEntry(a);
    else {
      kdDebug(5003) << "KNMemoryManager::checkMemoryUsage() : "
                    << "no removable article found !!" << endl;
      break;
    }
  }

  kdDebug(5003) << "KNMemoryManager::checkMemoryUsage() : "
                << c_olList.count() << " collections and "
                << a_rtList.count() << " articles in cache => Usage : "
                << ( float(m_emCacheSize*100) / max_size ) << "%" << endl;
}


void KNMemoryManager::ArticleItem::sync()
{
  storageSize=art->storageSize();
}


void KNMemoryManager::CollectionItem::sync()
{
  storageSize=col->length()*1024; // rule of thumb : ~1k per header
}

