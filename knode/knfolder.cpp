/*
    knfolder.cpp

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

#include <qtextstream.h>

#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <kdebug.h>

#include "knaccountmanager.h"
#include "knarticlemanager.h"
#include "knnntpaccount.h"
#include "knstringsplitter.h"
#include "kncollectionviewitem.h"
#include "knhdrviewitem.h"
#include "utilities.h"
#include "knglobals.h"
#include "knarticlefactory.h"
#include "knfolder.h"
#include "knarticlewidget.h"

KNFolder::KNFolder(int id, const QString &name, KNCollection *parent)
  : KNArticleCollection(parent) , i_d(id), i_ndexDirty(false)
{
  n_ame=name;
  QString fname=path()+QString("%1_%2.").arg(n_ame).arg(i_d);
  m_boxFile.setName(fname+"mbox");
  i_ndexFile.setName(fname+"idx");
}


KNFolder::KNFolder(int id, const QString &name, const QString &prefix, KNCollection *parent)
  : KNArticleCollection(parent) , i_d(id), i_ndexDirty(false)
{
  n_ame=name;
  QString fname=path()+QString("%1_%2.").arg(prefix).arg(i_d);
  m_boxFile.setName(fname+"mbox");
  i_ndexFile.setName(fname+"idx");
}


KNFolder::~KNFolder()
{
  if(i_ndexDirty)
    syncIndex();
  closeFiles();
}


void KNFolder::updateListItem()
{
  if(l_istItem)
    l_istItem->setNumber(1,c_ount);
}


QString KNFolder::path()
{
  QString dir(KGlobal::dirs()->saveLocation("appdata","folders/"));
  /*if (dir==QString::null)
    displayInternalFileError();*/
  return dir;
}


bool KNFolder::readInfo(const QString &)
{
#warning IMPLEMENT ME
  return true;
}


void KNFolder::saveInfo()
{
#warning IMPLEMENT ME
  /*QString dir(path());
  if (dir!=QString::null) {
    int pId=-1;
    //if(p_arent) pId=p_arent->id();
    KSimpleConfig info(dir+QString("folder%1.info").arg(i_d));
    
    info.writeEntry("foldername", n_ame);
    info.writeEntry("id", i_d);
    info.writeEntry("parentId", pId);
    info.writeEntry("count", c_ount);
  }*/
}


KNLocalArticle* KNFolder::byId(int id)
{
  int idx=findId(id);
  if(idx!=-1) return ( static_cast<KNLocalArticle*>(list[idx]) );
  else return 0;
}


