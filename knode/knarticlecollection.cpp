/*
    knarticlecollection.cpp

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

#include <stdlib.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "knglobals.h"
#include "knarticlecollection.h"
#include "knarticle.h"


static const int sizeIncr=50;

KNArticleVector::KNArticleVector(KNArticleVector *master, SortingType sorting)
  : m_aster(master), l_en(0), s_ize(0), l_ist(0), s_ortType(sorting)
{
}


KNArticleVector::~KNArticleVector()
{
  clear();
}


bool KNArticleVector::resize(int s)
{
  KNArticle **bak=l_ist;
  int nSize;

  if(s==0)
    nSize=s_ize+sizeIncr;
  else
    nSize=((s/sizeIncr)+1)*sizeIncr;

  l_ist=(KNArticle**) realloc(l_ist, sizeof(KNArticle*)*nSize);

  if(!l_ist) {
    KMessageBox::error(knGlobals.topWidget, i18n("Memory allocation failed.\nYou should close this application now\nto avoid data loss."));
    l_ist=bak;
    return false;
  }
  else {
    s_ize=nSize;
    //kdDebug(5003) << "size : " << siz << "\n" << endl;
    return true;
  }

}


bool KNArticleVector::append(KNArticle *a, bool autoSort)
{
  if( (l_en+1 > s_ize) && !resize()) // array too small => try to realloc
    return false; // allocation failed !!

  l_ist[l_en++]=a;

  if(autoSort) sort();
  return true;
}


void KNArticleVector::remove(int pos, bool autoDel, bool autoCompact)
{

  if(pos < 0 || pos > l_en-1)
    return;

  if(autoDel)
    delete l_ist[pos];

  l_ist[pos]=0;

  if(autoCompact)
    compact();
}


void KNArticleVector::clear()
{
  if(l_ist){
    if(m_aster==0)
      for(int i=0; i<l_en; i++) delete l_ist[i];
    free(l_ist);
  }

  l_ist=0; l_en=0; s_ize=0;
}


void KNArticleVector::compact()
{
  int newLen, nullStart=0, nullCnt=0, ptrStart=0, ptrCnt=0;

  for(int idx=0; idx<l_en; idx++) {
    if(l_ist[idx]==0) {
      ptrStart=-1;
      ptrCnt=-1;
      nullStart=idx;
      nullCnt=1;
      for(int idx2=idx+1; idx2<l_en; idx2++) {
        if(l_ist[idx2]==0) nullCnt++;
        else {
          ptrStart=idx2;
          ptrCnt=1;
          break;
        }
      }
      if(ptrStart!=-1) {
        for(int idx2=ptrStart+1; idx2<l_en; idx2++) {
          if(l_ist[idx2]!=0) ptrCnt++;
          else break;
        }
        memmove(&(l_ist[nullStart]), &(l_ist[ptrStart]), ptrCnt*sizeof(KNArticle*));
        for(int idx2=nullStart+ptrCnt; idx2<nullStart+ptrCnt+nullCnt; idx2++)
          l_ist[idx2]=0;
        idx=nullStart+ptrCnt-1;
        }
      else break;
    }
  }
  newLen=0;
  while(l_ist[newLen]!=0) newLen++;
  l_en=newLen;
}


void KNArticleVector::syncWithMaster()
{
  if(!m_aster) return;

  if(resize(m_aster->l_en)) {
    memcpy(l_ist, m_aster->l_ist, (m_aster->l_en) * sizeof(KNArticle*));
    l_en=m_aster->l_en;
    sort();
  }
}


void KNArticleVector::sort()
{
  int (*cmp)(const void*,const void*) = 0;

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
    qsort(l_ist, l_en, sizeof(KNArticle*), cmp);
  }
}


int KNArticleVector::compareById(const void *p1, const void *p2)
{
  KNArticle *a1, *a2;
  int rc=0, id1, id2;

  a1=*((KNArticle**)(p1));
  a2=*((KNArticle**)(p2));

  id1=a1->id(),
  id2=a2->id();

  if( id1 < id2 ) rc=-1;
  else if( id1 > id2 ) rc=1;

  return rc;
}


int KNArticleVector::compareByMsgId(const void *p1, const void *p2)
{
  KNArticle *a1, *a2;
  QCString mid1, mid2;

  a1=*(KNArticle**)(p1);
  a2=*(KNArticle**)(p2);

  mid1=a1->messageID(true)->as7BitString(false);
  mid2=a2->messageID(true)->as7BitString(false);

  if(mid1.isNull()) mid1="";
  if(mid2.isNull()) mid2="";

  return strcmp( mid1.data(), mid2.data() );
}


KNArticle* KNArticleVector::bsearch(int id)
{
  int idx=indexForId(id);

  return ( idx>-1 ? l_ist[idx] : 0 );
}


KNArticle* KNArticleVector::bsearch(const QCString &id)
{
  int idx=indexForMsgId(id);

  return ( idx>-1 ? l_ist[idx] : 0 );
}


int KNArticleVector::indexForId(int id)
{
  if(s_ortType!=STid) return -1;

  int start=0, end=l_en, mid=0, currentId=0;
  bool found=false;
  KNArticle *current=0;

  while(start!=end && !found) {
    mid=(start+end)/2;
    current=l_ist[mid];
    currentId=current->id();

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
    #ifndef NDEBUG
    qDebug("knode: KNArticleVector::indexForId() : id=%d not found", id);
    #endif
    return -1;
  }
}


int KNArticleVector::indexForMsgId(const QCString &id)
{
  if(s_ortType!=STmsgId) return -1;

  int start=0, end=l_en, mid=0;
  QCString currentMid=0;
  bool found=false;
  KNArticle *current=0;
  int cnt=0;

  while(start!=end && !found) {
    mid=(start+end)/2;
    current=l_ist[mid];
    currentMid=current->messageID(true)->as7BitString(false);

    if(currentMid==id)
      found=true;
    else if( strcmp(currentMid.data(),  id.data()) < 0 )
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



bool KNArticleCollection::resize(int s)
{
  return a_rticles.resize(s);
}



bool KNArticleCollection::append(KNArticle *a, bool autoSync)
{
  if(a_rticles.append(a, false)) {
    if(a->id()==-1)
      a->setId(++l_astID);
    if(autoSync)
      syncSearchIndex();
    return true;
  }
  return false;

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


KNArticle* KNArticleCollection::byId(int id)
{
  return a_rticles.bsearch(id);
}


KNArticle* KNArticleCollection::byMessageId(const QCString &mid)
{
  if(m_idIndex.isEmpty()) {
    m_idIndex.syncWithMaster();
    kdDebug(5003) << "KNArticleCollection::byMessageId() : created index" << endl;
  }
  return m_idIndex.bsearch(mid);
}


void KNArticleCollection::setLastID()
{
  if(a_rticles.length()>0)
    l_astID=a_rticles.at(a_rticles.length()-1)->id();
  else
    l_astID=0;
}


void KNArticleCollection::syncSearchIndex()
{
  m_idIndex.syncWithMaster();

  /*for(int i=0; i<m_idIndex.length(); i++) {
    kdDebug(5003) << m_idIndex.at(i)->id() << " , " << m_idIndex.at(i)->messageID()->as7BitString(false) << endl;
  } */
}


void KNArticleCollection::clearSearchIndex()
{
  m_idIndex.clear();
}
