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

#include <QPixmap>
#include <QTextStream>

#include <kconfig.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include <kpimutils/email.h>

#include "kndisplayedheader.h"
#include "knglobals.h"
#include "kngroupmanager.h"
#include "utilities.h"
#include "settings.h"



KNode::Identity::Identity(bool g)
 :  u_seSigFile(false), u_seSigGenerator(false), g_lobal(g)
{
  if(g_lobal) {
    KConfigGroup c(knGlobals.config(), "IDENTITY");
    loadConfig(c);
  }
}


KNode::Identity::~Identity()
{}


void KNode::Identity::loadConfig(const KConfigGroup &c)
{
  n_ame=c.readEntry("Name");
  e_mail=c.readEntry("Email");
  r_eplyTo=c.readEntry("Reply-To");
  m_ailCopiesTo=c.readEntry("Mail-Copies-To");
  o_rga=c.readEntry("Org");
  s_igningKey = c.readEntry("SigningKey").toLocal8Bit();
  u_seSigFile=c.readEntry("UseSigFile",false);
  u_seSigGenerator=c.readEntry("UseSigGenerator",false);
  s_igPath=c.readPathEntry("sigFile", QString());
  s_igText=c.readEntry("sigText");
}


void KNode::Identity::saveConfig(KConfigGroup &c)
{
  c.writeEntry("Name", n_ame);
  c.writeEntry("Email", e_mail);
  c.writeEntry("Reply-To", r_eplyTo);
  c.writeEntry("Mail-Copies-To", m_ailCopiesTo);
  c.writeEntry("Org", o_rga);
  c.writeEntry("SigningKey", QString(s_igningKey));
  c.writeEntry("UseSigFile", u_seSigFile);
  c.writeEntry("UseSigGenerator",u_seSigGenerator);
  c.writePathEntry("sigFile", s_igPath);
  c.writeEntry("sigText", s_igText);
  c.sync();
}


void KNode::Identity::save()
{
  kDebug(5003) <<"KNConfig::Identity::save()";
  if(g_lobal) {
    KConfigGroup c(knGlobals.config(), "IDENTITY");
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
  return KPIMUtils::isValidSimpleAddress( e_mail );
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
              s_igContents += '\n';
          }
          f.close();
        }
        else
          KMessageBox::error(knGlobals.topWidget, i18n("Cannot open the signature file."));
      } else {
        m_process = new KProcess;

        // construct command line...
        QStringList command = s_igPath.split(' ', QString::SkipEmptyParts);
        for ( QStringList::Iterator it = command.begin(); it != command.end(); ++it )
          *m_process << (*it);

        connect(m_process, SIGNAL(readyReadStandardOutput()), SLOT(slotReceiveStdout()));
        connect(m_process, SIGNAL(readyReadStandardError()), SLOT(slotReceiveStderr()));

	m_process->setOutputChannelMode(KProcess::SeparateChannels);
	m_process->start();
	const bool ok = m_process->waitForFinished();
	if( !ok)
	{
          KMessageBox::error(knGlobals.topWidget, i18n("Cannot run the signature generator."));
      	}
	delete m_process;
	m_process = 0L;
      }
    }
  }
  else
    s_igContents = s_igText;

  if (!s_igContents.isEmpty() && !s_igContents.contains("\n-- \n") && !(s_igContents.left(4) == "-- \n"))
    s_igContents.prepend("-- \n");

  return s_igContents;
}


void KNode::Identity::slotReceiveStdout()
{
  s_igContents.append(QString::fromLocal8Bit(m_process->readAllStandardOutput()));
}


void KNode::Identity::slotReceiveStderr()
{
  s_igStdErr.append(QString::fromLocal8Bit(m_process->readAllStandardError ()));
}


//==============================================================================================================


KNode::Appearance::Appearance()
{
  recreateLVIcons();
  i_cons[newFups]         = UserIcon("newsubs");
  i_cons[eyes]            = UserIcon("eyes");
  i_cons[ignore]          = UserIcon("ignore");
  i_cons[mail]            = SmallIcon("mail-message");
  i_cons[posting]         = UserIcon("article");
  i_cons[canceledPosting] = SmallIcon("edit-delete");
  i_cons[savedRemote]     = SmallIcon("edit-copy");
  i_cons[group]           = UserIcon("group");
}


void KNode::Appearance::recreateLVIcons()
{
  QPixmap tempPix = UserIcon("greyball");

  QImage tempImg=tempPix.toImage();
  KIconEffect::colorize(tempImg, knGlobals.settings()->readArticleColor(), 1.0);
  i_cons[greyBall] = QPixmap::fromImage(tempImg);

  tempImg=tempPix.toImage();
  KIconEffect::colorize(tempImg, knGlobals.settings()->unreadArticleColor(), 1.0);
  i_cons[redBall] = QPixmap::fromImage(tempImg);

  tempPix = UserIcon("greyballchk");

  tempImg=tempPix.toImage();
  KIconEffect::colorize(tempImg, knGlobals.settings()->readArticleColor(), 1.0);
  i_cons[greyBallChkd] = QPixmap::fromImage(tempImg);

  tempImg=tempPix.toImage();
  KIconEffect::colorize(tempImg, knGlobals.settings()->unreadArticleColor(), 1.0);
  i_cons[redBallChkd] = QPixmap::fromImage(tempImg);
}


