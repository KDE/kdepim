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


#include <ksimpleconfig.h>
#include <klocale.h>
#include <kdebug.h>

#include <kqcstringsplitter.h>

#include "knprotocolclient.h"
#include "knglobals.h"
#include "kncollectionviewitem.h"
#include "kngrouppropdlg.h"
#include "utilities.h"
#include "knconfigmanager.h"
#include "knmainwidget.h"
#include "knscoring.h"
#include "knarticlemanager.h"
#include "kngroupmanager.h"
#include "knnntpaccount.h"
#include "headerview.h"


#define SORT_DEPTH 5

KNGroup::KNGroup(KNCollection *p)
  : KNArticleCollection(p), n_ewCount(0), l_astFetchCount(0), r_eadCount(0), i_gnoreCount(0),
    l_astNr(0), m_axFetch(0), d_ynDataFormat(1), f_irstNew(-1), l_ocked(false),
    u_seCharset(false), s_tatus(unknown), i_dentity(0)
{
  mCleanupConf = new KNConfig::Cleanup( false );
}


KNGroup::~KNGroup()
{
  delete i_dentity;
  delete mCleanupConf;
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
  l_istItem->setTotalCount( c_ount );
  l_istItem->setUnreadCount( c_ount - r_eadCount - i_gnoreCount );
}


bool KNGroup::readInfo(const QString &confPath)
{
  KSimpleConfig info(confPath);

  g_roupname = info.readEntry("groupname");
  d_escription = info.readEntry("description");
  n_ame = info.readEntry("name");
  c_ount = info.readNumEntry("count",0);
  r_eadCount = info.readNumEntry("read",0);
  if (r_eadCount > c_ount) r_eadCount = c_ount;
  f_irstNr = info.readNumEntry("firstMsg",0);
  l_astNr = info.readNumEntry("lastMsg",0);
  d_ynDataFormat = info.readNumEntry("dynDataFormat",0);
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
  c_rosspostIDBuffer = info.readListEntry("crosspostIDBuffer");

  i_dentity=new KNConfig::Identity(false);
  i_dentity->loadConfig(&info);
  if(!i_dentity->isEmpty()) {
    kdDebug(5003) << "KNGroup::readInfo(const QString &confPath) : using alternative user for " << g_roupname << endl;
  }
  else {
    delete i_dentity;
    i_dentity=0;
  }

  mCleanupConf->loadConfig( &info );

  return (!g_roupname.isEmpty());
}


void KNGroup::saveInfo()
{
  QString dir(path());

  if (!dir.isNull()) {
    KSimpleConfig info(dir+g_roupname+".grpinfo");

    info.writeEntry("groupname", g_roupname);
    info.writeEntry("description", d_escription);
    info.writeEntry("firstMsg", f_irstNr);
    info.writeEntry("lastMsg", l_astNr);
    info.writeEntry("count", c_ount);
    info.writeEntry("read", r_eadCount);
    info.writeEntry("dynDataFormat", d_ynDataFormat);
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
    info.writeEntry("crosspostIDBuffer", c_rosspostIDBuffer);

    if(i_dentity)
      i_dentity->saveConfig(&info);
    else if(info.hasKey("Email")) {
      info.deleteEntry("Name", false);
      info.deleteEntry("Email", false);
      info.deleteEntry("Reply-To", false);
      info.deleteEntry("Mail-Copies-To", false);
      info.deleteEntry("Org", false);
      info.deleteEntry("UseSigFile", false);
      info.deleteEntry("UseSigGenerator", false);
      info.deleteEntry("sigFile", false);
      info.deleteEntry("sigText", false);
    }

    mCleanupConf->saveConfig( &info );
  }
}


KNNntpAccount* KNGroup::account()
{
  KNCollection *p=parent();
  while(p->type()!=KNCollection::CTnntpAccount) p=p->parent();

  return (KNNntpAccount*)p_arent;
}


