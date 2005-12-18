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

#include "knconfig.h"

#include <stdlib.h>

#include <qtextcodec.h>
//Added by qt3to4:
#include <QPixmap>
#include <QTextStream>
#include <Q3CString>

#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kprocess.h>

#include <email.h>

#include "kndisplayedheader.h"
#include "knglobals.h"
#include "kngroupmanager.h"
#include "utilities.h"
#include "settings.h"



KNode::Identity::Identity(bool g)
 :  u_seSigFile(false), u_seSigGenerator(false), g_lobal(g)
{
  if(g_lobal) {
    KConfig *c=knGlobals.config();
    c->setGroup("IDENTITY");
    loadConfig(c);
  }
}


KNode::Identity::~Identity()
{}


void KNode::Identity::loadConfig(KConfigBase *c)
{
  n_ame=c->readEntry("Name");
  e_mail=c->readEntry("Email");
  r_eplyTo=c->readEntry("Reply-To");
  m_ailCopiesTo=c->readEntry("Mail-Copies-To");
  o_rga=c->readEntry("Org");
  s_igningKey = c->readEntry("SigningKey").local8Bit();
  u_seSigFile=c->readBoolEntry("UseSigFile",false);
  u_seSigGenerator=c->readBoolEntry("UseSigGenerator",false);
  s_igPath=c->readPathEntry("sigFile");
  s_igText=c->readEntry("sigText");
}


void KNode::Identity::saveConfig(KConfigBase *c)
{
  c->writeEntry("Name", n_ame);
  c->writeEntry("Email", e_mail);
  c->writeEntry("Reply-To", r_eplyTo);
  c->writeEntry("Mail-Copies-To", m_ailCopiesTo);
  c->writeEntry("Org", o_rga);
  c->writeEntry("SigningKey", QString(s_igningKey));
  c->writeEntry("UseSigFile", u_seSigFile);
  c->writeEntry("UseSigGenerator",u_seSigGenerator);
  c->writePathEntry("sigFile", s_igPath);
  c->writeEntry("sigText", s_igText);
  c->sync();
}


void KNode::Identity::save()
{
  kdDebug(5003) << "KNConfig::Identity::save()" << endl;
  if(g_lobal) {
    KConfig *c=knGlobals.config();
    c->setGroup("IDENTITY");
    saveConfig(c);
  }
}


bool KNode::Identity::isEmpty()
{
  return (  n_ame.isEmpty() &&  e_mail.isEmpty() &&
            r_eplyTo.isEmpty() && m_ailCopiesTo.isEmpty() &&
            o_rga.isEmpty() && s_igPath.isEmpty() && s_igText.isEmpty() &&
            s_igningKey.isEmpty() );
}


bool KNode::Identity::emailIsValid()
{
  return KPIM::isValidSimpleEmailAddress( e_mail );
}


QString KNode::Identity::getSignature()
{
  s_igContents = QString::null;      // don't cache file contents
  s_igStdErr = QString::null;

  if (u_seSigFile) {
    if(!s_igPath.isEmpty()) {
      if (!u_seSigGenerator) {
        QFile f(s_igPath);
        if(f.open(QIODevice::ReadOnly)) {
          QTextStream ts(&f);
          while(!ts.atEnd()) {
            s_igContents += ts.readLine();
            if (!ts.atEnd())
              s_igContents += "\n";
          }
          f.close();
        }
        else
          KMessageBox::error(knGlobals.topWidget, i18n("Cannot open the signature file."));
      } else {
        KProcess process;

        // construct command line...
        QStringList command = QStringList::split(' ',s_igPath);
        for ( QStringList::Iterator it = command.begin(); it != command.end(); ++it )
          process << (*it);

        connect(&process, SIGNAL(receivedStdout(KProcess *, char *, int)), SLOT(slotReceiveStdout(KProcess *, char *, int)));
        connect(&process, SIGNAL(receivedStderr(KProcess *, char *, int)), SLOT(slotReceiveStderr(KProcess *, char *, int)));

        if (!process.start(KProcess::Block,KProcess::AllOutput))
          KMessageBox::error(knGlobals.topWidget, i18n("Cannot run the signature generator."));
      }
    }
  }
  else
    s_igContents = s_igText;

  if (!s_igContents.isEmpty() && !s_igContents.contains("\n-- \n") && !(s_igContents.left(4) == "-- \n"))
    s_igContents.prepend("-- \n");

  return s_igContents;
}