//==============================================================================================================


KNode::DisplayedHeaders::DisplayedHeaders()
{
  QString fname( KStandardDirs::locate( "data","knode/headers.rc" ) );

  if (!fname.isNull()) {
    KConfig headerConf( fname, KConfig::SimpleConfig);
    QStringList headers = headerConf.groupList();
    headers.removeAll("<default>");
    headers.sort();

    KNDisplayedHeader *h;
    QList<int> flags;

    QStringList::Iterator it;
    for( it = headers.begin(); it != headers.end(); ++it ) {
      h=createNewHeader();
      KConfigGroup cg(&headerConf, (*it));
      h->setName(cg.readEntry("Name"));
      h->setTranslateName(cg.readEntry("Translate_Name",true));
      h->setHeader(cg.readEntry("Header"));
      flags=cg.readEntry("Flags",QList<int>());
      if(h->name().isNull() || h->header().isNull() || (flags.count()!=8)) {
        kDebug(5003) <<"KNConfig::DisplayedHeaders::DisplayedHeaders() : ignoring invalid/incomplete Header";
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

  kDebug(5003) <<"KNConfig::DisplayedHeaders::save()";

  QString dir( KStandardDirs::locateLocal( "data", "knode/" ) );
  if (dir.isNull()) {
    KNHelper::displayInternalFileError();
    return;
  }
  KConfig headerConf(dir+"headers.rc", KConfig::SimpleConfig);
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
    KConfigGroup cg(&headerConf, group);
    cg.writeEntry("Name",(*it)->name());
    cg.writeEntry("Translate_Name",(*it)->translateName());
    cg.writeEntry("Header",(*it)->header());
    flags.clear();
    for (int i=0; i<8; i++) {
      if ((*it)->flag(i))
        flags << 1;
      else
        flags << 0;
    }
    cg.writeEntry("Flags",flags);
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
  if ( !mHeaderList.removeAll( h ) )
    kDebug(5003) <<"KNConfig::DisplayedHeaders::remove() : cannot find pointer in list!";

}


void KNode::DisplayedHeaders::up(KNDisplayedHeader *h)
{
  int idx = mHeaderList.indexOf( h );
  if ( idx != -1 ) {
    mHeaderList.takeAt( idx );
    mHeaderList.insert( idx - 1, h );
  }
  else kDebug(5003) <<"KNConfig::DisplayedHeaders::up() : item not found in list";
}


void KNode::DisplayedHeaders::down(KNDisplayedHeader *h)
{
  int idx = mHeaderList.indexOf( h );
  if ( idx != -1 ) {
    mHeaderList.takeAt( idx );
    mHeaderList.insert( idx + 1, h );
  }
  else kDebug(5003) <<"KNConfig::DisplayedHeaders::down() : item not found in list";
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
    loadConfig( knGlobals.config()->group( "EXPIRE" ) );
  }
}


void KNode::Cleanup::loadConfig(const KConfigGroup &conf)
{
  // group expire settings
  d_oExpire = conf.readEntry( "doExpire", true );
  r_emoveUnavailable = conf.readEntry( "removeUnavailable", true );
  p_reserveThr = conf.readEntry( "saveThreads", true );
  e_xpireInterval = conf.readEntry( "expInterval", 5 );
  r_eadMaxAge = conf.readEntry( "readDays", 10 );
  u_nreadMaxAge = conf.readEntry( "unreadDays", 15 );
  mLastExpDate = conf.readEntry( "lastExpire", QDateTime() ).date();

  // folder compaction settings (only available globally)
  if (mGlobal) {
    d_oCompact = conf.readEntry( "doCompact", true );
    c_ompactInterval = conf.readEntry( "comInterval", 5 );
    mLastCompDate = conf.readEntry( "lastCompact", QDateTime() ).date();
  }

  if (!mGlobal)
    mDefault = conf.readEntry( "UseDefaultExpConf", true );
}


void KNode::Cleanup::saveConfig(KConfigGroup &conf)
{
  // group expire settings
  conf.writeEntry( "doExpire", d_oExpire );
  conf.writeEntry( "removeUnavailable", r_emoveUnavailable );
  conf.writeEntry( "saveThreads", p_reserveThr );
  conf.writeEntry( "expInterval", e_xpireInterval );
  conf.writeEntry( "readDays", r_eadMaxAge );
  conf.writeEntry( "unreadDays", u_nreadMaxAge );
  conf.writeEntry( "lastExpire", QDateTime(mLastExpDate) );

  // folder compaction settings (only available globally)
  if (mGlobal) {
    conf.writeEntry( "doCompact", d_oCompact );
    conf.writeEntry( "comInterval", c_ompactInterval );
    conf.writeEntry( "lastCompact", QDateTime(mLastCompDate) );
  }

  if (!mGlobal)
    conf.writeEntry( "UseDefaultExpConf", mDefault );

  conf.sync();
}


void KNode::Cleanup::save()
{
  kDebug(5003) <<"KNConfig::Cleanup::save()";
  if (mGlobal) {
    KConfigGroup conf( knGlobals.config(), "EXPIRE" );
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
