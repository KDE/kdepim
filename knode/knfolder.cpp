/*
    knfolder.cpp

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

#include <qfileinfo.h>

#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>

#include <kqcstringsplitter.h>

#include "knarticlemanager.h"
#include "kncollectionviewitem.h"
#include "knhdrviewitem.h"
#include "utilities.h"
#include "knglobals.h"
#include "knarticlefactory.h"
#include "knfolder.h"
#include "knarticlewidget.h"
#include "knarticlewindow.h"
#include "knmainwidget.h"


KNFolder::KNFolder()
  : KNArticleCollection(0), i_d(-1), p_arentId(-1), i_ndexDirty(false), w_asOpen(true)
{
}


KNFolder::KNFolder(int id, const QString &name, KNFolder *parent)
  : KNArticleCollection(parent), i_d(id), i_ndexDirty(false), w_asOpen(true)
{
  QString fname=path()+QString("custom_%1").arg(i_d);

  n_ame = name;
  m_boxFile.setName(fname+".mbox");
  i_ndexFile.setName(fname+".idx");
  i_nfoPath=fname+".info";

  p_arentId=parent?parent->id():-1;

  if(i_ndexFile.exists())
    c_ount=i_ndexFile.size()/sizeof(DynData);
  else
    c_ount=0;
}


KNFolder::KNFolder(int id, const QString &name, const QString &prefix, KNFolder *parent)
  : KNArticleCollection(parent), i_d(id), i_ndexDirty(false), w_asOpen(true)
{
  QString fname=path()+QString("%1_%2").arg(prefix).arg(i_d);

  n_ame = name;
  m_boxFile.setName(fname+".mbox");
  i_ndexFile.setName(fname+".idx");
  i_nfoPath=fname+".info";

  p_arentId=parent?parent->id():-1;

  if(i_ndexFile.exists())
    c_ount=i_ndexFile.size()/sizeof(DynData);
  else
    c_ount=0;
}


KNFolder::~KNFolder()
{
  closeFiles();
}


void KNFolder::updateListItem()
{
  if(l_istItem) {
    l_istItem->setText(0, n_ame);
    if (!isRootFolder())
      l_istItem->setTotalCount( c_ount );
  }
}


QString KNFolder::path()
{
  QString dir(locateLocal("data","knode/")+"folders/");
  /*if (dir.isNull())
    KNHelper::displayInternalFileError();*/
  return dir;
}


bool KNFolder::readInfo(const QString &infoPath)
{
  if(infoPath.isEmpty())
    return false;

  i_nfoPath=infoPath;

  KSimpleConfig info(i_nfoPath);
  if (!isRootFolder() && !isStandardFolder()) {
    n_ame=info.readEntry("name");
    i_d=info.readNumEntry("id", -1);
    p_arentId=info.readNumEntry("parentId", -1);
  }
  w_asOpen=info.readBoolEntry("wasOpen", true);

  if(i_d>-1) {
    QFileInfo fi(infoPath);
    QString fname=fi.dirPath(true)+"/"+fi.baseName();
    closeFiles();
    clear();

    m_boxFile.setName(fname+".mbox");
    i_ndexFile.setName(fname+".idx");
    c_ount=i_ndexFile.exists() ? (i_ndexFile.size()/sizeof(DynData)) : 0;
  }

  return (i_d!=-1);
}


bool KNFolder::readInfo()
{
  return readInfo(i_nfoPath);
}


void KNFolder::saveInfo()
{
  if(!i_nfoPath.isEmpty()) {
    KSimpleConfig info(i_nfoPath);
    if (!isRootFolder() && !isStandardFolder()) {
      info.writeEntry("name", n_ame);
      info.writeEntry("id", i_d);
      info.writeEntry("parentId", p_arentId);
    }
    if(l_istItem)
      info.writeEntry("wasOpen", l_istItem->isOpen());
  }
}


void KNFolder::setParent(KNCollection *p)
{
  p_arent = p;
  p_arentId = p ? (static_cast<KNFolder*>(p))->id() : -1;
}