void KNode::Identity::slotReceiveStdout(KProcess *, char *buffer, int buflen)
{
  s_igContents.append(QString::fromLocal8Bit(buffer,buflen));
}


void KNode::Identity::slotReceiveStderr(KProcess *, char *buffer, int buflen)
{
  s_igStdErr.append(QString::fromLocal8Bit(buffer,buflen));
}


//==============================================================================================================


KNode::Appearance::Appearance()
{
  recreateLVIcons();
  i_cons[newFups]         = UserIcon("newsubs");
  i_cons[eyes]            = UserIcon("eyes");
  i_cons[ignore]          = UserIcon("ignore");
  i_cons[mail]            = SmallIcon("mail_generic");
  i_cons[posting]         = UserIcon("article");
  i_cons[canceledPosting] = SmallIcon("editdelete");
  i_cons[savedRemote]     = SmallIcon("editcopy");
  i_cons[group]           = UserIcon("group");
}


void KNode::Appearance::recreateLVIcons()
{
  QPixmap tempPix = UserIcon("greyball");

  QImage tempImg=tempPix.convertToImage();
  KIconEffect::colorize(tempImg, knGlobals.settings()->readArticleColor(), 1.0);
  i_cons[greyBall].convertFromImage(tempImg);

  tempImg=tempPix.convertToImage();
  KIconEffect::colorize(tempImg, knGlobals.settings()->unreadArticleColor(), 1.0);
  i_cons[redBall].convertFromImage(tempImg);

  tempPix = UserIcon("greyballchk");

  tempImg=tempPix.convertToImage();
  KIconEffect::colorize(tempImg, knGlobals.settings()->readArticleColor(), 1.0);
  i_cons[greyBallChkd].convertFromImage(tempImg);

  tempImg=tempPix.convertToImage();
  KIconEffect::colorize(tempImg, knGlobals.settings()->unreadArticleColor(), 1.0);
  i_cons[redBallChkd].convertFromImage(tempImg);
}


//==============================================================================================================


KNode::DisplayedHeaders::DisplayedHeaders()
{
  QString fname( locate("data","knode/headers.rc") );

  if (!fname.isNull()) {
    KSimpleConfig headerConf(fname,true);
    QStringList headers = headerConf.groupList();
    headers.remove("<default>");
    headers.sort();

    KNDisplayedHeader *h;
    QList<int> flags;

    QStringList::Iterator it;
    for( it = headers.begin(); it != headers.end(); ++it ) {
      h=createNewHeader();
      headerConf.setGroup((*it));
      h->setName(headerConf.readEntry("Name"));
      h->setTranslateName(headerConf.readBoolEntry("Translate_Name",true));
      h->setHeader(headerConf.readEntry("Header"));
      flags=headerConf.readIntListEntry("Flags");
      if(h->name().isNull() || h->header().isNull() || (flags.count()!=8)) {
        kdDebug(5003) << "KNConfig::DisplayedHeaders::DisplayedHeaders() : ignoring invalid/incomplete Header" << endl;
        remove(h);
      }
      else {
        for (int i=0; i<8; i++)
          h->setFlag(i, (flags[i]>0));
        h->createTags();
      }
    }
  }
}


KNode::DisplayedHeaders::~DisplayedHeaders()
{
  for ( KNDisplayedHeader::List::Iterator it = mHeaderList.begin(); it != mHeaderList.end(); ++it )
    delete (*it);
}