bool KNGroup::loadHdrs()
{
  if(isLoaded()) {
    kdDebug(5003) << "KNGroup::loadHdrs() : nothing to load" << endl;
    return true;
  }

  kdDebug(5003) << "KNGroup::loadHdrs() : loading headers" << endl;
  QCString buff, hdrValue;
  KNFile f;
  KQCStringSplitter split;
  int cnt=0, id, lines, fileFormatVersion, artNumber;
  time_t timeT;//, fTimeT;
  KNRemoteArticle *art;

  QString dir(path());
  if (dir.isNull())
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
          clear();
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
      if (sscanf(buff,"%d %d %u %d", &id, &lines, (uint*) &timeT, &fileFormatVersion) < 4)
        fileFormatVersion = 0;          // KNode <= 0.4 had no version number
      art->setId(id);
      art->lines()->setNumberOfLines(lines);
      art->date()->setUnixTime(timeT);

      if (fileFormatVersion > 0) {
        buff=f.readLine();
        sscanf(buff,"%d", &artNumber);
        art->setArticleNumber(artNumber);
      }

      // optional headers
      if (fileFormatVersion > 1) {
        // first line is the number of addiotion headers
        buff = f.readLine();
        // following lines contain one header per line
        for (uint i = buff.toUInt(); i > 0; --i) {
          buff = f.readLine();
          int pos = buff.find(':');
          hdrValue = buff.right( buff.length() - (pos + 2) );
          if (hdrValue.length() > 0)
            art->setHeader( new KMime::Headers::Generic( buff.left(pos), art, hdrValue ) );
        }
      }

      if(append(art)) cnt++;
      else {
        f.close();
        clear();
        return false;
      }
    }

    setLastID();
    f.close();
  }
  else {
    clear();
    return false;
  }


  f.setName(dir+g_roupname+".dynamic");

  if (f.open(IO_ReadOnly)) {

    dynDataVer0 data0;
    dynDataVer1 data1;
    int readCnt=0,byteCount,dataSize;
    if (d_ynDataFormat==0)
      dataSize = sizeof(data0);
    else
      dataSize = sizeof(data1);

    while(!f.atEnd()) {

      if (d_ynDataFormat==0)
        byteCount = f.readBlock((char*)(&data0), dataSize);
      else
        byteCount = f.readBlock((char*)(&data1), dataSize);
      if ((byteCount == -1)||(byteCount!=dataSize))
        if (f.status() == IO_Ok) {
          kdWarning(5003) << "Found broken entry in dynamic-file: Ignored!" << endl;
          continue;
        } else {
          kdError(5003) << "Corrupted dynamic file, IO-error!" << endl;
          clear();
          return false;
        }

      if (d_ynDataFormat==0)
        art=byId(data0.id);
      else
        art=byId(data1.id);

      if(art) {
        if (d_ynDataFormat==0)
          data0.getData(art);
        else
          data1.getData(art);

      if (art->isRead()) readCnt++;
      }

    }

    f.close();

    r_eadCount=readCnt;

  }

  else  {
    clear();
    return false;
  }

  kdDebug(5003) << cnt << " articles read from file" << endl;
  c_ount=length();

  // convert old data files into current format:
  if (d_ynDataFormat!=1) {
    saveDynamicData(length(), true);
    d_ynDataFormat=1;
  }

  // restore "New" - flags
  if( f_irstNew > -1 ) {
    for( int i = f_irstNew; i < length(); i++ ) {
      at(i)->setNew(true);
    }
  }

  updateThreadInfo();
  processXPostBuffer(false);
  return true;
}


bool KNGroup::unloadHdrs(bool force)
{
  if(l_ockedArticles>0)
    return false;

  if (!force && isNotUnloadable())
    return false;

  KNRemoteArticle *a;
  for(int idx=0; idx<length(); idx++) {
    a=at(idx);
    if (a->hasContent() && !knGlobals.articleManager()->unloadArticle(a, force))
      return false;
  }
  syncDynamicData();
  clear();

  return true;
}


