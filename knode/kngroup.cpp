/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/


#include "kngroup.h"

#include "knglobals.h"
#include "kncollectionview.h"
#include "kncollectionviewitem.h"
#include "kngrouppropdlg.h"
#include "utilities.h"
#include "knmainwidget.h"
#include "knscoring.h"
#include "knarticlemanager.h"
#include "kngroupmanager.h"
#include "knnntpaccount.h"
#include "headerview.h"
#include "settings.h"
#include "utils/locale.h"
#include "utils/scoped_cursor_override.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityManager>
#include <QTextStream>
#include <QByteArray>


using namespace KNode::Utilities;
#define SORT_DEPTH 5


KNGroup::KNGroup( KNCollection::Ptr p )
  : KNArticleCollection(p), n_ewCount(0), l_astFetchCount(0), r_eadCount(0), i_gnoreCount(0),
    f_irstNr(0), l_astNr(0), m_axFetch(0), d_ynDataFormat(1), f_irstNew(-1), l_ocked(false),
    u_seCharset(false), s_tatus(unknown),
    mIdentityUoid( -1 )
{
  mCleanupConf = new KNode::Cleanup( false );
}


KNGroup::~KNGroup()
{
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

  if( knGlobals.top && knGlobals.top->collectionView() ) {
    l_istItem->updateColumn( knGlobals.top->collectionView()->totalColumnIndex() );
    l_istItem->updateColumn( knGlobals.top->collectionView()->unreadColumnIndex() );
  }
}


bool KNGroup::readInfo(const QString &confPath)
{
  KConfigGroup info( KSharedConfig::openConfig(confPath, KConfig::SimpleConfig), QString() );

  g_roupname = info.readEntry("groupname");
  d_escription = info.readEntry("description");
  n_ame = info.readEntry("name");
  c_ount = info.readEntry("count",0);
  r_eadCount = info.readEntry("read",0);
  if (r_eadCount > c_ount) r_eadCount = c_ount;
  f_irstNr = info.readEntry("firstMsg",0);
  l_astNr = info.readEntry("lastMsg",0);
  d_ynDataFormat = info.readEntry("dynDataFormat",0);
  u_seCharset = info.readEntry("useCharset", false);
  d_efaultChSet = info.readEntry("defaultChSet").toLatin1();
  QString s = info.readEntry("status","unknown");
  if (s=="readOnly")
    s_tatus = readOnly;
  else if (s=="postingAllowed")
    s_tatus = postingAllowed;
  else if (s=="moderated")
    s_tatus = moderated;
  else
    s_tatus = unknown;
  c_rosspostIDBuffer = info.readEntry("crosspostIDBuffer", QStringList() );

  mIdentityUoid = info.readEntry( "identity", -1 );

  mCleanupConf->loadConfig( info );

  return (!g_roupname.isEmpty());
}


void KNGroup::writeConfig()
{
  QString dir(path());

  if (!dir.isNull()) {
    KConfig _info(dir+g_roupname+".grpinfo", KConfig::SimpleConfig);
    KConfigGroup info( &_info, QString() );

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

    info.writeEntry( "identity", mIdentityUoid );

    mCleanupConf->saveConfig( info );
  }
}


KNNntpAccount::Ptr KNGroup::account()
{
  KNCollection::Ptr p = parent();
  while(p->type()!=KNCollection::CTnntpAccount) p=p->parent();

  return boost::static_pointer_cast<KNNntpAccount>( p_arent );
}

const KPIMIdentities::Identity & KNGroup::identity() const
{
  if ( mIdentityUoid < 0 ) {
    return KPIMIdentities::Identity::null();
  }
  return KNGlobals::self()->identityManager()->identityForUoid( mIdentityUoid );
}


