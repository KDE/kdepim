/***************************************************************************
                          kngroup.cpp  -  description
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

#include <qfile.h>
#include <qtextstream.h>
#include <mimelib/datetime.h>

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "knglobals.h"
#include "knuserentry.h"
#include "knfetcharticle.h"
#include "knstringsplitter.h"
#include "kncollectionviewitem.h"
#include "kngrouppropdlg.h"
#include "knnntpaccount.h"
#include "utilities.h"
#include "kngroup.h"


#define SORT_DEPTH 5

KNGroup::KNGroup(KNCollection *p)
  : KNArticleCollection(p), n_ewCount(0), r_eadCount(0),
    l_astNr(0), m_axFetch(0), u_ser(0), l_ocked(false)
{
}



KNGroup::~KNGroup()
{
  delete u_ser;
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

  /*if(tmp.isEmpty()) {
    tmp=info.deleteEntry("name", false);
    info.writeEntry("groupname", tmp);
    kdDebug(5003) << "Group info-file converted" << endl;
    info.sync();
  }*/
  
  g_roupname = info.readEntry("groupname").local8Bit();
  d_escription = info.readEntry("description").local8Bit();
  n_ame = info.readEntry("name");
  c_ount = info.readNumEntry("count",0);
  r_eadCount = info.readNumEntry("read",0);
  l_astNr = info.readNumEntry("lastMsg",0);

  u_ser=new KNUserEntry();
  u_ser->load(&info);
  if(!u_ser->isEmpty()) {
    kdDebug(5003) << "using alternative user for " << g_roupname.data() << endl;
  } else {
    delete u_ser;
    u_ser=0;
  }
  
  return (!g_roupname.isEmpty());
}



void KNGroup::saveInfo()
{
  QString dir(path());
  
  if (dir != QString::null){
    KSimpleConfig info(dir+QString(g_roupname)+".grpinfo");
  
    info.writeEntry("groupname", QString(g_roupname));
    info.writeEntry("description", QString(d_escription));
    info.writeEntry("lastMsg", l_astNr);
    info.writeEntry("count", c_ount);
    info.writeEntry("read", r_eadCount);
    info.writeEntry("name", n_ame);
  
    if(u_ser) u_ser->save(&info);
    else if(info.hasKey("Email")) {
      info.deleteEntry("Name", false);
      info.deleteEntry("Email", false);
      info.deleteEntry("Reply-To", false);
      info.deleteEntry("Org", false);
      info.deleteEntry("sigFile", false);
    }
  }
}



KNNntpAccount* KNGroup::account()
{
  KNCollection *p=parent();
  while(p->type()!=KNCollection::CTnntpAccount) p=p->parent();
  
  return (KNNntpAccount*)p_arent;
}