// Attention: this method is called from the network thread!
void KNGroup::insortNewHeaders(QStrList *hdrs, QStrList *hdrfmt, KNProtocolClient *client)
{
  KNRemoteArticle *art=0, *art2=0;
  QCString data, hdr, hdrName;
  KQCStringSplitter split;
  split.setIncludeSep(false);
  int new_cnt=0, added_cnt=0, todo=hdrs->count();
  QTime timer;

  l_astFetchCount=0;

  if(!hdrs || hdrs->count()==0)
    return;

  timer.start();

  //resize the list
  if(!resize(size()+hdrs->count())) return;

  // recreate msg-ID index
  syncSearchIndex();

  // remember index of first new
  if(f_irstNew == -1)
    f_irstNew = length(); // index of last + 1

  for(char *line=hdrs->first(); line; line=hdrs->next()) {
    split.init(line, "\t");

    //new Header-Object
    art=new KNRemoteArticle(this);
    art->setNew(true);

    //Article Number
    split.first();
    art->setArticleNumber(split.string().toInt());

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

    // Bytes
    split.next();

    //Lines
    split.next();
    art->lines()->setNumberOfLines(split.string().toInt());

    // optinal additional headers
    mOptionalHeaders = *hdrfmt;
    for (hdr = hdrfmt->first(); hdr; hdr = hdrfmt->next()) {
      if (!split.next())
        break;
      data = split.string();
      int pos = hdr.find(':');
      hdrName = hdr.left( pos );
      // if the header format is 'full' we have to strip the header name
      if (hdr.findRev("full") == (int)(hdr.length() - 5))
        data = data.right( data.length() - (hdrName.length() + 2) );

      // add header
      art->setHeader( new KMime::Headers::Generic( hdrName, art, data ) );
    }

    // check if we have this article already in this group,
    // if so mark it as new (useful with leafnodes delay-body function)
    art2=byMessageId(art->messageID()->as7BitString(false));
    if(art2) { // ok, we already have this article
      art2->setNew(true);
      art2->setArticleNumber(art->articleNumber());
      delete art;
      new_cnt++;
    }
    else if (append(art)) {
      added_cnt++;
      new_cnt++;
    }
    else {
      delete art;
      return;
    }

    if (timer.elapsed() > 200) {           // don't flicker
      timer.restart();
      if (client) client->updatePercentage((new_cnt*30)/todo);
    }
  }

  // now we build the threads
  syncSearchIndex(); // recreate the msgId-index so it contains the appended headers
  buildThreads(added_cnt, client);
  updateThreadInfo();

  // save the new headers
  saveStaticData(added_cnt);
  saveDynamicData(added_cnt);

  // update group-info
  c_ount=length();
  n_ewCount+=new_cnt;
  l_astFetchCount=new_cnt;
  updateListItem();
  saveInfo();
}


int KNGroup::saveStaticData(int cnt,bool ovr)
{
  int idx, savedCnt=0, mode;
  KNRemoteArticle *art;

  QString dir(path());
  if (dir.isNull())
    return 0;

  QFile f(dir+g_roupname+".static");

  if(ovr) mode=IO_WriteOnly;
  else mode=IO_WriteOnly | IO_Append;

  if(f.open(mode)) {

    QTextStream ts(&f);
    ts.setEncoding(QTextStream::Latin1);

    for(idx=length()-cnt; idx<length(); idx++) {

      art=at(idx);

      if(art->isExpired()) continue;

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
      ts << art->date()->unixTime() << ' ';
      ts << "2\n";       // version number to achieve backward compatibility easily

      ts << art->articleNumber() << '\n';

      // optional headers
      ts << mOptionalHeaders.count() << '\n';
      for (QCString hdrName = mOptionalHeaders.first(); hdrName; hdrName = mOptionalHeaders.next()) {
        hdrName = hdrName.left( hdrName.find(':') );
        KMime::Headers::Base *hdr = art->getHeaderByType( hdrName );
        if ( hdr )
          ts << hdrName << ": " << hdr->asUnicodeString() << '\n';
        else
          ts << hdrName << ": \n";
      }

      savedCnt++;
    }

    f.close();
  }

  return savedCnt;
}


