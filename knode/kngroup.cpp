/*
    kngroup.cpp

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

#include <qfile.h>
#include <qtextstream.h>
#include <mimelib/datetime.h>

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "knserverinfo.h"
#include "knprotocolclient.h"
#include "knglobals.h"
#include "knmime.h"
#include "knstringsplitter.h"
#include "kncollectionviewitem.h"
#include "kngrouppropdlg.h"
#include "knnntpaccount.h"
#include "knaccountmanager.h"
#include "utilities.h"
#include "kngroup.h"
#include "knconfigmanager.h"
#include "knodeview.h"
#include "knode.h"
#include "knscoring.h"

#define SORT_DEPTH 5

KNGroup::KNGroup(KNCollection *p)
  : KNArticleCollection(p), n_ewCount(0), r_eadCount(0),
    l_astNr(0), m_axFetch(0), l_ocked(false), u_seCharset(false),
    s_tatus(unknown), i_dentity(0)
{
}



KNGroup::~KNGroup()
{
  delete i_dentity;
}



QString KNGroup::path()
{
  return p_arent->path();
}



const QString& KNGroup::name()
{
  static QString ret;
  if(n_ame.isEmpty()) ret=g_roupname;
  else ret=n_ame;
  return ret;
}



void KNGroup::updateListItem()
{
  if(!l_istItem) return;
  l_istItem->setNumber(1,c_ount);
  l_istItem->setNumber(2,c_ount-r_eadCount);
}



bool KNGroup::readInfo(const QString &confPath)
{
  KSimpleConfig info(confPath);

  g_roupname = info.readEntry("groupname");
  d_escription = info.readEntry("description");
  n_ame = info.readEntry("name");
  c_ount = info.readNumEntry("count",0);
  r_eadCount = info.readNumEntry("read",0);
  l_astNr = info.readNumEntry("lastMsg",0);
  u_seCharset = info.readBoolEntry("useCharset", false);
  d_efaultChSet = info.readEntry("defaultChSet").latin1();
  QString s = info.readEntry("status","unknown");
  if (s=="readOnly")
    s_tatus = readOnly;
  else if (s=="postingAllowed")
    s_tatus = postingAllowed;
  else if (s=="moderated")
    s_tatus = moderated;
  else
    s_tatus = unknown;

  i_dentity=new KNConfig::Identity(false);
  i_dentity->loadConfig(&info);
  if(!i_dentity->isEmpty()) {
    kdDebug(5003) << "KNGroup::readInfo(const QString &confPath) : using alternative user for " << g_roupname << endl;
  }
  else {
    delete i_dentity;
    i_dentity=0;
  }
  
  return (!g_roupname.isEmpty());
}



void KNGroup::saveInfo()
{
  QString dir(path());
  
  if (dir != QString::null){
    KSimpleConfig info(dir+g_roupname+".grpinfo");
  
    info.writeEntry("groupname", g_roupname);
    info.writeEntry("description", d_escription);
    info.writeEntry("lastMsg", l_astNr);
    info.writeEntry("count", c_ount);
    info.writeEntry("read", r_eadCount);
    info.writeEntry("name", n_ame);
    info.writeEntry("useCharset", u_seCharset);
    info.writeEntry("defaultChSet", QString::fromLatin1(d_efaultChSet));
    switch (s_tatus) {
      case unknown: info.writeEntry("status","unknown");
                    break;
      case readOnly: info.writeEntry("status","readOnly");
                     break;
      case postingAllowed: info.writeEntry("status","postingAllowed");
                           break;
      case moderated: info.writeEntry("status","moderated");
                           break;
    }

    if(i_dentity)
      i_dentity->saveConfig(&info);
    else if(info.hasKey("Email")) {
      info.deleteEntry("Name", false);
      info.deleteEntry("Email", false);
      info.deleteEntry("Reply-To", false);
      info.deleteEntry("Org", false);
      info.deleteEntry("UseSigFile", false);
      info.deleteEntry("UseSigGenerator", false);
      info.deleteEntry("sigFile", false);
      info.deleteEntry("sigText", false);
    }
  }
}



KNNntpAccount* KNGroup::account()
{
  KNCollection *p=parent();
  while(p->type()!=KNCollection::CTnntpAccount) p=p->parent();
  
  return (KNNntpAccount*)p_arent;
}



KNRemoteArticle* KNGroup::byId(int id)
{
  int idx=findId(id);
  if(idx!=-1) return ( static_cast<KNRemoteArticle*> (list[idx]) );
  else return 0;
}



bool KNGroup::loadHdrs()
{
  if(c_ount>0 && len==0) {
    kdDebug(5003) << "KNGroup::loadHdrs() : loading headers" << endl;
    QCString buff;
    KNFile f;
    KNStringSplitter split;
    int cnt=0, id, lines;
    time_t timeT;//, fTimeT;
    KNRemoteArticle *art;

    QString dir(path());
    if (dir == QString::null)
      return false;
      
    f.setName(dir+g_roupname+".static");

    if(f.open(IO_ReadOnly)) {

      if(!resize(c_ount)) {
        f.close();
        return false;
      }

      while(!f.atEnd()) {
        buff=f.readLine();    
        if(buff.isEmpty()){
          if (f.status() == IO_Ok) {
            kdWarning(5003) << "Found broken line in static-file: Ignored!" << endl;
            continue;
          } else {        
            kdError(5003) << "Corrupted static file, IO-error!" << endl;
            clearList();
            return false;
          }
        }
        
        split.init(buff, "\t");

        art=new KNRemoteArticle(this);

        split.first();
        art->messageID()->from7BitString(split.string());
    
        split.next();
        art->subject()->from7BitString(split.string());
        
        split.next();
        art->from()->setEmail(split.string());
        split.next();
        if(split.string()!="0")
          art->from()->setNameFrom7Bit(split.string());

        buff=f.readLine();
        if(buff!="0") art->references()->from7BitString(buff.copy());
                      
        buff=f.readLine();
        sscanf(buff,"%d %d %u", &id, &lines, (uint*) &timeT);
        art->setId(id);
        art->lines()->setNumberOfLines(lines);
        art->date()->setUnixTime(timeT);

        if(append(art)) cnt++;
        else {
          f.close();
          clearList();
          return false;
        }
      }

      setLastID();
      f.close();
    }
    else {
      clearList();
      return false;
    } 
    
  
    f.setName(dir+g_roupname+".dynamic");
  
    if (f.open(IO_ReadOnly)) {
  
      dynData data;
      int readCnt=0,byteCount;
    
      while(!f.atEnd()) {
      
        byteCount = f.readBlock((char*)(&data), sizeof(dynData));
        if ((byteCount == -1)||(byteCount!=sizeof(dynData)))
          if (f.status() == IO_Ok) {
            kdWarning(5003) << "Found broken entry in dynamic-file: Ignored!" << endl;
            continue;
          } else {
            kdError(5003) << "Corrupted dynamic file, IO-error!" << endl;
            clearList();
            return false;
          }   
      
        art=byId(data.id);
      
        if(art) {
          art->setIdRef(data.idRef);
          art->setRead(data.read);
          art->setThreadingLevel(data.thrLevel);
          art->setScore(data.score);
        }
    
        if(data.read) readCnt++;
        
      }
    
      f.close();
      
      r_eadCount=readCnt;
      
    } 
    
    else  {
      clearList();
      return false;
    }
          
    kdDebug(5003) << cnt << " articles read from file" << endl;
    c_ount=len;
    
    updateThreadInfo();
    return true;
  }   
  
  else {
    kdDebug(5003) << "KNGroup::loadHdrs() : nothing to load" << endl;
    return true;
  }
}


// Attention: this method is called from the network thread!
void KNGroup::insortNewHeaders(QStrList *hdrs, KNProtocolClient *client)
{
  KNRemoteArticle *art=0;
  QCString tmp;
  KNStringSplitter split;
  split.setIncludeSep(false);
  int cnt=0,todo=hdrs->count();
  QTime timer;

  timer.start();

  //resize the list 
  if(!resize(siz+hdrs->count())) return;

  for(char *line=hdrs->first(); line; line=hdrs->next()) {
    split.init(line, "\t");
      
    //new Header-Object
    art=new KNRemoteArticle(this);
    art->setNew(true);

    //Article Number
    split.first();
    // ignored hdr->artNr=split.string().toInt();

    //Subject
    split.next();
    art->subject()->from7BitString(split.string());
    if(art->subject()->isEmpty())
      art->subject()->fromUnicodeString(i18n("no subject"), art->defaultCharset());
    
    //From and Email
    split.next();
    art->from()->from7BitString(split.string());
        
    //Date
    split.next();
    art->date()->from7BitString(split.string());
                    
    //Message-ID
    split.next();
    art->messageID()->from7BitString(split.string().simplifyWhiteSpace());
      
    //References
    split.next();
    if(!split.string().isEmpty())
      art->references()->from7BitString(split.string()); //use QCString::copy() ?
    
    //Lines
    split.next();
    split.next();
    art->lines()->setNumberOfLines(split.string().toInt());
        
    if(append(art)) cnt++;
    else {
      delete art;
      return;
    }

    if (timer.elapsed() > 200) {           // don't flicker
      timer.restart();
      if (client) client->updatePercentage((cnt*30)/todo);
    }
  }

  sortHdrs(cnt,client);

  int count = saveStaticData(cnt);
#ifndef NDEBUG
  qDebug("knode: %d headers wrote to file",count);
#endif
  saveDynamicData(cnt);
  updateThreadInfo();
  c_ount=len;
  n_ewCount+=cnt;
  updateListItem();
  saveInfo();
}



int KNGroup::saveStaticData(int cnt,bool ovr)
{
  int idx, savedCnt=0, mode;
  KNRemoteArticle *art;
  
  QString dir(path());
  if (dir == QString::null)
    return 0;
  
  QFile f(dir+g_roupname+".static");
  
  if(ovr) mode=IO_WriteOnly;
  else mode=IO_WriteOnly | IO_Append;
  
  if(f.open(mode)) {
  
    QTextStream ts(&f);
    
    for(idx=len-cnt; idx<len; idx++) {
            
      art=at(idx);    
      
      if(art->subject()->isEmpty()) continue;
    
      ts << art->messageID()->as7BitString(false) << '\t';
      ts << art->subject()->as7BitString(false) << '\t';
      ts << art->from()->email() << '\t';

      if(art->from()->hasName())
        ts << art->from()->nameAs7Bit() << '\n';
      else
        ts << "0\n";
          
      if(!art->references()->isEmpty())
        ts << art->references()->as7BitString(false) << "\n";
      else
        ts << "0\n";
    
      ts << art->id() << ' ';
      ts << art->lines()->numberOfLines() << ' ';
      ts << art->date()->unixTime() << '\n';

    
      savedCnt++;
      
    }
  
    f.close();
  }
  
  return savedCnt;
}



void KNGroup::saveDynamicData(int cnt,bool ovr)
{
  dynData data;
  int mode;
  KNRemoteArticle *art;
  
  if(len>0) {
    QString dir(path());
    if (dir == QString::null)
      return;
    
    QFile f(dir+g_roupname+".dynamic");
    
    if(ovr) mode=IO_WriteOnly;
    else mode=IO_WriteOnly | IO_Append;
    
    if(f.open(mode)) {
        
      for(int idx=len-cnt; idx<len; idx++) {    
        art=at(idx);  
        if(art->subject()->isEmpty()) continue;
        data.setData(art);
        f.writeBlock((char*)(&data), sizeof(dynData));
      }
      f.close();
    }
    else displayInternalFileError();
  }
}



void KNGroup::syncDynamicData()
{
  dynData data;
  int cnt=0, readCnt=0, sOfData;
  KNRemoteArticle *art;
  
  if(len>0) {

    QString dir(path());
    if (dir == QString::null)
      return;
    
    QFile f(dir+g_roupname+".dynamic");
        
    if(f.open(IO_ReadWrite)) {
      
      sOfData=sizeof(dynData);  
    
      for(int i=0; i<len; i++) {
        art=at(i);
        
        if(art->hasChanged() && !art->subject()->isEmpty()) {
          
          data.setData(art);
          f.at(i*sOfData);
          f.writeBlock((char*) &data, sOfData);
          cnt++;
          art->setChanged(false);
        }
      
        if(art->isRead()) readCnt++;
      }
      
      f.close();

      kdDebug(5003) << g_roupname << " => updated " << cnt << " entries of dynamic data" << endl;

      r_eadCount=readCnt;
    }
    else displayInternalFileError();
  }
}


void KNGroup::sortHdrs(int cnt, KNProtocolClient *client)
{
  int end=len,
      start=len-cnt,
      foundCnt_1=0, foundCnt_2=0, bySubCnt=0, refCnt=0,
      resortCnt=0, idx, oldRef; // idRef;
  KNRemoteArticle *art;
  QTime timer;

  timer.start();

  // this method is called from the nntp-thread!!!
#ifndef NDEBUG
  qDebug("knode: KNGroup::sortHdrs() : start = %d  end = %d",start,end);
#endif

  //resort old hdrs
  if(start>0)
    for(idx=0; idx<start; idx++) {
      art=at(idx);
      if(art->threadingLevel()>1) {
        oldRef=art->idRef();
        if(findRef(art, start, end)!=-1) {
          // this method is called from the nntp-thread!!!
          #ifndef NDEBUG
          qDebug("knode: %d: old %d  new %d",art->id(), oldRef, art->idRef());
          #endif
          resortCnt++;
          art->setChanged(true);
        }
      }
    }
    
  
  for(idx=start; idx<end; idx++) {
  
    art=at(idx);
    
    if(art->idRef()==-1 && !art->references()->isEmpty() ){   //hdr has references
      refCnt++;
      
      if(findRef(art, start, end)!=-1)  foundCnt_1++;   //scan new hdrs
      else if(start!=0)                                 //scan old hdrs
        if(findRef(art, 0, start, true)!=-1)
          foundCnt_2++;
    }
    else {
      if(art->subject()->isReply()) {
        art->setIdRef(0); //hdr has no references
        art->setThreadingLevel(0);
      }
      else if(art->idRef()==-1) refCnt++;
    }

    if (timer.elapsed() > 200) {           // don't flicker
      timer.restart();
      if (client) client->updatePercentage(30+((foundCnt_1+foundCnt_2)*70)/cnt);
    }
  }
  
    
  if((foundCnt_1+foundCnt_2)<refCnt) {    // if some references could not be found
  
    //try to sort by subject
    KNRemoteArticle *oldest;
    QList<KNRemoteArticle> list;
    list.setAutoDelete(false);
      
    for(idx=start; idx<end; idx++) {
      
      art=at(idx);      
        
      if(art->idRef()==-1) {  //for all not sorted headers
      
        list.clear();
        list.append(art);
        
        //find all headers with same subject
        for(int idx2=0; idx2<len; idx2++)
          if(at(idx2)==art) continue;
          else if(at(idx2)->subject()==art->subject())
            list.append(at(idx2));
      
        if(list.count()==1) {
          art->setIdRef(0);
          art->setThreadingLevel(6);
          bySubCnt++;
        }
        else {
      
          //find oldest
          oldest=list.first();
          for(KNRemoteArticle *var=list.next(); var; var=list.next())
            if(var->date()->unixTime() < oldest->date()->unixTime()) oldest=var;
          
          //oldest gets idRef 0 
          if(oldest->idRef()==-1) bySubCnt++;
          oldest->setIdRef(0);
          oldest->setThreadingLevel(6);   
          
          for(KNRemoteArticle *var=list.first(); var; var=list.next()) {
            if(var==oldest) continue;
            else if(var->idRef()==-1 || (var->idRef()!=-1 && var->threadingLevel()==6)) {
              var->setIdRef(oldest->id());
              var->setThreadingLevel(6);
              if(var->id() >= at(start)->id()) bySubCnt++;
            }
          }
        }
      }

      if (timer.elapsed() > 200) {           // don't flicker
        timer.restart();
        if (client) client->updatePercentage(30+((bySubCnt+foundCnt_1+foundCnt_2)*70)/cnt);
      }
    } 
  }

  //all not found items get refID 0
  for (int idx=start; idx<end; idx++){
    art=at(idx);
    if(art->idRef()==-1) {
      art->setIdRef(0);
      art->setThreadingLevel(6);   //was 0 !!!
    }
  }
  
#ifdef CHECKLOOPS 
  //check for loops in threads
  int startId;
  bool isLoop;
  for (int idx=start; idx<end; idx++){
    en=hList[idx];
    startId=en->id;
    idRef=en->idRef;
    isLoop=false;
    while(idRef!=0 && !isLoop) {
      en=hList.byID(idRef);
      idRef=en->idRef;
      isLoop=(idRef==startId);
    }
    
    if(isLoop) {
      // this method is called from the nntp-thread!!!
      #ifndef NDEBUG
      qDebug("knode: Sorting : loop in %d",hList[idx]->id);
      #endif
      hList[idx]->idRef=0;
      hList[idx]->thrLevel=0;
    }
      
  }
#endif  
  

  //set score for new Headers
  /*for(int idx=start; idx<end; idx++) {
    art=at(idx);
    idRef=art->idRef();
    
    if(idRef!=0) {
      while(idRef!=0) {
        art=byId(idRef);
        idRef=art->idRef();
      }
      if(art) at(idx)->setScore(art->score());
    }
  }*/
  
  // this method is called from the nntp-thread!!!