bool KNFolder::loadHdrs()
{
  if(isLoaded()) {
    kdDebug(5003) << "KNFolder::loadHdrs() : already loaded" << endl;
    return true;
  }

  if(!i_ndexFile.open(IO_ReadOnly)) {
    kdError(5003) << "KNFolder::loadHdrs() : cannot open index-file!" << endl;
    closeFiles();
    return false;
  }

  if(!m_boxFile.open(IO_ReadOnly)) {
    kdError(5003) << "KNFolder::loadHdrs() : cannot open mbox-file!" << endl;
    closeFiles();
    return false;
  }

  if(!resize(c_ount)) {
    closeFiles();
    return false;
  }

  QCString tmp;
  KQCStringSplitter split;
  KNLocalArticle *art;
  DynData dynamic;
  int pos1=0, pos2=0, cnt=0, byteCount;

  knGlobals.top->setCursorBusy(true);
  knGlobals.setStatusMsg(i18n(" Loading folder..."));
  knGlobals.top->secureProcessEvents();

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
        clear();
        return false;
      }

    art=new KNLocalArticle(this);

    //set index-data
    dynamic.getData(art);

    //read overview
    if(!m_boxFile.at(art->startOffset())) {
      kdError(5003) << "KNFolder::loadHdrs() : cannot set mbox file-pointer!" << endl;
      closeFiles();
      clear();
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
        kdError(5003) << "KNFolder::loadHdrs() : corrupted mbox-file, IO-error!"<< endl;
        closeFiles();
        clear();
        return false;
      }
    }

    //set overview
    bool end=false;
    pos1=tmp.find(' ')+1;
    pos2=tmp.find('\t', pos1);
    if (pos2 == -1) {
      pos2=tmp.length();
      end=true;
    }
    art->subject()->from7BitString(tmp.mid(pos1, pos2-pos1));

    if (!end) {
      pos1=pos2+1;
      pos2=tmp.find('\t', pos1);
      if (pos2 == -1) {
        pos2=tmp.length();
        end=true;
      }
      art->newsgroups()->from7BitString(tmp.mid(pos1, pos2-pos1));
    }

    if (!end) {
      pos1=pos2+1;
      pos2=tmp.find('\t', pos1);
      if (pos2 == -1) {
        pos2=tmp.length();
        end=true;
      }
      art->to()->from7BitString(tmp.mid(pos1,pos2-pos1));
    }

    if (!end) {
      pos1=pos2+1;
      pos2=tmp.length();
      art->lines()->from7BitString(tmp.mid(pos1,pos2-pos1));
    }

    if(!append(art)) {
      kdError(5003) << "KNFolder::loadHdrs() : cannot append article!"<< endl;
      delete art;
      clear();
      closeFiles();

      knGlobals.setStatusMsg(QString::null);
      knGlobals.top->setCursorBusy(false);
      return false;
    }

    cnt++;
  }

  closeFiles();
  setLastID();
  c_ount=cnt;
  updateListItem();

  knGlobals.setStatusMsg(QString::null);
  knGlobals.top->setCursorBusy(false);

  return true;
}


