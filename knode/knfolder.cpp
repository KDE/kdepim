/***************************************************************************
                          knfolder.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <qtextstream.h>

#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <kdebug.h>

#include "knaccountmanager.h"
#include "knnntpaccount.h"
#include "knsavedarticle.h"
#include "knstringsplitter.h"
#include "kncollectionviewitem.h"
#include "knhdrviewitem.h"
#include "utilities.h"
#include "knglobals.h"
#include "knfolder.h"


KNFolder::KNFolder(KNCollection *p) : KNArticleCollection(p)
{
  t_oSync=false;
}



KNFolder::~KNFolder()
{
}



QString KNFolder::path()
{
  QString dir(KGlobal::dirs()->saveLocation("appdata","folders/"));
  if (dir==QString::null)
    displayInternalFileError();
  return dir;
}



void KNFolder::updateListItem()
{
  if(l_istItem)
    l_istItem->setNumber(1,c_ount);
}



bool KNFolder::readInfo(const QString &)
{
  return true;
}



void KNFolder::saveInfo()
{
  QString dir(path());
  if (dir!=QString::null) {
    int pId=-1;
    //if(p_arent) pId=p_arent->id();
    KSimpleConfig info(dir+QString("folder%1.info").arg(i_d));
    
    info.writeEntry("foldername", n_ame);
    info.writeEntry("id", i_d);
    info.writeEntry("parentId", pId);
    info.writeEntry("count", c_ount);
  }
}



void KNFolder::syncDynamicData(bool force)
{
  if(force) t_oSync=true;
  if(t_oSync) saveDynamicData(0, len, true);
}
            


KNSavedArticle* KNFolder::byId(int id)
{
  int idx=findId(id);
  if(idx!=-1) return ((KNSavedArticle*)list[idx]);
  else return 0;
}




bool KNFolder::loadHdrs()
{
  QString dir(path());
  QCString tmp;
  KNFile f;
  KNStringSplitter split;
  KNSavedArticle *art;
  dynData dynamic;
  int pos1=0, pos2=0, cnt=0, byteCount;
  
  if (dir==QString::null)
    return false;
    
  if(c_ount>0 && len==0) {
    kdDebug(5003) << "KNFolder::loadIndex() : loading headers" << endl;
    if(!resize(c_ount)) return false;
    
    f.setName(dir+QString("folder%1.idx").arg(i_d));
    
    if(f.open(IO_ReadOnly)) {
      while(!f.atEnd()) {
        byteCount = f.readBlock((char*)(&dynamic), sizeof(dynData));
        if ((byteCount == -1)||(byteCount!=sizeof(dynData)))
          if (f.status() == IO_Ok) {
            kdWarning(5003) << "Found broken entry in dynamic-file: Ignored!" << endl;
            continue;
          } else {
            kdError(5003) << "Corrupted dynamic file, IO-error!" << endl;
            clearList();
            return false;
          }                 
        art=new KNSavedArticle();
        art->setId(dynamic.id);
        art->setStatus((KNArticleBase::articleStatus)dynamic.status);
        art->setTimeT(dynamic.ti);
        art->setStartOffset(dynamic.so);
        art->setEndOffset(dynamic.eo);
        art->setServerId(dynamic.sId);
        art->setFolder(this);
        cnt++;
        if(!append(art)) {
          delete art;
          clearList();
          f.close();
          return false;
        }
      }
      f.close();
      setLastID();
      c_ount=cnt;
      updateListItem();
    }
    else {
      displayInternalFileError();
      return false;
    }

      
    f.setName(dir+QString("folder%1.mbox").arg(i_d));

    if(f.open(IO_ReadOnly)) {
      for(int idx=0; idx<len; idx++) {
        art=at(idx);
        if(f.at(art->startOffset())) {
          tmp = f.readLine();   
          if(tmp.isEmpty()){
            if (f.status() == IO_Ok) {
              kdWarning(5003) << "Found broken entry in mbox-file: Ignored!" << endl;
              removeArticle(art);
              delete art;
              continue;
            }         
          }
          pos1=tmp.find(' ')+1;
          pos2=tmp.find('\t', pos1);
          art->setSubject(tmp.mid(pos1, pos2-pos1));
          pos1=pos2+1;
          art->setDestination(tmp.mid(pos1, tmp.length()-pos1));
        }
      }
    }
    else {
      displayInternalFileError();
      clearList();
      return false;
    }
    return true;
  }
  else {
    kdDebug(5003) << "KNFolder::loadIndex() : already loaded" << endl;
    return true;
  }
}



void KNFolder::saveDynamicData(int start, int cnt, bool ovr)
{
  int mode;
  KNSavedArticle *art;
  dynData dynamic;

  QString dir(path());  
  if (dir != QString::null) {
    QFile f(dir+QString("folder%1.idx").arg(i_d));
  
    if(ovr) mode=IO_WriteOnly;
    else mode=IO_WriteOnly | IO_Append;

    if(f.open(mode)) {
      for(int idx=start; idx<(start+cnt); idx++) {
        art=at(idx);
        dynamic.setData(art);
        f.writeBlock((char*)(&dynamic), sizeof(dynData));   
      }
      f.close();
    } else
      displayInternalFileError();
  }
}



bool KNFolder::loadArticle(KNSavedArticle *a)
{
  QCString line;  
  bool isHead=true;
  
  if(a->hasContent()) return true;    

  QString dir(path());
  if (dir != QString::null) {
    KNFile f(dir+QString("folder%1.mbox").arg(i_d));  
    if(f.open(IO_ReadOnly)) {
      if(!f.at(a->startOffset())) {
        f.close();
        return false;
      }
      else {
        a->initContent();
        line=f.readLine();
        while(f.at() < a->endOffset()) {
          line=f.readLine();
          if(line.isEmpty()) {
            if (f.status() != IO_Ok) {
              kdError(5003) << "Corrupted mbox file, IO-error!" << endl;
              return false;
            }
            else if(isHead) {
              isHead=false;
              continue;
            }
          }
          if(isHead) a->addHeaderLine(line.data());
          else a->addBodyLine(line.data());
        }
        a->parse();
      }
      return true;
    }
    else
      return false;   
  } else
    return false;
}



void KNFolder::saveStaticData(int, int cnt, bool ovr)
{
  int mode;
  KNSavedArticle *art;
  
  QString dir(path());  
  if (dir == QString::null)
    return;
    
  QFile f(dir+QString("folder%1.mbox").arg(i_d));     

  if(ovr) mode=IO_WriteOnly;
  else mode=IO_WriteOnly | IO_Append;

  if(f.open(mode)) {
    QTextStream ts(&f);

    for(int idx=len-cnt; idx<len; idx++) {
      ts << "From aaa@aaa Mon Jan 01 00:00:00 1997\n";    
      art=at(idx);
      art->setStartOffset(f.at());
      ts << "X-KNode-Overview: ";
      ts << art->subject() << '\t';
      ts << art->destination() << '\n';
      art->toStream(ts);
      ts << '\n';
      //f.flush();
      art->setEndOffset(f.at());
    }
    f.close();
  }
  else displayInternalFileError();
}



bool KNFolder::addArticle(KNSavedArticle *a)
{
  KNFolder *oldFolder=a->folder();
  if(!loadHdrs()) return false;
  if(oldFolder==this) return true;
  
  if(!resize(siz+1)) return false;
  
  if(oldFolder) oldFolder->removeArticle(a);
  a->setId(-1);
  if(!append(a)) {
    delete a;
    return false;
  }
  else {
    a->setFolder(this);
    //if(!a->type()==KNArticleBase::ATcontrol) {
      saveStaticData(len-1, 1);
      saveDynamicData(len-1, 1);
    //}
    c_ount++;
    updateListItem();
    delete a->listItem();
    a->setListItem(0);
    return true;
  } 
}



bool KNFolder::saveArticle(KNSavedArticle *a)
{
  int idx;
  if(a->folder()!=this) return addArticle(a);
  else {
    idx=findId(a->id());
    if(idx!=-1 && at(idx)==a) {
      saveStaticData(len-1, 1);
      t_oSync=true;
      return true;
    }
    else {
      kdWarning(5003) << "KNFolder::saveArticle() : article not in folder !!" << endl;
      return false;
    }
  }     
}



void KNFolder::removeArticle(KNSavedArticle *a)
{
  int idx=findId(a->id());
  
  if(idx!=-1 && at(idx)==a) {
    list[idx]=0;
    compactList();
    c_ount--;
    updateListItem();
    t_oSync=true;
  }
}



void KNFolder::deleteAll()
{
  KNSavedArticle *a;
  int lastId=-1;
  KNNntpAccount *lastAcc=0;
  for(int idx=0; idx<len; idx++) {
    a=at(idx);
    if(!a->sent() && !a->isMail()) {
      if(a->serverId()!=lastId) {
        lastId=a->serverId();
        lastAcc=knGlobals.accManager->account(lastId);
      }
      if(lastAcc) lastAcc->decUnsentCount();
    }
  }
  clearList();
  saveStaticData(0,0, true);
  saveDynamicData(0,0, true);
  c_ount=0;
  saveInfo();
  t_oSync=false;
  updateListItem(); 
}


//==============================================================================

void KNFolder::dynData::setData(KNSavedArticle *a)
{
  id=a->id();
  status=(int)a->status();
  so=a->startOffset();
  eo=a->endOffset();
  sId=a->serverId();
  ti=a->timeT();
}

