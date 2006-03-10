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

#include "knconfig.h"

#include <stdlib.h>

#include <qtextcodec.h>
//Added by qt3to4:
#include <QPixmap>
#include <QTextStream>

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
  s_igningKey = c->readEntry("SigningKey").toLocal8Bit();
  u_seSigFile=c->readEntry("UseSigFile",false);
  u_seSigGenerator=c->readEntry("UseSigGenerator",false);
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
  kDebug(5003) << "KNConfig::Identity::save()" << endl;
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
  s_igContents.clear();      // don't cache file contents
  s_igStdErr.clear();

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
        QStringList command = s_igPath.split(' ', QString::SkipEmptyParts);
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
      h->setTranslateName(headerConf.readEntry("Translate_Name",true));
      h->setHeader(headerConf.readEntry("Header"));
      flags=headerConf.readEntry("Flags",QList<int>());
      if(h->name().isNull() || h->header().isNull() || (flags.count()!=8)) {
        kDebug(5003) << "KNConfig::DisplayedHeaders::DisplayedHeaders() : ignoring invalid/incomplete Header" << endl;
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

  kDebug(5003) << "KNConfig::DisplayedHeaders::save()" << endl;

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
    kDebug(5003) << "KNConfig::DisplayedHeaders::remove() : cannot find pointer in list!" << endl;

}


void KNode::DisplayedHeaders::up(KNDisplayedHeader *h)
{
  int idx = mHeaderList.indexOf( h );
  if ( idx != -1 ) {
    mHeaderList.takeAt( idx );
    mHeaderList.insert( idx - 1, h );
  }
  else kDebug(5003) << "KNConfig::DisplayedHeaders::up() : item not found in list" << endl;
}


void KNode::DisplayedHeaders::down(KNDisplayedHeader *h)
{
  int idx = mHeaderList.indexOf( h );
  if ( idx != -1 ) {
    mHeaderList.takeAt( idx );
    mHeaderList.insert( idx + 1, h );
  }
  else kDebug(5003) << "KNConfig::DisplayedHeaders::down() : item not found in list" << endl;
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
  d_oExpire = conf->readEntry( "doExpire", true );
  r_emoveUnavailable = conf->readEntry( "removeUnavailable", true );
  p_reserveThr = conf->readEntry( "saveThreads", true );
  e_xpireInterval = conf->readEntry( "expInterval", 5 );
  r_eadMaxAge = conf->readEntry( "readDays", 10 );
  u_nreadMaxAge = conf->readEntry( "unreadDays", 15 );
  mLastExpDate = conf->readDateTimeEntry( "lastExpire" ).date();

  // folder compaction settings (only available globally)
  if (mGlobal) {
    d_oCompact = conf->readEntry( "doCompact", true );
    c_ompactInterval = conf->readEntry( "comInterval", 5 );
    mLastCompDate = conf->readDateTimeEntry( "lastCompact" ).date();
  }

  if (!mGlobal)
    mDefault = conf->readEntry( "UseDefaultExpConf", true );
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
  kDebug(5003) << "KNConfig::Cleanup::save()" << endl;
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