void KNGroup::saveDynamicData(int cnt,bool ovr)
{
  dynDataVer1 data;
  int mode;
  KNRemoteArticle *art;

  if(length()>0) {
    QString dir(path());
    if (dir.isNull())
      return;

    QFile f(dir+g_roupname+".dynamic");

    if(ovr) mode=IO_WriteOnly;
    else mode=IO_WriteOnly | IO_Append;

    if(f.open(mode)) {

      for(int idx=length()-cnt; idx<length(); idx++) {
        art=at(idx);
        if(art->isExpired()) continue;
        data.setData(art);
        f.writeBlock((char*)(&data), sizeof(data));
        art->setChanged(false);
      }
      f.close();
    }
    else KNHelper::displayInternalFileError();
  }
}


void KNGroup::syncDynamicData()
{
  dynDataVer1 data;
  int cnt=0, readCnt=0, sOfData;
  KNRemoteArticle *art;

  if(length()>0) {

    QString dir(path());
    if (dir.isNull())
      return;

    QFile f(dir+g_roupname+".dynamic");

    if(f.open(IO_ReadWrite)) {

      sOfData=sizeof(data);

      for(int i=0; i<length(); i++) {
        art=at(i);

        if(art->hasChanged() && !art->isExpired()) {

          data.setData(art);
          f.at(i*sOfData);
          f.writeBlock((char*) &data, sOfData);
          cnt++;
          art->setChanged(false);
        }

        if(art->isRead() && !art->isExpired()) readCnt++;
      }

      f.close();

      kdDebug(5003) << g_roupname << " => updated " << cnt << " entries of dynamic data" << endl;

      r_eadCount=readCnt;
    }
    else KNHelper::displayInternalFileError();
  }
}


void KNGroup::appendXPostID(const QString &id)
{
  c_rosspostIDBuffer.append(id);
}


void KNGroup::processXPostBuffer(bool deleteAfterwards)
{
  QStringList remainder;
  KNRemoteArticle *xp;
  KNRemoteArticle::List al;

  for (QStringList::Iterator it = c_rosspostIDBuffer.begin(); it != c_rosspostIDBuffer.end(); ++it) {
    if ((xp=byMessageId((*it).local8Bit())))
      al.append(xp);
    else
      remainder.append(*it);
  }
  knGlobals.articleManager()->setRead(al, true, false);

  if (!deleteAfterwards)
    c_rosspostIDBuffer = remainder;
  else
    c_rosspostIDBuffer.clear();
}