bool KNFolder::loadHdrs()
{
  if(c_ount==0 || len>0) {
    kdDebug(5003) << "KNFolder::loadHdrs() : already loaded" << endl;
    return true;
  }

  if(!i_ndexFile.open(IO_ReadOnly)) {
    kdError(5003) << "KNFolder::loadHdrs() : cannot open index-file !!" << endl;
    closeFiles();
    return false;
  }

  if(!m_boxFile.open(IO_ReadOnly)) {
    kdError(5003) << "KNFolder::loadHdrs() : cannot open mbox-file !!" << endl;
    closeFiles();
    return false;
  }

  if(!resize(c_ount)) {
    closeFiles();
    return false;
  }

  QCString tmp;
  KNStringSplitter split;
  KNLocalArticle *art;
  DynData dynamic;
  int pos1=0, pos2=0, cnt=0, byteCount;


  while(!i_ndexFile.atEnd()) {

    //read index-data
    byteCount=i_ndexFile.readBlock((char*)(&dynamic), sizeof(DynData));
    if(byteCount!=sizeof(DynData))
      if(i_ndexFile.status() == IO_Ok) {
        kdWarning(5003) << "KNFolder::loadHeaders() : found broken entry in index-file: Ignored!" << endl;
        continue;
      }
      else {
        kdError(5003) << "KNFolder::loadHeaders() : corrupted index-file, IO-error!" << endl;
        closeFiles();
        clearList();
        return false;
      }

    art=new KNLocalArticle(this);


    //set index-data
    art->setId(dynamic.id);
    art->date()->setUnixTime(dynamic.ti);
    art->setStartOffset(dynamic.so);
    art->setEndOffset(dynamic.eo);
    art->setServerId(dynamic.sId);
    art->setDoMail(dynamic.flags[0]);
    art->setMailed(dynamic.flags[1]);
    art->setDoPost(dynamic.flags[2]);
    art->setPosted(dynamic.flags[3]);
    art->setCanceled(dynamic.flags[4]);
    art->setEditDisabled(dynamic.flags[5]);

    //read overview
    if(!m_boxFile.at(art->startOffset())) {
      kdError(5003) << "KNFolder::loadHdrs() : cannot set mbox file-pointer !!" << endl;
      closeFiles();
      clearList();
      return false;
    }
    tmp=m_boxFile.readLine(); //KNFile::readLine()
    if(tmp.isEmpty()) {
      if(m_boxFile.status() == IO_Ok) {
        kdWarning(5003) << "found broken entry in mbox-file: Ignored!" << endl;
        delete art;
        continue;
      }
      else {
        kdError(5003) << "KNFolder::loadHdrs() : corrupted mbox-file, IO-error !!"<< endl;
        closeFiles();
        clearList();
        return false;
      }
    }

    //set overview
    pos1=tmp.find(' ')+1;
    pos2=tmp.find('\t', pos1);
    art->subject()->from7BitString(tmp.mid(pos1, pos2-pos1));
    pos1=pos2+1;
    pos2=tmp.find('\t', pos1);
    art->newsgroups()->from7BitString(tmp.mid(pos1, pos2-pos1));
    pos1=pos2+1;
    pos2=tmp.length();
    art->to()->from7BitString(tmp.mid(pos1,pos2-pos1));

    if(!append(art)) {
      kdError(5003) << "KNFolder::loadHdrs() : cannot append article !!"<< endl;
      delete art;
      clearList();
      closeFiles();
      return false;
    }

    cnt++;
  }

  closeFiles();
  setLastID();
  c_ount=cnt;
  updateListItem();

  return true;
}


bool KNFolder::loadArticle(KNLocalArticle *a)
{
  if(a->hasContent())
    return true;

  closeFiles();
  if(!m_boxFile.open(IO_ReadOnly)) {
    kdError(5003) << "KNFolder::loadArticle(KNLocalArticle *a) : cannot open mbox file: "
                  << m_boxFile.name() << endl;
    return false;
  }

  //set file-pointer
  if(!m_boxFile.at(a->startOffset())) {
    kdError(5003) << "KNFolder::loadArticle(KNLocalArticle *a) : cannot set mbox file-pointer !!" << endl;
    closeFiles();
    return false;
  }

  //read content
  m_boxFile.readLine(); //skip X-KNode-Overview

  unsigned int size=a->endOffset()-m_boxFile.at();
  QCString buff(size+10);
  int readBytes=m_boxFile.readBlock(buff.data(), size);
  closeFiles();
  if(readBytes < (int)(size) && m_boxFile.status() != IO_Ok) {  //cannot read file
    kdError(5003) << "KNFolder::loadArticle(KNLocalArticle *a) : corrupted mbox file, IO-error!" << endl;
    return false;
  }

  //set content
  buff.at(readBytes)='\0'; //terminate string
  a->setContent(buff);
  a->parse();

  return true;
}