#ifndef NDEBUG
  qDebug("knode: Sorting : %d headers resorted", resortCnt);
  qDebug("knode: Sorting : %d references of %d found in step 1", foundCnt_1, refCnt);
  qDebug("knode: Sorting : %d references of %d found in step 2", foundCnt_2, refCnt);
  qDebug("knode: Sorting : %d references of %d sorted by subject", bySubCnt, refCnt);
#endif
    
}


int KNGroup::findRef(KNRemoteArticle *a, int from, int to, bool reverse)
{
  bool found=false;
  int foundID=-1, idx=0;
  short refNr=0;
  QCString ref;
  ref=a->references()->first();
  
  if(!reverse){
    
    while(!found && !ref.isNull() && refNr < SORT_DEPTH) {
      for(idx=from; idx<to; idx++) {
        if(at(idx)->messageID()->as7BitString(false)==ref) {
          found=true;
          foundID=at(idx)->id();
          a->setThreadingLevel(refNr+1);
          break;
        }
      }
      ++refNr;
      ref=a->references()->next();
    }
  }
  else {
    while(!found && !ref.isNull() && refNr < SORT_DEPTH) {
      for(idx=to; idx>=from; idx--) {
        if(at(idx)->messageID()->as7BitString(false)==ref){
          found=true;
          foundID=at(idx)->id();
          a->setThreadingLevel(refNr+1);
          break;
        }
      }
      ++refNr;
      ref=a->references()->next();
    }
  }
      
  if(foundID!=-1) a->setIdRef(foundID);
  return foundID;
}