void KNode::DisplayedHeaders::save()
{
  if(!d_irty)
    return;

  kdDebug(5003) << "KNConfig::DisplayedHeaders::save()" << endl;

  QString dir(locateLocal("data","knode/"));
  if (dir.isNull()) {
    KNHelper::displayInternalFileError();
    return;
  }
  KSimpleConfig headerConf(dir+"headers.rc");
  QStringList oldHeaders = headerConf.groupList();

  QStringList::Iterator oldIt=oldHeaders.begin();
  for( ;oldIt != oldHeaders.end(); ++oldIt )      // remove all old groups
    headerConf.deleteGroup((*oldIt));             // find a better way to do it?

  QList<int> flags;
  int idx=0;
  QString group;

  for ( KNDisplayedHeader::List::Iterator it = mHeaderList.begin(); it != mHeaderList.end(); ++it ) {
    group.setNum(idx++);
    while (group.length()<3)
      group.prepend("0");
    headerConf.setGroup(group);
    headerConf.writeEntry("Name",(*it)->name());
    headerConf.writeEntry("Translate_Name",(*it)->translateName());
    headerConf.writeEntry("Header",(*it)->header());
    flags.clear();
    for (int i=0; i<8; i++) {
      if ((*it)->flag(i))
        flags << 1;
      else
        flags << 0;
    }
    headerConf.writeEntry("Flags",flags);
  }
  headerConf.sync();
  d_irty = false;
}


KNDisplayedHeader* KNode::DisplayedHeaders::createNewHeader()
{
  KNDisplayedHeader *h=new KNDisplayedHeader();
  mHeaderList.append( h );

  return h;
}


void KNode::DisplayedHeaders::remove(KNDisplayedHeader *h)
{
  if ( !mHeaderList.remove( h ) )
    kdDebug(5003) << "KNConfig::DisplayedHeaders::remove() : cannot find pointer in list!" << endl;

}


void KNode::DisplayedHeaders::up(KNDisplayedHeader *h)
{
  int idx = mHeaderList.indexOf( h );
  if ( idx != -1 ) {
    mHeaderList.takeAt( idx );
    mHeaderList.insert( idx - 1, h );
  }
  else kdDebug(5003) << "KNConfig::DisplayedHeaders::up() : item not found in list" << endl;
}


void KNode::DisplayedHeaders::down(KNDisplayedHeader *h)
{
  int idx = mHeaderList.indexOf( h );
  if ( idx != -1 ) {
    mHeaderList.takeAt( idx );
    mHeaderList.insert( idx + 1, h );
  }
  else kdDebug(5003) << "KNConfig::DisplayedHeaders::down() : item not found in list" << endl;
}


//==============================================================================================================


KNode::XHeader::XHeader(const QString &s)
{
  int pos = s.indexOf(": ");
  if ( pos != -1 ) {
    mName = s.left( pos );
    pos += 2;
    mValue = s.right( s.length() - pos );
  }
}


//==============================================================================================================


