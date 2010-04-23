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

#include "knarticlecollection.h"

#include "knarticle.h"

#include <KDebug>
#include <QByteArray>


KNArticleVector::KNArticleVector(KNArticleVector *master, SortingType sorting)
  : m_aster( master ),
    s_ortType( sorting )
{
}

KNArticleVector::~KNArticleVector()
{
  clear();
}

void KNArticleVector::append( KNArticle::Ptr a )
{
  mList.append( a );
}

void KNArticleVector::remove( int pos, bool autoDel )
{
  if ( autoDel && pos >= 0 && pos < mList.size() ) {
    mList.takeAt( pos ).reset();
  }
}


void KNArticleVector::clear()
{
  mList.clear();
}


void KNArticleVector::compact()
{
  mList.removeAll( KNArticle::Ptr() );
}


void KNArticleVector::syncWithMaster()
{
  if (!m_aster) return;

  mList = m_aster->mList;
  sort();
}


void KNArticleVector::sort()
{
  bool (*cmp)( KNArticle::Ptr, KNArticle::Ptr ) = 0;

  switch(s_ortType) {
    case STid:
      cmp=compareById;
    break;
    case STmsgId:
      cmp=compareByMsgId;
    break;
    default:
      cmp=0;
    break;
  }

  if(cmp) {
    //compact(); // remove null-pointers
    qSort( mList.begin(), mList.end(), cmp );
  }
}


bool KNArticleVector::compareById( KNArticle::Ptr a1, KNArticle::Ptr a2 )
{
  return ( a1->id() < a2->id() );
}


bool KNArticleVector::compareByMsgId( KNArticle::Ptr a1, KNArticle::Ptr a2 )
{
  QByteArray mid1 = a1->messageID( true )->as7BitString( false );
  QByteArray mid2 = a2->messageID( true )->as7BitString( false );
  return ( mid1 < mid2 );
}


KNArticle::Ptr KNArticleVector::bsearch( int id )
{
  return at( indexForId( id ) );
}


KNArticle::Ptr KNArticleVector::bsearch( const QByteArray &id )
{
  return at ( indexForMsgId( id ) );
}


int KNArticleVector::indexForId(int id)
{
  if(s_ortType!=STid) return -1;

  int start = 0, mid = 0, currentId = 0;
  int end = mList.size();
  bool found=false;

  while(start!=end && !found) {
    mid=(start+end)/2;
    currentId = mList[ mid ]->id();

    if(currentId==id)
      found=true;
    else if(currentId < id)
      start=mid+1;
    else
      end=mid;
  }

  if(found)
    return mid;
  else {
    kDebug() << "id" << id << "not found";
    return -1;
  }
}


int KNArticleVector::indexForMsgId( const QByteArray &id )
{
  if(s_ortType!=STmsgId) return -1;

  int start = 0, mid = 0;
  int end = mList.size();
  QByteArray currentMid;
  bool found=false;
  int cnt=0;

  while(start!=end && !found) {
    mid=(start+end)/2;
    currentMid = mList[ mid ]->messageID( true )->as7BitString( false );

    if(currentMid==id)
      found=true;
    else if( currentMid < id )
      start=mid+1;
    else
      end=mid;

    cnt++;
  }

  if(found) {
    /*#ifndef NDEBUG
    qDebug("KNArticleVector::indexForMsgID() : msgID=%s found after %d compares", id.data(), cnt);
    #endif*/
    return mid;
  }
  else {
    /*#ifndef NDEBUG
    qDebug("knode: KNArticleVector::indexForMsgID() : msgID=%s not found", id.data());
    #endif*/
    return -1;
  }
}




// -------------------------------------------------------------------------------------------



KNArticleCollection::KNArticleCollection(KNCollection *p)
  : KNCollection(p), l_astID(0), l_ockedArticles(0), n_otUnloadable(false)
{
  a_rticles.setSortMode(KNArticleVector::STid);
  m_idIndex.setSortMode(KNArticleVector::STmsgId);
  m_idIndex.setMaster(&a_rticles);
}



KNArticleCollection::~KNArticleCollection()
{
  clear();
}

void KNArticleCollection::append( KNArticle::Ptr a )
{
  a_rticles.append( a );
  if( a->id() == -1 ) {
    a->setId( ++l_astID );
  }
}

void KNArticleCollection::clear()
{
  a_rticles.clear();
  m_idIndex.clear();
  l_astID=0;
}



void KNArticleCollection::compact()
{
  a_rticles.compact();
  m_idIndex.clear();
}


KNArticle::Ptr KNArticleCollection::byId( int id )
{
  return a_rticles.bsearch(id);
}


KNArticle::Ptr KNArticleCollection::byMessageId( const QByteArray &mid )
{
  if(m_idIndex.isEmpty()) {
    m_idIndex.syncWithMaster();
    kDebug(5003) <<"KNArticleCollection::byMessageId() : created index";
  }
  return m_idIndex.bsearch(mid);
}


void KNArticleCollection::setLastID()
{
  if ( !a_rticles.isEmpty() ) {
    l_astID = a_rticles.at( a_rticles.size()-1 )->id();
  } else {
    l_astID=0;
  }
}


void KNArticleCollection::syncSearchIndex()
{
  m_idIndex.syncWithMaster();

  /*for(int i=0; i<m_idIndex.length(); i++) {
    kDebug(5003) << m_idIndex.at(i)->id() <<" ," << m_idIndex.at(i)->messageID()->as7BitString(false);
  } */
}


void KNArticleCollection::clearSearchIndex()
{
  m_idIndex.clear();
}