void KNGroup::scoreArticles(bool onlynew)
{
  kdDebug(5003) << "KNGroup::scoreArticles()" << endl;
  if (onlynew) {
    if (newCount() > 0) {
      int cnt = newCount();
      kdDebug(5003) << "scoring " << newCount() << " articles" << endl;
      kdDebug(5003) << "(total " << len << " article in group)" << endl;
      knGlobals.top->setCursorBusy(true);
      knGlobals.top->setStatusMsg(i18n("Scoring..."));
      KScoringManager *sm = knGlobals.scoreManager;
      sm->initCache(name());
      for(int idx=0; idx<newCount(); idx++) {
	KNRemoteArticle *a = at(len-idx-1);
	ASSERT( a );
	KNScorableArticle sa(a);
	sm->applyRules( sa );
	if (idx % 10 == 0 ) {
	  kdDebug(5003) << "still " << cnt-idx << " articles to score..." << endl;
	}
      }
      knGlobals.top->setStatusMsg(QString::null);
      knGlobals.top->setCursorBusy(false);
    }
  }
}

void KNGroup::reorganize()
{
  kdDebug(5003) << "KNGroup::reorganize()" << endl;

  knGlobals.top->setCursorBusy(true);
  knGlobals.top->setStatusMsg(i18n("Reorganizing headers..."));

  KScoringManager *sm = knGlobals.scoreManager;
  sm->initCache(name());
  for(int idx=0; idx<len; idx++) {
    KNRemoteArticle *a = at(idx);
    ASSERT( a );
    a->setScore(50);
    KNScorableArticle sa(a);
    sm->applyRules( sa );
    if (a->score() != 50) {
      kdDebug(5003) << "score of Article " << idx << " changed" << endl;
    }
    a->setId(idx+1); //new ids
    a->setIdRef(-1);
    a->setThreadingLevel(0);
  }

  sortHdrs(len);
  saveStaticData(len, true);
  saveDynamicData(len, true);
  knGlobals.view->headerView()->repaint();
  knGlobals.top->setStatusMsg(QString::null);
  knGlobals.top->setCursorBusy(false);
}