KNode::PostNewsTechnical::PostNewsTechnical()
 : findComposerCSCache(113)
{
  findComposerCSCache.setAutoDelete(true);

  KConfig *conf=knGlobals.config();
  conf->setGroup("POSTNEWS");

  c_omposerCharsets=conf->readListEntry("ComposerCharsets");
  if (c_omposerCharsets.isEmpty())
    c_omposerCharsets=QStringList::split(',',"us-ascii,utf-8,iso-8859-1,iso-8859-2,"
    "iso-8859-3,iso-8859-4,iso-8859-5,iso-8859-6,iso-8859-7,iso-8859-8,"
    "iso-8859-9,iso-8859-10,iso-8859-13,iso-8859-14,iso-8859-15,koi8-r,koi8-u,"
    "iso-2022-jp,iso-2022-jp-2,iso-2022-kr,euc-jp,euc-kr,Big5,gb2312");

  c_harset=conf->readEntry("Charset").toLatin1();
  if (c_harset.isEmpty()) {
    Q3CString localeCharset(QTextCodec::codecForLocale()->mimeName());

    // special logic for japanese users:
    // "euc-jp" is default encoding for them, but in the news
    // "iso-2022-jp" is used
    if (localeCharset.toLower() == "euc-jp")
      localeCharset = "iso-2022-jp";

    c_harset=findComposerCharset(localeCharset);
    if (c_harset.isEmpty())
      c_harset="iso-8859-1";  // shit
  }

  QString dir(locateLocal("data","knode/"));
  if (!dir.isNull()) {
    QFile f(dir+"xheaders");
    if(f.open(QIODevice::ReadOnly)) {
      QTextStream ts(&f);
      while(!ts.atEnd())
        mXheaders.append( XHeader( ts.readLine() ) );

      f.close();
    }
  }
}


KNode::PostNewsTechnical::~PostNewsTechnical()
{
}


void KNode::PostNewsTechnical::save()
{
  if(!d_irty)
    return;

  kdDebug(5003) << "KNConfig::PostNewsTechnical::save()" << endl;

  KConfig *conf=knGlobals.config();
  conf->setGroup("POSTNEWS");

  conf->writeEntry("ComposerCharsets", c_omposerCharsets);
  conf->writeEntry("Charset", QString::fromLatin1(c_harset));

  QString dir(locateLocal("data","knode/"));
  if (dir.isNull())
    KNHelper::displayInternalFileError();
  else {
    QFile f(dir+"xheaders");
    if(f.open(QIODevice::WriteOnly)) {
      QTextStream ts(&f);
      XHeaders::Iterator it;
      for ( it = mXheaders.begin(); it != mXheaders.end(); ++it )
        ts << (*it).header() << "\n";
      ts.flush();
      f.close();
    }
    else
      KNHelper::displayInternalFileError();
  }
  conf->sync();
  d_irty = false;
}


int KNode::PostNewsTechnical::indexForCharset(const Q3CString &str)
{
  int i=0;
  bool found=false;
  for ( QStringList::Iterator it = c_omposerCharsets.begin(); it != c_omposerCharsets.end(); ++it ) {
    if ((*it).toLower() == str.toLower().data()) {
      found = true;
      break;
    }
    i++;
  }
  if (!found) {
    i=0;
    for ( QStringList::Iterator it = c_omposerCharsets.begin(); it != c_omposerCharsets.end(); ++it ) {
      if ((*it).toLower() == c_harset.toLower().data()) {
        found = true;
        break;
      }
      i++;
    }
    if (!found)
      i=0;
  }
  return i;
}


Q3CString KNode::PostNewsTechnical::findComposerCharset(Q3CString cs)
{
  Q3CString *ret=findComposerCSCache.find(cs);
  if (ret)
    return *ret;

  Q3CString s;

  QStringList::Iterator it;
  for( it = c_omposerCharsets.begin(); it != c_omposerCharsets.end(); ++it ) {
    // match by name
    if ((*it).toLower()==cs.toLower().data()) {
      s = (*it).toLatin1();
      break;
    }
  }

  if (s.isEmpty()) {
    for( it = c_omposerCharsets.begin(); it != c_omposerCharsets.end(); ++it ) {
    // match by charset, avoid to return "us-ascii" for iso-8859-1
      if ((*it).toLower()!="us-ascii") {
        QTextCodec *composerCodec = QTextCodec::codecForName((*it).toLatin1());
        QTextCodec *csCodec = QTextCodec::codecForName(cs);
        if ((composerCodec != 0) &&
            (csCodec != 0) &&
            (0 == strcmp(composerCodec->name(), csCodec->name()))) {
      s = (*it).toLatin1();
      break;
    }
  }
    }
  }

  if (s.isEmpty())
    s = "us-ascii";

  findComposerCSCache.insert(cs, new Q3CString(s));

  return s;
}


