/*
    knarticlecollection.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
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
#include "knmime.h"


KNArticleCollection::KNArticleCollection(KNCollection *p)
  : KNCollection(p)
{
  lastID=0;
  len=0;
  siz=0;
  list=0;
  incr=50;
  l_ockedArticles=0;
}



KNArticleCollection::~KNArticleCollection()
{
  clearList();
}



bool KNArticleCollection::resize(int s)
{
  KNArticle **bak=list;
  int nSize;
  
  if(s==0) nSize=siz+incr;
  else nSize=((s/incr)+1)*incr;
  
  list=(KNArticle**) realloc(list,sizeof(KNArticle*)*nSize);

  if(!list) {
    KMessageBox::error(knGlobals.topWidget, i18n("Memory allocation failed!\nYou should close this application now\n, to avoid data loss."));
    list=bak;
    return false;
  }
  else {
    siz=nSize;
    //kdDebug(5003) << "size : " << siz << "\n" << endl;
    return true;
  }
}



bool KNArticleCollection::append(KNArticle *a)
{
  if(len+1>siz)  //array too small
    if (!resize()) return false; //try to realloc

  if(a->id()==-1) a->setId(++lastID);
  list[len]=a;
  len++;
  
  return true;        
}



void KNArticleCollection::clearList()
{
  if(list){
    for(int i=0; i<len; i++) delete list[i];
    free(list);
  }
  
  list=0; len=0; siz=0; lastID=0;
}



void KNArticleCollection::compactList()
{
  int newLen, nullStart=0, nullCnt=0, ptrStart=0, ptrCnt=0;
  
  for(int idx=0; idx<len; idx++) {
    if(list[idx]==0) {
      ptrStart=-1;
      ptrCnt=-1;
      nullStart=idx;
      nullCnt=1;
      for(int idx2=idx+1; idx2<len; idx2++) {
        if(list[idx2]==0) nullCnt++;
        else {
          ptrStart=idx2;
          ptrCnt=1;
          break;
        }
      }
      if(ptrStart!=-1) {
        for(int idx2=ptrStart+1; idx2<len; idx2++) {
          if(list[idx2]!=0) ptrCnt++;
          else break;
        }
        memmove(&(list[nullStart]), &(list[ptrStart]), ptrCnt*sizeof(KNArticle*));
        for(int idx2=nullStart+ptrCnt; idx2<nullStart+ptrCnt+nullCnt; idx2++)
          list[idx2]=0;
        idx=nullStart+ptrCnt-1;
        }
      else break;
    }
  }
  newLen=0;
  while(list[newLen]!=0) newLen++;
  len=newLen;
}



int KNArticleCollection::findId(int id)
{
  int start=0, end=len, mid=0, currentId=0;
  bool found=false;
  end=len;
  KNArticle *current=0;

  while (start!=end && !found) {
    mid=(start+end)/2;
    current=list[mid];
    /*if(!current) {
      int idx=mid;
      while(!current && ++idx<len)
        current=list[idx];
      if(!current) {
        idx=mid;
        while(!current && --idx>=0)
          current=list[mid];
      }
    }

    if(!current) {
      found=false;
      break;
    }*/
    currentId=current->id();
    if(currentId==id)
      found=true;
    else if(currentId < id)
      start=mid+1;
    else
      end=mid;
  }

  if (found) return mid;  
  else {
    kdDebug(5003) << "KNArticleCollection::findId() : ID " << id << " not found!\n" << endl;
    return -1;
  } 
    
  /*int start=0, end=len, mid=0, currentId=0;
  bool found=false;
  
  while (start!=end && !found) {
    mid=(start+end)/2;
    while(mid<len && list[mid]==0) mid++;
    if(mid==len || list[mid]==0) {
      mid=(start+end)/2;
      while(mid>-1 && list[mid]==0) mid--;
    }
    
    if(mid==len || mid==-1 || list[mid]==0) {
      found=false;
      break;
    }     
    else {
      currentId=list[mid]->id();
      if(currentId==id) found=true;
      else if (currentId < id) start=mid+1;
      else end=mid;
    }
  }

  if (found) return list[mid];  
  else {
    kdDebug(5003) << "ID " << id << " not found!\n" << endl;
    return 0;
  } */
}


void KNArticleCollection::setLastID()
{
  if(len>0)
    lastID=list[len-1]->id();
  else
    lastID=0;
}

/*bool KNArticleCollection::setCurrent(KNArticle *a)
{
  if(a) {
    if(byId(a->id())!=0) {
      c_urrent=a;
      return true;
    }
    else {
      c_urrent=0;
      return false;
    }
  }
  else {
    c_urrent=0;
    return true;
  }
}  */


// ================================================================================

KNFile::KNFile(const QString& fname)
 : QFile(fname), filePos(0), readBytes(0)
{
  buffer.resize(512);
  dataPtr=buffer.data();
  dataPtr[0]='\0';
}



KNFile::~KNFile()
{
}



const QCString& KNFile::readLine()
{
  filePos=at();
  readBytes=QFile::readLine(dataPtr, buffer.size()-1);
  if(readBytes!=-1) {
    while ((dataPtr[readBytes-1]!='\n')&&(static_cast<uint>(readBytes+2)==buffer.size())) {  // don't get tricked by files without newline
      at(filePos);
      if (!increaseBuffer() ||
         (readBytes=QFile::readLine(dataPtr, buffer.size()-1))==-1) {
        readBytes=1;
        break;
      }
    }
  } else
    readBytes=1;

  dataPtr[readBytes-1] = '\0';
  return buffer;
}


const QCString& KNFile::readLineWnewLine()
{
  filePos=at();
  readBytes=QFile::readLine(dataPtr, buffer.size()-1);
  if(readBytes!=-1) {
    while ((dataPtr[readBytes-1]!='\n')&&(static_cast<uint>(readBytes+2)==buffer.size())) {  // don't get tricked by files without newline
      at(filePos);
      if (!increaseBuffer() ||
         (readBytes=QFile::readLine(dataPtr, buffer.size()-1))==-1) {
        dataPtr[0] = '\0';
        break;
      }
    }
  }
  else dataPtr[0] = '\0';

  return buffer;
}


bool KNFile::increaseBuffer()
{
  if(buffer.resize(2*buffer.size())) {;
    dataPtr=buffer.data();
    dataPtr[0]='\0';
    kdDebug(5003) << "KNFile::increaseBuffer() : buffer doubled" << endl;
    return true;
  }
  else return false;
}