void KNGroup::updateThreadInfo()
{
  KNRemoteArticle *ref;
  bool brokenThread=false;
  
  for(int idx=0; idx<len; idx++) {
    at(idx)->setUnreadFollowUps(0);
    at(idx)->setNewFollowUps(0);
  }
    
  for(int idx=0; idx<len; idx++) {
    int idRef=at(idx)->idRef();
    while(idRef!=0) {
      ref=byId(idRef);
      if(!ref) {
        brokenThread=true;
        break;
      }
      
      if(!at(idx)->isRead())  {
        ref->incUnreadFollowUps();
        if(at(idx)->isNew()) ref->incNewFollowUps();
      }
      idRef=ref->idRef();
    }
    if(brokenThread) break;
  }

  if(brokenThread) {
    kdWarning(5003) << "KNGroup::updateThreadInfo() : Found broken threading infos! Restoring ..." << endl;
    reorganize();
    updateThreadInfo();
  }
}




KNRemoteArticle* KNGroup::byMessageId(const QCString &mId)
{
  KNRemoteArticle *ret=0;
  
  for(int i=0; i<len; i++) {
    if(at(i)->messageID()->as7BitString(false)==mId) {
      ret=at(i);
      break;
    }
  }
  return ret;   
}



void KNGroup::showProperties()
{
  if(!i_dentity) i_dentity=new KNConfig::Identity(false);
  KNGroupPropDlg *d=new KNGroupPropDlg(this, knGlobals.topWidget);
  
  if(d->exec())
    if(d->nickHasChanged())
      l_istItem->setText(0, name());

  if(i_dentity->isEmpty()) {
    delete i_dentity;
    i_dentity=0;
  }

  delete d;
}



int KNGroup::statThrWithNew()
{
  int cnt=0;
  for(int i=0; i<len; i++)
    if( (at(i)->idRef()==0) && (at(i)->hasNewFollowUps()) ) cnt++;
  return cnt;
}



int KNGroup::statThrWithUnread()
{
  int cnt=0;
  for(int i=0; i<len; i++)
    if( (at(i)->idRef()==0) && (at(i)->hasUnreadFollowUps()) ) cnt++;
  return cnt;
}



void KNGroup::dynData::setData(KNRemoteArticle *a)
{
  id=a->id();
  idRef=a->idRef();
  thrLevel=a->threadingLevel();
  read=a->isRead();
  score=a->score(); 
}