KNFetchArticle* KNGroup::byId(int id)
{
  int idx=findId(id);
  if(idx!=-1) return ((KNFetchArticle*)list[idx]);
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
    KNFetchArticle *art;

    QString dir(path());
    if (dir == QString::null)
      return false;
      
    f.setName(dir+QString(g_roupname)+".static");
      
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
        art=new KNFetchArticle();
        split.first();
        art->setMessageId(split.string());
    
        split.next();
        art->setSubject(split.string());
        
        split.next();
        art->setFromEmail(split.string());
    
        split.next();
        if(split.string()!="0") art->setFromName(split.string());
        else {
          art->setFromName(art->fromEmail());
        }
          
        /*split.init(f.readLine(), ";");
      
        bool isRef=split.first(); 
    
        if(isRef){
                
          int RefNr=0;
        
          while(isRef && RefNr<5) {
            art->setReference(RefNr, split.string());
            isRef=split.next();
            RefNr++;
          }
        }*/
        
        buff=f.readLine();
        if(!buff.isEmpty()) art->references().setLine(buff.copy());
                      
        buff=f.readLine();
        sscanf(buff,"%d %d %d", &id, &lines, (uint*) &timeT);//, (uint*) &fTimeT);
        //kdDebug(5003) << "id = " << id << endl;
      
        art->setId(id);
        art->setLines(lines);
        art->setTimeT(timeT);
        //art->setFetchTime(fTimeT);
                
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
    
  
    f.setName(dir+QString(g_roupname)+".dynamic");
  
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



void KNGroup::insortNewHeaders(QStrList *hdrs)
{
  KNFetchArticle *art=0;
  QCString tmp;
  //DwDateTime dt;
  //time_t fTimeT=dt.AsUnixTime();
  KNStringSplitter split;
  split.setIncludeSep(false);
  int cnt=0;
  
  //resize the list 
  if(!resize(siz+hdrs->count())) return;
  
  for(char *line=hdrs->first(); line; line=hdrs->next()) {
    split.init(line, "\t");
      
    //new Header-Object
    art=new KNFetchArticle();
    art->setNew(true);
    //art->setFetchTime(fTimeT);
        
    //Article Number
    split.first();
    // ignored hdr->artNr=split.string().toInt();
    
    //Subject
    split.next();
    art->setSubject(split.string());
    if(art->subject().isEmpty()) art->setSubject(i18n("no subject").local8Bit());
    
    //From and Email
    split.next();
    art->parseFrom(split.string());
        
    //Date
    split.next();
    art->parseDate(split.string());
                    
    //Message-ID
    split.next();
    art->setMessageId(split.string().simplifyWhiteSpace());
      
    //References
    split.next();
    if(!split.string().isEmpty())
      art->references().setLine(split.string().copy());
    
    //Lines
    split.next();
    split.next();
    art->setLines(split.string().toInt());      
        
    if(append(art)) cnt++;
    else {
      delete art;
      return;
    }
  }
  
  sortHdrs(cnt);
  int count = saveStaticData(cnt);
  kdDebug(5003) << count << " headers wrote to file" << endl;
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
  KNFetchArticle *art;
  
  QString dir(path());
  if (dir == QString::null)
    return 0;
  
  QFile f(dir+QString(g_roupname)+".static");
  
  if(ovr) mode=IO_WriteOnly;
  else mode=IO_WriteOnly | IO_Append;
  
  if(f.open(mode)) {
  
    QTextStream ts(&f);
    
    for(idx=len-cnt; idx<len; idx++) {
            
      art=at(idx);    
      
      if(!art->hasData()) continue;
    
      ts << art->messageId() << '\t';
      ts << art->subject() << '\t';
      ts << art->fromEmail() << '\t';

      if(art->fromEmail().nrefs()==1)
        ts << art->fromName() << '\n';
      else
        ts << "0\n";
          
      if(art->hasReferences())
        ts << art->references().line() << "\n";
      else
        ts << "0\n";
    
      ts << art->id() << ' ';
      ts << art->lines() << ' ';
      ts << art->timeT() << '\n';
      //ts << art->fetchTime() << '\n';
    
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
  KNFetchArticle *art;  
  
  if(len>0) {
    QString dir(path());
    if (dir == QString::null)
      return;
    
    QFile f(dir+QString(g_roupname)+".dynamic");      
    
    if(ovr) mode=IO_WriteOnly;
    else mode=IO_WriteOnly | IO_Append;
    
    if(f.open(mode)) {
        
      for(int idx=len-cnt; idx<len; idx++) {    
        art=at(idx);  
        if(!art->hasData()) continue;
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
  KNFetchArticle *art;
  
  if(len>0) {

    QString dir(path());
    if (dir == QString::null)
      return;
    
    QFile f(dir+QString(g_roupname)+".dynamic");          
        
    if(f.open(IO_ReadWrite)) {
      
      sOfData=sizeof(dynData);  
    
      for(int i=0; i<len; i++) {
        art=at(i);
        
        if(art->hasChanged() && art->hasData()) {
          
          data.setData(art);
          f.at(i*sOfData);
          f.writeBlock((char*) &data, sOfData);
          cnt++;
          art->setHasChanged(false);
        }
      
        if(art->isRead()) readCnt++;
      }
      
      f.close();

      kdDebug(5003) << n_ame << " => updated " << cnt << " entries of dynamic data" << endl;

      r_eadCount=readCnt;
    }
    else displayInternalFileError();
  }
}



void KNGroup::sortHdrs(int cnt)
{
  int end=len,
      start=len-cnt,
      foundCnt_1=0, foundCnt_2=0, bySubCnt=0, refCnt=0,
      resortCnt=0, idx, oldRef, idRef;
  KNFetchArticle *art;
  
  kdDebug(5003) << "KNGroup::sortHdrs() : start = " << start << "   end = " << end << endl;
  
  //resort old hdrs
  if(start>0)
    for(idx=0; idx<start; idx++) {
      art=at(idx);
      if(art->threadingLevel()>1) {
        oldRef=art->idRef();
        if(findRef(art, start, end)!=-1) {
          kdDebug(5003) << art->id() << " : old " << oldRef << "    new " << art->idRef() << "\n" << endl;
          resortCnt++;
          art->setHasChanged(true);
        }
      }
    }
    
  
  for(idx=start; idx<end; idx++) {
  
    art=at(idx);
    
    if(art->hasReferences() && art->idRef()==-1){   //hdr has references
      refCnt++;
      
      if(findRef(art, start, end)!=-1)  foundCnt_1++;   //scan new hdrs
      else if(start!=0)                                 //scan old hdrs
        if(findRef(art, 0, start, true)!=-1)
          foundCnt_2++;
    }
    else {
      if(strncasecmp(art->subject(),"Re:",3)!=0) {
        art->setIdRef(0); //hdr has no references
        art->setThreadingLevel(0);
      }
      else if(art->idRef()==-1) refCnt++;
    }
  }
  
    
  if((foundCnt_1+foundCnt_2)<refCnt) {    // if some references could not be found
  
    //try to sort by subject
    KNFetchArticle *oldest; 
    QList<KNFetchArticle> list;
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
          for(KNFetchArticle *var=list.next(); var; var=list.next())
            if(var->timeT() < oldest->timeT()) oldest=var;
          
          //oldest gets idRef 0 
          if(oldest->idRef()==-1) bySubCnt++;
          oldest->setIdRef(0);
          oldest->setThreadingLevel(6);   
          
          for(KNFetchArticle *var=list.first(); var; var=list.next()) {
            if(var==oldest) continue;
            else if(var->idRef()==-1 || (var->idRef()!=-1 && var->threadingLevel()==6)) {
              var->setIdRef(oldest->id());
              var->setThreadingLevel(6);
              if(var->id() >= at(start)->id()) bySubCnt++;
            }
          }
        }
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
      kdDebug(5003) << "Sorting : loop in " << hList[idx]->id << endl;
      hList[idx]->idRef=0;
      hList[idx]->thrLevel=0;
    }
      
  }
#endif  
  

  //set score for new Headers
  for(int idx=start; idx<end; idx++) {
    art=at(idx);
    idRef=art->idRef();
    
    if(idRef!=0) {
      while(idRef!=0) {
        art=byId(idRef);
        idRef=art->idRef();
      }
      if(art) at(idx)->setScore(art->score());
    }
  }
  
  
  kdDebug(5003) << "Sorting : " << resortCnt << " headers resorted" << endl;
  kdDebug(5003) << "Sorting : " << foundCnt_1 << " references of " << refCnt << " found in step 1" << endl;
  kdDebug(5003) << "Sorting : " << foundCnt_2 << " references of " << refCnt << " found in step 2" << endl;
  kdDebug(5003) << "Sorting : " << bySubCnt << " references of " << refCnt << " sorted by subject" << endl;
    
}



int KNGroup::findRef(KNFetchArticle *a, int from, int to, bool reverse)
{
  bool found=false;
  int foundID=-1, idx=0;
  short refNr=0;
  QCString ref;
  ref=a->references().first();  
  
  if(!reverse){
    
    while(!found && !ref.isNull() && refNr < SORT_DEPTH) {
      for(idx=from; idx<to; idx++) {
        if(at(idx)->messageId()==ref) {
          found=true;
          foundID=at(idx)->id();
          a->setThreadingLevel(refNr+1);
          break;
        }
      }
      ++refNr;
      ref=a->references().next();
    }
  }
  else {
    while(!found && !ref.isNull() && refNr < SORT_DEPTH) {
      for(idx=to; idx>=from; idx--) {
        if(at(idx)->messageId()==ref){
          found=true;
          foundID=at(idx)->id();
          a->setThreadingLevel(refNr+1);
          break;
        }
      }
      ++refNr;
      ref=a->references().next();
    }
  }
      
  if(foundID!=-1) a->setIdRef(foundID);
  return foundID;
}



void KNGroup::resort()
{
  for(int idx=0; idx<len; idx++) {
    at(idx)->setIdRef(-1);
    at(idx)->setThreadingLevel(0);
  }
  kdDebug(5003) << "KNGroup::resort()" << endl;
  sortHdrs(len);
  saveDynamicData(len, true);
}



void KNGroup::updateThreadInfo()
{
  KNFetchArticle *ref;
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
    kdWarning(5003) << "KNGroup::updateThreadInfo() : Found broken threading infos !! Restoring ..." << endl;
    resort();
    updateThreadInfo();
  }
}




KNFetchArticle* KNGroup::byMessageId(const QCString &mId)
{
  KNFetchArticle *ret=0;
  
  for(int i=0; i<len; i++) {
    if(at(i)->messageId()==mId) {
      ret=at(i);
      break;
    }
  }
  return ret;   
}



void KNGroup::showProperties()
{
  if(!u_ser) u_ser=new KNUserEntry();
  KNGroupPropDlg *d=new KNGroupPropDlg(this, knGlobals.topWidget);
  
  if(d->exec()) {
    if(d->nickHasChanged()) {
      KNCollectionViewItem *p=p_arent->listItem();
      QListView *v=l_istItem->listView();
      QPixmap pm=*(l_istItem->pixmap(0));
      bool sel=l_istItem->isSelected();
    
      delete l_istItem;
      l_istItem=new KNCollectionViewItem(p);
      l_istItem->coll=this;
    
      l_istItem->setPixmap(0, pm);
      l_istItem->setText(0, name());
      
      updateListItem();
      v->setSelected(l_istItem, sel);
    }
  }
  
  if(u_ser->isEmpty()) {
    delete u_ser;
    u_ser=0;
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



void KNGroup::dynData::setData(KNFetchArticle *a)
{
  id=a->id();
  idRef=a->idRef();
  thrLevel=a->threadingLevel();
  read=a->isRead();
  score=a->score(); 
}