//==============================================================================================================



//BEGIN: Cleanup configuration ===============================================


KNode::Cleanup::Cleanup( bool global ) :
  // default values for new accounts / groups
  d_oExpire( true ), r_emoveUnavailable( true ), p_reserveThr( true ),
  e_xpireInterval( 5 ), r_eadMaxAge( 10 ), u_nreadMaxAge( 15 ),
  mGlobal(global), mDefault(!global), mLastExpDate( QDate::currentDate() )
{
  if (mGlobal) {
    KConfig *conf = knGlobals.config();
    conf->setGroup( "EXPIRE" );
    loadConfig( conf );
  }
}


void KNode::Cleanup::loadConfig(KConfigBase *conf)
{
  // group expire settings
  d_oExpire = conf->readBoolEntry( "doExpire", true );
  r_emoveUnavailable = conf->readBoolEntry( "removeUnavailable", true );
  p_reserveThr = conf->readBoolEntry( "saveThreads", true );
  e_xpireInterval = conf->readNumEntry( "expInterval", 5 );
  r_eadMaxAge = conf->readNumEntry( "readDays", 10 );
  u_nreadMaxAge = conf->readNumEntry( "unreadDays", 15 );
  mLastExpDate = conf->readDateTimeEntry( "lastExpire" ).date();

  // folder compaction settings (only available globally)
  if (mGlobal) {
    d_oCompact = conf->readBoolEntry( "doCompact", true );
    c_ompactInterval = conf->readNumEntry( "comInterval", 5 );
    mLastCompDate = conf->readDateTimeEntry( "lastCompact" ).date();
  }

  if (!mGlobal)
    mDefault = conf->readBoolEntry( "UseDefaultExpConf", true );
}


void KNode::Cleanup::saveConfig(KConfigBase *conf)
{
  // group expire settings
  conf->writeEntry( "doExpire", d_oExpire );
  conf->writeEntry( "removeUnavailable", r_emoveUnavailable );
  conf->writeEntry( "saveThreads", p_reserveThr );
  conf->writeEntry( "expInterval", e_xpireInterval );
  conf->writeEntry( "readDays", r_eadMaxAge );
  conf->writeEntry( "unreadDays", u_nreadMaxAge );
  conf->writeEntry( "lastExpire", mLastExpDate );

  // folder compaction settings (only available globally)
  if (mGlobal) {
    conf->writeEntry( "doCompact", d_oCompact );
    conf->writeEntry( "comInterval", c_ompactInterval );
    conf->writeEntry( "lastCompact", mLastCompDate );
  }

  if (!mGlobal)
    conf->writeEntry( "UseDefaultExpConf", mDefault );

  conf->sync();
}


void KNode::Cleanup::save()
{
  kdDebug(5003) << "KNConfig::Cleanup::save()" << endl;
  if (mGlobal) {
    KConfig *conf = knGlobals.config();
    conf->setGroup( "EXPIRE" );
    saveConfig( conf );
  }
}


bool KNode::Cleanup::expireToday()
{
  if (!d_oExpire)
    return false;

  QDate today = QDate::currentDate();
  if (mLastExpDate == today)
    return false;

  return (mLastExpDate.daysTo( today ) >= e_xpireInterval);
}


void KNode::Cleanup::setLastExpireDate()
{
  mLastExpDate = QDateTime::currentDateTime().date();
}


bool KNode::Cleanup::compactToday()
{
  if (!d_oCompact)
    return false;

  QDate today = QDate::currentDate();
  if (mLastCompDate == today)
    return false;

  return (mLastCompDate.daysTo( today ) >= c_ompactInterval);
}


void KNode::Cleanup::setLastCompactDate()
{
  mLastCompDate = QDateTime::currentDateTime().date();
}


//END: Cleanup configuration =================================================


#include "knconfig.moc"