bool KNFolder::unloadHdrs(bool force)
{
  if(l_ockedArticles>0)
    return false;

  if (!force && isNotUnloadable())
    return false;

  KNLocalArticle *a;
  for(int idx=0; idx<length(); idx++) {
    a=at(idx);
    if (a->hasContent() && !knGlobals.articleManager()->unloadArticle(a, force))
      return false;
  }
  syncIndex();
  clear();

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
    kdError(5003) << "KNFolder::loadArticle(KNLocalArticle *a) : cannot set mbox file-pointer!" << endl;
    closeFiles();
    return false;
  }

  //read content
  m_boxFile.readLine(); //skip X-KNode-Overview

  unsigned int size=a->endOffset()-m_boxFile.at()-1;
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
  if(!isLoaded())  // loading should not be done here - keep the StorageManager in sync !!
    return false;

  if(!m_boxFile.open(IO_WriteOnly | IO_Append)) {
    kdError(5003) << "KNFolder::saveArticles() : cannot open mbox-file!" << endl;
    closeFiles();
    return false;
  }

  int addCnt=0;
  bool ret=true;
  bool clear=false;
  QTextStream ts(&m_boxFile);
  ts.setEncoding(QTextStream::Latin1);

  for(KNLocalArticle *a=l->first(); a; a=l->next()) {

    clear=false;
    if(a->id()==-1 || a->collection()!=this) {
      if(a->id()!=-1) {
        KNFolder *oldFolder=static_cast<KNFolder*>(a->collection());
        if(!a->hasContent())
          if( !( clear=oldFolder->loadArticle(a) ) ) {
            ret=false;
            continue;
          }

        KNLocalArticle::List l;
        l.append(a);
        oldFolder->removeArticles(&l, false);
      }
      if(!append(a)) {
        kdError(5003) << "KNFolder::saveArticle(KNLocalArticle::List *l) : cannot append article!" << endl;
        ret=false;
        continue;
        a->setCollection(0);
      }
      else {
        a->setCollection(this);
        addCnt++;
      }
    }

    if(byId(a->id())==a) {

      //MBox
      ts << "From aaa@aaa Mon Jan 01 00:00:00 1997\n";
      a->setStartOffset(m_boxFile.at()); //save offset

      //write overview information
      ts << "X-KNode-Overview: ";
      ts << a->subject()->as7BitString(false) << '\t';

      KMime::Headers::Base* h;
      if( (h=a->newsgroups(false))!=0 )
        ts << h->as7BitString(false);
      ts << '\t';

      if( (h=a->to(false))!=0 )
        ts << h->as7BitString(false);
      ts << '\t';

      ts << a->lines()->as7BitString(false) << '\n';

      //write article
      a->toStream(ts);
      ts << "\n";

      a->setEndOffset(m_boxFile.at()); //save offset

      //update
      KNArticleWidget::articleChanged(a);
      i_ndexDirty=true;

    }
    else {
      kdError(5003) << "KNFolder::saveArticle() : article not in folder!" << endl;
      ret=false;
    }

    if(clear)
      a->KMime::Content::clear();
  }

  closeFiles();
  syncIndex();

  if(addCnt>0) {
    c_ount=length();
    updateListItem();
    knGlobals.articleManager()->updateViewForCollection(this);
  }

  return ret;
}


void KNFolder::removeArticles(KNLocalArticle::List *l, bool del)
{
  if(!isLoaded() || l->isEmpty())
    return;

  int idx=0, delCnt=0, *positions;
  positions=new int[l->count()];
  KNLocalArticle *a=0;

  for(a=l->first(); a; a=l->next()) {
    idx=l->at();
    if(a->isLocked())
      positions[idx]=-1;
    else
      positions[idx]=a_rticles.indexForId(a->id());
  }

  for(idx=0; idx < (int)(l->count()); idx++) {
    if(positions[idx]==-1)
      continue;

    a=at(positions[idx]);

    //update
    knGlobals.artFactory->deleteComposerForArticle(a);
    KNArticleWindow::closeAllWindowsForArticle(a);
    KNArticleWidget::articleRemoved(a);
    delete a->listItem();

    //delete article
    a_rticles.remove(positions[idx], del, false);
    delCnt++;
    if(!del)
      a->setId(-1);
  }

  if(delCnt>0) {
    compact();
    c_ount-=delCnt;
    updateListItem();
    i_ndexDirty=true;
  }
  delete positions;
}


void KNFolder::deleteAll()
{
  if(l_ockedArticles>0)
    return;

  if (!unloadHdrs(true))
    return;

  clear();
  c_ount=0;
  syncIndex(true);
  updateListItem();
}


void KNFolder::deleteFiles()
{
  m_boxFile.remove();
  i_ndexFile.remove();
  QFile::remove(i_nfoPath);
}


void KNFolder::syncIndex(bool force)
{
  if(!i_ndexDirty && !force)
    return;

  if(!i_ndexFile.open(IO_WriteOnly)) {
    kdError(5003) << "KNFolder::syncIndex(bool force) : cannot open index-file!" << endl;
    closeFiles();
    return;
  }

  KNLocalArticle *a;
  DynData d;
  for(int idx=0; idx<length(); idx++) {
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


void KNFolder::DynData::getData(KNLocalArticle *a)
{
  a->setId(id);
  a->date()->setUnixTime(ti);
  a->setStartOffset(so);
  a->setEndOffset(eo);
  a->setServerId(sId);
  a->setDoMail(flags[0]);
  a->setMailed(flags[1]);
  a->setDoPost(flags[2]);
  a->setPosted(flags[3]);
  a->setCanceled(flags[4]);
  a->setEditDisabled(flags[5]);
}