void KNGroup::buildThreads(int cnt, KNProtocolClient *client)
{
  int end=length(),
      start=end-cnt,
      foundCnt=0, bySubCnt=0, refCnt=0,
      resortCnt=0, idx, oldRef; // idRef;
  KNRemoteArticle *art, *ref;
  QTime timer;

  timer.start();

  // this method is called from the nntp-thread!!!
#ifndef NDEBUG
  qDebug("knode: KNGroup::buildThreads() : start = %d  end = %d",start,end);
#endif

  //resort old hdrs
  if(start>0)
    for(idx=0; idx<start; idx++) {
      art=at(idx);
      if(art->threadingLevel()>1) {
        oldRef=art->idRef();
        ref=findReference(art);
        if(ref) {
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
      if(findReference(art))
        foundCnt++;
    }
    else {
      if(art->subject()->isReply()) {
        art->setIdRef(0); //hdr has no references
        art->setThreadingLevel(0);
      }
      else if(art->idRef()==-1)
        refCnt++;
    }

    if (timer.elapsed() > 200) {           // don't flicker
      timer.restart();
      if(client)
        client->updatePercentage(30+((foundCnt)*70)/cnt);
    }
  }


  if(foundCnt<refCnt) {    // some references could not been found

    //try to sort by subject
    KNRemoteArticle *oldest;
    QPtrList<KNRemoteArticle> list;
    list.setAutoDelete(false);

    for(idx=start; idx<end; idx++) {

      art=at(idx);

      if(art->idRef()==-1) {  //for all not sorted headers

        list.clear();
        list.append(art);

        //find all headers with same subject
        for(int idx2=0; idx2<length(); idx2++)
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
        if (client) client->updatePercentage(30+((bySubCnt+foundCnt)*70)/cnt);
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

  //check for loops in threads
  int startId;
  bool isLoop;
  int iterationCount;
  for (int idx=start; idx<end; idx++){
    art=at(idx);
    startId=art->id();
    isLoop=false;
    iterationCount=0;
    while(art->idRef()!=0 && !isLoop && (iterationCount < end)) {
      art=byId(art->idRef());
      isLoop=(art->id()==startId);
      iterationCount++;
    }

    if(isLoop) {
      // this method is called from the nntp-thread!!!
      #ifndef NDEBUG
      qDebug("knode: Sorting : loop in %d",startId);
      #endif
      art=at(idx);
      art->setIdRef(0);
      art->setThreadingLevel(0);
    }
  }

  // propagate ignored/watched flags to new headers
  for(int idx=start; idx<end; idx++) {
    art=at(idx);
    int idRef=art->idRef();
    int tmpIdRef;

    if(idRef!=0) {
      while(idRef!=0) {
        art=byId(idRef);
         tmpIdRef=art->idRef();
         idRef = (idRef!=tmpIdRef)? tmpIdRef : 0;

      }
      if (art) {
        if (art->isIgnored()) {
          at(idx)->setIgnored(true);
          ++i_gnoreCount;
        }
        at(idx)->setWatched(art->isWatched());
      }
    }
  }

  // this method is called from the nntp-thread!!!
#ifndef NDEBUG
  qDebug("knode: Sorting : %d headers resorted", resortCnt);
  qDebug("knode: Sorting : %d references of %d found", foundCnt, refCnt);
  qDebug("knode: Sorting : %d references of %d sorted by subject", bySubCnt, refCnt);
#endif
}


KNRemoteArticle* KNGroup::findReference(KNRemoteArticle *a)
{
  int found=false;
  QCString ref_mid;
  int ref_nr=0;
  KNRemoteArticle *ref_art=0;

  ref_mid=a->references()->first();

  while(!found && !ref_mid.isNull() && ref_nr < SORT_DEPTH) {
    ref_art=byMessageId(ref_mid);
    if(ref_art) {
      found=true;
      a->setThreadingLevel(ref_nr+1);
      a->setIdRef(ref_art->id());
    }
    ref_nr++;
    ref_mid=a->references()->next();
  }

  return ref_art;
}


void KNGroup::scoreArticles(bool onlynew)
{
  kdDebug(5003) << "KNGroup::scoreArticles()" << endl;
  int len=length(),
      todo=(onlynew)? lastFetchCount():length();

  if (todo) {
    // reset the notify collection
    delete KNScorableArticle::notifyC;
    KNScorableArticle::notifyC = 0;

    kdDebug(5003) << "scoring " << newCount() << " articles" << endl;
    kdDebug(5003) << "(total " << length() << " article in group)" << endl;
    knGlobals.top->setCursorBusy(true);
    knGlobals.setStatusMsg(i18n(" Scoring..."));

    int defScore;
    KScoringManager *sm = knGlobals.scoringManager();
    sm->initCache(groupname());
    for(int idx=0; idx<todo; idx++) {
      KNRemoteArticle *a = at(len-idx-1);
      if ( !a ) {
        kdWarning( 5003 ) << "found no article at " << len-idx-1 << endl;
        continue;
      }

      defScore = 0;
      if (a->isIgnored())
        defScore = knGlobals.configManager()->scoring()->ignoredThreshold();
      else if (a->isWatched())
        defScore = knGlobals.configManager()->scoring()->watchedThreshold();

      if (a->score() != defScore) {
        a->setScore(defScore);
        a->setChanged(true);
      }

      KNScorableArticle sa(a);
      sm->applyRules(sa);
    }

    knGlobals.setStatusMsg(QString::null);
    knGlobals.top->setCursorBusy(false);

    //kdDebug(5003) << KNScorableArticle::notifyC->collection() << endl;
    if (KNScorableArticle::notifyC)
      KNScorableArticle::notifyC->displayCollection(knGlobals.topWidget);
  }
}


void KNGroup::reorganize()
{
  kdDebug(5003) << "KNGroup::reorganize()" << endl;

  knGlobals.top->setCursorBusy(true);
  knGlobals.setStatusMsg(i18n(" Reorganizing headers..."));

  for(int idx=0; idx<length(); idx++) {
    KNRemoteArticle *a = at(idx);
    Q_ASSERT( a );
    a->setId(idx+1); //new ids
    a->setIdRef(-1);
    a->setThreadingLevel(0);
  }

  buildThreads(length());
  saveStaticData(length(), true);
  saveDynamicData(length(), true);
  knGlobals.top->headerView()->repaint();
  knGlobals.setStatusMsg(QString::null);
  knGlobals.top->setCursorBusy(false);
}


void KNGroup::updateThreadInfo()
{
  KNRemoteArticle *ref;
  bool brokenThread=false;

  for(int idx=0; idx<length(); idx++) {
    at(idx)->setUnreadFollowUps(0);
    at(idx)->setNewFollowUps(0);
  }

  for(int idx=0; idx<length(); idx++) {
    int idRef=at(idx)->idRef();
    int tmpIdRef;
    int iterCount=1;         // control iteration count to avoid infinite loops
    while((idRef!=0) && (iterCount <= length())) {
      ref=byId(idRef);
      if(!ref) {
        brokenThread=true;
        break;
      }

      if(!at(idx)->isRead())  {
        ref->incUnreadFollowUps();
        if(at(idx)->isNew()) ref->incNewFollowUps();
      }
      tmpIdRef=ref->idRef();
      idRef= (idRef!=tmpIdRef) ? ref->idRef() : 0;
      iterCount++;
    }
    if(iterCount > length())
      brokenThread=true;
    if(brokenThread) break;
  }

  if(brokenThread) {
    kdWarning(5003) << "KNGroup::updateThreadInfo() : Found broken threading infos! Restoring ..." << endl;
    reorganize();
    updateThreadInfo();
  }
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
  for(int i=0; i<length(); i++)
    if( (at(i)->idRef()==0) && (at(i)->hasNewFollowUps()) ) cnt++;
  return cnt;
}


int KNGroup::statThrWithUnread()
{
  int cnt=0;
  for(int i=0; i<length(); i++)
    if( (at(i)->idRef()==0) && (at(i)->hasUnreadFollowUps()) ) cnt++;
  return cnt;
}

QString KNGroup::prepareForExecution()
{
  if (knGlobals.groupManager()->loadHeaders(this))
    return QString::null;
  else
    return i18n("Cannot load saved headers: %1").arg(groupname());
}

//***************************************************************************

void KNGroup::dynDataVer0::setData(KNRemoteArticle *a)
{
  id=a->id();
  idRef=a->idRef();
  thrLevel=a->threadingLevel();
  read=a->getReadFlag();
  score=a->score();
}


void KNGroup::dynDataVer0::getData(KNRemoteArticle *a)
{
  a->setId(id);
  a->setIdRef(idRef);
  a->setRead(read);
  a->setThreadingLevel(thrLevel);
  a->setScore(score);
}


void KNGroup::dynDataVer1::setData(KNRemoteArticle *a)
{
  id=a->id();
  idRef=a->idRef();
  thrLevel=a->threadingLevel();
  read=a->getReadFlag();
  score=a->score();
  ignoredWatched = 0;
  if (a->isWatched())
    ignoredWatched = 1;
  else if (a->isIgnored())
    ignoredWatched = 2;
}


void KNGroup::dynDataVer1::getData(KNRemoteArticle *a)
{
  a->setId(id);
  a->setIdRef(idRef);
  a->setRead(read);
  a->setThreadingLevel(thrLevel);
  a->setScore(score);
  a->setWatched(ignoredWatched==1);
  a->setIgnored(ignoredWatched==2);
}


KNConfig::Cleanup * KNGroup::activeCleanupConfig()
{
  if (!cleanupConfig()->useDefault())
    return cleanupConfig();
  return account()->activeCleanupConfig();
}