bool KNFolder::saveArticles(KNLocalArticle::List *l)
{
  if(!loadHdrs())
    return false;

  if(!m_boxFile.open(IO_WriteOnly | IO_Append)) {
    kdError(5003) << "KNFolder::loadHdrs() : cannot open mbox-file !!" << endl;
    closeFiles();
    return false;
  }

  int idx=0, addCnt=0;
  bool ret=true;
  QTextStream ts(&m_boxFile);
  ts.setEncoding(QTextStream::Latin1);
  DynData dynamic;

  for(KNLocalArticle *a=l->first(); a; a=l->next()) {

    if(a->id()==-1 || a->collection()!=this) {
      if(a->id()!=-1) {
        KNFolder *oldFolder=static_cast<KNFolder*>(a->collection());
        KNLocalArticle::List l;
        l.append(a);
        oldFolder->removeArticles(&l, false);
      }
      if(!append(a)) {
        kdError(5003) << "KNFolder::saveArticle(KNLocalArticle::List *l) : cannot append article !!" << endl;
        ret=false;
        continue;
        a->setCollection(0);
      }
      else {
        a->setCollection(this);
        addCnt++;
      }
    }

    idx=findId(a->id());
    if(idx!=-1 && at(idx)==a) {

      if(!a->hasContent()) {
        KNFolder *f=static_cast<KNFolder*>(a->collection());
        if(!f->loadArticle(a)) {
          ret=false;
          continue;
        }
      }

      //MBox
      ts << "From aaa@aaa Mon Jan 01 00:00:00 1997\n";
      a->setStartOffset(m_boxFile.at()); //save offset

      //write overview information
      ts << "X-KNode-Overview: ";
      ts << a->subject()->as7BitString(false) << '\t';

      KNHeaders::Base* h;
      if( (h=a->newsgroups(false))!=0 )
        ts << h->as7BitString(false);
      ts << '\t';

      if( (h=a->to(false))!=0 )
        ts << h->as7BitString(false);
      ts << '\n';

      //write article
      a->toStream(ts);
      ts << '\n';

      a->setEndOffset(m_boxFile.at()); //save offset


      //update
      KNArticleWidget::articleChanged(a);
      i_ndexDirty=true;

    }
    else {
      kdError(5003) << "KNFolder::saveArticle() : article not in folder !!" << endl;
      ret=false;
    }
  }

  closeFiles();

  if(addCnt>0) {
    c_ount=len;
    updateListItem();
    knGlobals.artManager->updateViewForCollection(this);
  }

  return ret;
}


void KNFolder::removeArticles(KNLocalArticle::List *l, bool del)
{
  int idx=0, delCnt=0;
  for(KNLocalArticle *a=l->first(); a; a=l->next()) {
    if(a->isLocked())
      continue;
    idx=findId(a->id());
    if(idx!=-1 && at(idx)==a) {
      list[idx]=0;
      delCnt++;

      //update
      knGlobals.artFactory->deleteComposerForArticle(a);
      KNArticleWidget::articleRemoved(a);
      delete a->listItem();

      if(del)
        delete a;
      else
        a->setId(-1);

    }
  }

  if(delCnt>0) {
    compactList();
    c_ount-=delCnt;
    updateListItem();
    i_ndexDirty=true;
  }
}


void KNFolder::deleteAll()
{
  if(l_ockedArticles>0)
    return;

  KNLocalArticle *a;
  for(int idx=0; idx<len; idx++) {
    a=at(idx);
    knGlobals.artFactory->deleteComposerForArticle(a);
    KNArticleWidget::articleRemoved(a);
  }

  clearList();
  c_ount=0;
  syncIndex(true);
  saveInfo();

  updateListItem();
}


void KNFolder::syncIndex(bool force)
{
  if(!i_ndexDirty && !force)
    return;

  if(!i_ndexFile.open(IO_WriteOnly)) {
    kdError(5003) << "KNFolder::syncIndex(bool force) : cannot open index-file !!" << endl;
    closeFiles();
    return;
  }

  KNLocalArticle *a;
  DynData d;
  for(int idx=0; idx<len; idx++) {
    a=at(idx);
    d.setData(a);
    i_ndexFile.writeBlock((char*)(&d), sizeof(DynData));
  }
  closeFiles();

  i_ndexDirty=false;
}


void KNFolder::closeFiles()
{
  if(m_boxFile.isOpen())
    m_boxFile.close();
  if(i_ndexFile.isOpen())
    i_ndexFile.close();
}


//==============================================================================


void KNFolder::DynData::setData(KNLocalArticle *a)
{
  id=a->id();
  so=a->startOffset();
  eo=a->endOffset();
  sId=a->serverId();
  ti=a->date()->unixTime();

  flags[0]=a->doMail();
  flags[1]=a->mailed();
  flags[2]=a->doPost();
  flags[3]=a->posted();
  flags[4]=a->canceled();
  flags[5]=a->editDisabled();
}