bool KNGroup::loadHdrs()
{
  if(isLoaded()) {
    kDebug(5003) <<"KNGroup::loadHdrs() : nothing to load";
    return true;
  }

  kDebug(5003) <<"KNGroup::loadHdrs() : loading headers";
  QByteArray buffer, hdrValue;
  QFile f;
  int cnt=0, id, lines, fileFormatVersion, artNumber;
  unsigned int timeT;
  KNRemoteArticle::Ptr art;

  QString dir(path());
  if (dir.isNull())
    return false;

  f.setFileName(dir+g_roupname+".static");

  if(f.open(QIODevice::ReadOnly)) {

    while ( !f.atEnd() ) {
      buffer = f.readLine();
      if ( buffer.isEmpty() ){
        if ( f.error() == QFile::NoError ) {
          kWarning(5003) <<"Found broken line in static-file: Ignored!";
          continue;
        } else {
          kError(5003) <<"Corrupted static file, IO-error!";
          clear();
          return false;
        }
      }

      QList<QByteArray> splits = buffer.split( '\t' );
      if ( splits.size() < 4 ) {
        kWarning(5003) <<"Found broken line in static-file: Ignored!";
        continue;
      }
      QList<QByteArray>::ConstIterator it = splits.constBegin();

      art = KNRemoteArticle::Ptr( new KNRemoteArticle( thisGroupPtr() ) );

      art->messageID()->from7BitString( *it );
      ++it;

      art->subject()->from7BitString( *it );
      ++it;

      KMime::Types::Mailbox mbox;
      mbox.setAddress( *it );
      ++it;

      if ( (*it) != "0\n" ) // last item has the line ending
        mbox.setNameFrom7Bit( (*it).trimmed() );
      art->from()->addAddress( mbox );

      buffer = f.readLine().trimmed();
      if ( buffer != "0" )
        art->references()->from7BitString( buffer );

      buffer = f.readLine();
      if ( sscanf( buffer, "%d %d %u %d", &id, &lines, &timeT, &fileFormatVersion) < 4 )
        fileFormatVersion = 0;          // KNode <= 0.4 had no version number
      art->setId(id);
      art->lines()->setNumberOfLines(lines);
      KDateTime dt;
      dt.setTime_t( timeT );
      art->date()->setDateTime( dt );

      if ( fileFormatVersion > 0 ) {
        buffer = f.readLine();
        sscanf( buffer, "%d", &artNumber );
        art->setArticleNumber(artNumber);
      }

      // optional headers
      if ( fileFormatVersion > 1 ) {
        // first line is the number of addiotion headers
        buffer = f.readLine().trimmed();
        // following lines contain one header per line
        for ( uint i = buffer.toUInt(); i > 0; --i ) {
          buffer = f.readLine().trimmed();
          int pos = buffer.indexOf( ':' );
          QByteArray hdrName = buffer.left( pos );
          // skip headers we already set above and which we actually never should
          // find here, but however it still happens... (eg. #101355)
          if ( hdrName == "Subject" || hdrName == "From" || hdrName == "Date"
              || hdrName == "Message-ID" || hdrName == "References"
              || hdrName == "Bytes" || hdrName == "Lines" )
            continue;
          hdrValue = buffer.right( buffer.length() - (pos + 2) );
          if ( hdrValue.length() > 0 )
            art->setHeader( new KMime::Headers::Generic( hdrName, art.get(), hdrValue ) );
        }
      }

      append( art );
      cnt++;
    }

    setLastID();
    f.close();
  }
  else {
    clear();
    return false;
  }


  f.setFileName(dir+g_roupname+".dynamic");

  if (f.open(QIODevice::ReadOnly)) {

    dynDataVer0 data0;
    dynDataVer1 data1;
    int readCnt=0,byteCount,dataSize;
    if (d_ynDataFormat==0)
      dataSize = sizeof(data0);
    else
      dataSize = sizeof(data1);

    while(!f.atEnd()) {

      if (d_ynDataFormat==0)
        byteCount = f.read((char*)(&data0), dataSize);
      else
        byteCount = f.read((char*)(&data1), dataSize);
      if ((byteCount == -1)||(byteCount!=dataSize)) {
        if ( f.error() == QFile::NoError ) {
          kWarning(5003) <<"Found broken entry in dynamic-file: Ignored!";
          continue;
        } else {
          kError(5003) <<"Corrupted dynamic file, IO-error!";
          clear();
          return false;
        }
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

  kDebug(5003) << cnt <<" articles read from file";
  c_ount=length();

  // convert old data files into current format:
  if (d_ynDataFormat!=1) {
    saveDynamicData(length(), true);
    d_ynDataFormat=1;
  }

  // restore "New" - flags
  if( f_irstNew > -1 ) {
    for( int i = f_irstNew; i < length(); ++i ) {
      at(i)->setNew(true);
    }
  }

  updateThreadInfo();
  processXPostBuffer(false);
  return true;
}


bool KNGroup::unloadHdrs(bool force)
{
  if ( lockedArticles() > 0 ) {
    return false;
  }

  if (!force && isNotUnloadable())
    return false;

  KNRemoteArticle::Ptr a;
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
void KNGroup::insortNewHeaders( const KIO::UDSEntryList &list, KNJobData *job)
{
  KNRemoteArticle::Ptr art, art2;
  int new_cnt=0, added_cnt=0;
  int todo = list.count();
  QTime timer;

  l_astFetchCount=0;

  if ( list.isEmpty() )
    return;

  timer.start();


  // recreate msg-ID index
  syncSearchIndex();

  // remember index of first new
  if(f_irstNew == -1)
    f_irstNew = length(); // index of last + 1

  // get a list of additional headers provided in the listing
  if ( mOptionalHeaders.isEmpty() ) {
    KIO::UDSEntry entry = list.first();
    const QList<uint> fields = entry.listFields();
    foreach ( uint field, fields ) {
      if ( field < KIO::UDSEntry::UDS_EXTRA || field > KIO::UDSEntry::UDS_EXTRA_END )
        continue;
      QString value = entry.stringValue( field );
      kDebug(5003) << value;
      QString hdrName = value.left( value.indexOf( ':' ) );
      if ( hdrName == "Subject" || hdrName == "From" || hdrName == "Date"
           || hdrName == "Message-ID" || hdrName == "References"
           || hdrName == "Bytes" || hdrName == "Lines" )
        continue;
      kDebug(5003) <<"Adding optional header:" << hdrName;
      mOptionalHeaders.append( hdrName.toLatin1() );
    }
  }

  QString hdrName, hdrValue;
  for( KIO::UDSEntryList::ConstIterator it = list.begin(); it != list.end(); ++it ) {

    //new Header-Object
    art = KNRemoteArticle::Ptr( new KNRemoteArticle( thisGroupPtr() ) );
    art->setNew(true);

    const QList<uint> fields = (*it).listFields();
    foreach ( uint field, fields ) {

      if ( field == KIO::UDSEntry::UDS_NAME ) {
        //Article Number
        art->setArticleNumber( (*it).stringValue( field ).toInt() );
      } else if ( field >= KIO::UDSEntry::UDS_EXTRA && field <= KIO::UDSEntry::UDS_EXTRA_END ) {
        QString value = (*it).stringValue( field );
        int pos = value.indexOf( ':' );
        if ( pos >= value.length() - 1 )
          continue; // value is empty
        hdrName = value.left( pos );
        hdrValue = value.right( value.length() - ( hdrName.length() + 2 ) );

        if ( hdrName == "Subject" ) {
          QByteArray subject;
          Locale::recodeString( hdrValue.toLatin1(), thisGroupPtr(), subject );
          art->subject()->from7BitString( subject );
          if ( art->subject()->isEmpty() )
            art->subject()->fromUnicodeString( i18n("no subject"), art->defaultCharset() );
        } else if ( hdrName == "From" ) {
          QByteArray from;
          Locale::recodeString( hdrValue.toLatin1(), thisGroupPtr(), from );
          art->from()->from7BitString( from );
          if ( art->from()->as7BitString().isEmpty() ) {
            // If the address incorrect (e.g. "toto <toto AT domain>"), the from is empty !
            // Used the full header as display name.
            const QByteArray email = "@invalid";
            art->from()->addAddress( email, from );
          }
        } else if ( hdrName == "Date" ) {
          art->date()->from7BitString( hdrValue.toLatin1() );
        } else if ( hdrName == "Message-ID" ) {
          art->messageID()->from7BitString( hdrValue.simplified().toLatin1() );
        } else if ( hdrName == "References" ) {
          if( !hdrValue.isEmpty() )
            art->references()->from7BitString( hdrValue.toLatin1() );
        } else if ( hdrName == "Lines" ) {
          art->lines()->setNumberOfLines( hdrValue.toInt() );
        } else {
          // optional extra headers
          art->setHeader( new KMime::Headers::Generic( hdrName.toLatin1(), art.get(), hdrValue.toLatin1() ) );
        }
      }
    }

    // check if we have this article already in this group,
    // if so mark it as new (useful with leafnodes delay-body function)
    art2=byMessageId(art->messageID()->as7BitString(false));
    if(art2) { // ok, we already have this article
      art2->setNew(true);
      art2->setArticleNumber(art->articleNumber());
      new_cnt++;
    } else {
      append( art );
      added_cnt++;
      new_cnt++;
    }

    if (timer.elapsed() > 200) {           // don't flicker
      timer.restart();
      if(job) {
        job->setProgress((new_cnt*30)/todo);
      }
    }
  }

  // now we build the threads
  syncSearchIndex(); // recreate the msgId-index so it contains the appended headers
  buildThreads(added_cnt, job);
  updateThreadInfo();

  // save the new headers
  saveStaticData(added_cnt);
  saveDynamicData(added_cnt);

  // update group-info
  c_ount=length();
  n_ewCount+=new_cnt;
  l_astFetchCount=new_cnt;
  updateListItem();
  writeConfig();
}


int KNGroup::saveStaticData(int cnt,bool ovr)
{
  int idx, savedCnt = 0;
  KNRemoteArticle::Ptr art;

  QString dir(path());
  if (dir.isNull())
    return 0;

  QFile f(dir+g_roupname+".static");

  QIODevice::OpenMode mode;
  if(ovr) mode=QIODevice::WriteOnly;
  else mode=QIODevice::WriteOnly | QIODevice::Append;

  if(f.open(mode)) {

    QTextStream ts(&f);
    ts.setCodec( "ISO 8859-1" );

    for(idx=length()-cnt; idx<length(); ++idx) {

      art=at(idx);

      if(art->isExpired()) continue;

      ts << art->messageID()->as7BitString(false) << '\t';
      ts << art->subject()->as7BitString(false) << '\t';

      KMime::Types::Mailbox mbox;
      if ( !art->from()->isEmpty() )
        mbox = art->from()->mailboxes().first();
      ts << mbox.address() << '\t';

      if ( mbox.hasName() )
        ts << KMime::encodeRFC2047String( mbox.name(), art->from()->rfc2047Charset() ) << '\n';
      else
        ts << "0\n";

      if(!art->references()->isEmpty())
        ts << art->references()->as7BitString(false) << "\n";
      else
        ts << "0\n";

      ts << art->id() << ' ';
      ts << art->lines()->numberOfLines() << ' ';
      ts << art->date()->dateTime().toTime_t() << ' ';
      ts << "2\n";       // version number to achieve backward compatibility easily

      ts << art->articleNumber() << '\n';

      // optional headers
      ts << mOptionalHeaders.count() << '\n';
      Q_FOREACH( const QByteArray &hdrName, mOptionalHeaders ) {
        KMime::Headers::Base *hdr = art->headerByType( hdrName.data() );
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
  KNRemoteArticle::Ptr art;

  if(length()>0) {
    QString dir(path());
    if (dir.isNull())
      return;

    QFile f(dir+g_roupname+".dynamic");

    QIODevice::OpenMode mode;
    if(ovr) mode=QIODevice::WriteOnly;
    else mode=QIODevice::WriteOnly | QIODevice::Append;

    if(f.open(mode)) {

      for(int idx=length()-cnt; idx<length(); idx++) {
        art=at(idx);
        if(art->isExpired()) continue;
        data.setData(art);
        f.write((char*)(&data), sizeof(data));
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
  KNRemoteArticle::Ptr art;

  if(length()>0) {

    QString dir(path());
    if (dir.isNull())
      return;

    QFile f(dir+g_roupname+".dynamic");

    if(f.open(QIODevice::ReadWrite)) {

      sOfData=sizeof(data);

      for(int i=0; i<length(); ++i) {
        art=at(i);

        if(art->hasChanged() && !art->isExpired()) {

          data.setData(art);
          f.seek( i * sOfData );
          f.write((char*) &data, sOfData);
          cnt++;
          art->setChanged(false);
        }

        if(art->isRead() && !art->isExpired()) readCnt++;
      }

      f.close();

      kDebug(5003) << g_roupname <<" => updated" << cnt <<" entries of dynamic data";

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
  KNRemoteArticle::Ptr xp;
  KNRemoteArticle::List al;

  for (QStringList::Iterator it = c_rosspostIDBuffer.begin(); it != c_rosspostIDBuffer.end(); ++it) {
    if ((xp=byMessageId((*it).toLocal8Bit())))
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


void KNGroup::buildThreads(int cnt, KNJobData *job)
{
  int end=length(),
      start=end-cnt,
      foundCnt=0, bySubCnt=0, refCnt=0,
      resortCnt=0, idx, oldRef; // idRef;
  KNRemoteArticle::Ptr art, ref;
  QTime timer;

  timer.start();

  // this method is called from the nntp-thread!!!
#ifndef NDEBUG
  kDebug(5003) << "start =" << start << "end =" << end;
#endif

  //resort old hdrs
  if(start>0)
    for(idx=0; idx<start; ++idx) {
      art=at(idx);
      if(art->threadingLevel()>1) {
        oldRef=art->idRef();
        ref=findReference(art);
        if(ref) {
          // this method is called from the nntp-thread!!!
          #ifndef NDEBUG
          kDebug(5003) << art->id() << ": Old" << oldRef << "New" << art->idRef();
          #endif
          resortCnt++;
          art->setChanged(true);
        }
      }
    }


  for(idx=start; idx<end; ++idx) {

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
      if(job) {
        job->setProgress(30+((foundCnt)*70)/cnt);
      }
    }
  }


  if(foundCnt<refCnt) {    // some references could not been found

    //try to sort by subject
    KNRemoteArticle::Ptr oldest;
    KNRemoteArticle::List list;

    for(idx=start; idx<end; ++idx) {

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
          for ( KNRemoteArticle::List::Iterator it = list.begin(); it != list.end(); ++it )
            if ( (*it)->date()->dateTime() < oldest->date()->dateTime() )
              oldest = (*it);

          //oldest gets idRef 0
          if(oldest->idRef()==-1) bySubCnt++;
          oldest->setIdRef(0);
          oldest->setThreadingLevel(6);

          for ( KNRemoteArticle::List::Iterator it = list.begin(); it != list.end(); ++it ) {
            if ( (*it) == oldest )
              continue;
            if ( (*it)->idRef() == -1 || ( (*it)->idRef() != -1 && (*it)->threadingLevel() == 6 ) ) {
              (*it)->setIdRef(oldest->id());
              (*it)->setThreadingLevel(6);
              if ( (*it)->id() >= at(start)->id() )
                bySubCnt++;
            }
          }
        }
      }

      if (timer.elapsed() > 200) {           // don't flicker
        timer.restart();
        if(job) {
          job->setProgress(30+((bySubCnt+foundCnt)*70)/cnt);
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
      kDebug(5003) << "Sorting : loop in" << startId;
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
         if (art) {
            tmpIdRef=art->idRef();
            idRef = (idRef!=tmpIdRef)? tmpIdRef : 0;
         }
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
  kDebug(5003) << "Sorting :" << resortCnt << "headers resorted";
  kDebug(5003) << "Sorting :" << foundCnt << "references of" << refCnt << "found";
  kDebug(5003) << "Sorting :" << bySubCnt << "references of" << refCnt << "sorted by subject";
#endif
}


KNRemoteArticle::Ptr KNGroup::findReference( KNRemoteArticle::Ptr a )
{
  QByteArray ref_mid;
  KNRemoteArticle::Ptr ref_art;

  QList<QByteArray> references = a->references()->identifiers();

  for ( int ref_nr = 0; ref_nr < references.count() && ref_nr < SORT_DEPTH; ++ref_nr ) {
    ref_mid = '<' + references.at( references.count() - ref_nr - 1 ) + '>';
    ref_art = byMessageId(ref_mid);
    if(ref_art) {
      a->setThreadingLevel(ref_nr+1);
      a->setIdRef(ref_art->id());
      break;
    }
  }

  return ref_art;
}


void KNGroup::scoreArticles(bool onlynew)
{
  kDebug(5003) <<"KNGroup::scoreArticles()";
  int len=length(),
      todo=(onlynew)? lastFetchCount():length();

  if (todo) {
    // reset the notify collection
    delete KNScorableArticle::notifyC;
    KNScorableArticle::notifyC = 0;

    kDebug(5003) <<"scoring" << newCount() <<" articles";
    kDebug(5003) <<"(total" << length() <<" article in group)";
    ScopedCursorOverride cursor( Qt::WaitCursor );
    knGlobals.setStatusMsg(i18n(" Scoring..."));

    int defScore;
    KScoringManager *sm = knGlobals.scoringManager();
    sm->initCache(groupname());
    for(int idx=0; idx<todo; idx++) {
      KNRemoteArticle::Ptr a = at( len-idx-1 );
      if ( !a ) {
        kWarning( 5003 ) <<"found no article at" << len-idx-1;
        continue;
      }

      defScore = 0;
      if (a->isIgnored())
        defScore = knGlobals.settings()->ignoredThreshold();
      else if (a->isWatched())
        defScore = knGlobals.settings()->watchedThreshold();

      if (a->score() != defScore) {
        a->setScore(defScore);
        a->setChanged(true);
      }

      bool read = a->isRead();

      KNScorableArticle sa(a);
      sm->applyRules(sa);

      if ( a->isRead() != read && !read )
        incReadCount();
    }

    knGlobals.setStatusMsg( QString() );
    cursor.restore();

    //kDebug(5003) << KNScorableArticle::notifyC->collection();
    if (KNScorableArticle::notifyC)
      KNScorableArticle::notifyC->displayCollection(knGlobals.topWidget);
  }
}


void KNGroup::reorganize()
{
  kDebug(5003) <<"KNGroup::reorganize()";

  ScopedCursorOverride cursor( Qt::WaitCursor );
  knGlobals.setStatusMsg(i18n(" Reorganizing headers..."));

  for(int idx=0; idx<length(); idx++) {
    KNRemoteArticle::Ptr a = at( idx );
    Q_ASSERT( a );
    a->setId(idx+1); //new ids
    a->setIdRef(-1);
    a->setThreadingLevel(0);
  }

  buildThreads(length());
  saveStaticData(length(), true);
  saveDynamicData(length(), true);
  knGlobals.top->headerView()->repaint();
  knGlobals.setStatusMsg( QString() );
}


void KNGroup::updateThreadInfo()
{
  KNRemoteArticle::Ptr ref;
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
    kWarning(5003) <<"KNGroup::updateThreadInfo() : Found broken threading information! Restoring ...";
    reorganize();
    updateThreadInfo();
  }
}


void KNGroup::showProperties()
{
  KNGroupPropDlg *d=new KNGroupPropDlg(this, knGlobals.topWidget);

  if(d->exec()) {
    if(d->nickHasChanged()) {
      l_istItem->setLabelText( name() );
    }
  }

  delete d;
}


int KNGroup::statThrWithNew()
{
  int cnt=0;
  for(int i=0; i<length(); ++i)
    if( (at(i)->idRef()==0) && (at(i)->hasNewFollowUps()) ) cnt++;
  return cnt;
}


int KNGroup::statThrWithUnread()
{
  int cnt=0;
  for(int i=0; i<length(); ++i)
    if( (at(i)->idRef()==0) && (at(i)->hasUnreadFollowUps()) ) cnt++;
  return cnt;
}

QString KNGroup::prepareForExecution()
{
  if ( KNGlobals::self()->groupManager()->loadHeaders( thisGroupPtr() ) ) {
    return QString();
  } else {
    return i18n("Cannot load saved headers: %1", groupname());
  }
}

//***************************************************************************

void KNGroup::dynDataVer0::setData( KNRemoteArticle::Ptr a )
{
  id=a->id();
  idRef=a->idRef();
  thrLevel=a->threadingLevel();
  read=a->getReadFlag();
  score=a->score();
}


void KNGroup::dynDataVer0::getData( KNRemoteArticle::Ptr a )
{
  a->setId(id);
  a->setIdRef(idRef);
  a->setRead(read);
  a->setThreadingLevel(thrLevel);
  a->setScore(score);
}


void KNGroup::dynDataVer1::setData( KNRemoteArticle::Ptr a )
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


void KNGroup::dynDataVer1::getData( KNRemoteArticle::Ptr a )
{
  a->setId(id);
  a->setIdRef(idRef);
  a->setRead(read);
  a->setThreadingLevel(thrLevel);
  a->setScore(score);
  a->setWatched(ignoredWatched==1);
  a->setIgnored(ignoredWatched==2);
}


KNode::Cleanup * KNGroup::activeCleanupConfig()
{
  if (!cleanupConfig()->useDefault())
    return cleanupConfig();
  return account()->activeCleanupConfig();
}

KNGroup::Ptr KNGroup::thisGroupPtr()
{
    return KNGlobals::self()->groupManager()->group( groupname(), account() );
}
