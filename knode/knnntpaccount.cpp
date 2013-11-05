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

#include "knnntpaccount.h"

#include "utilities.h"
#include "kncollectionviewitem.h"
#include "knconfigmanager.h"
#include "knconfigwidgets.h"
#include "kngroupmanager.h"
#include "knglobals.h"
#include "knaccountmanager.h"

#include <QTimer>
#include <kconfig.h>
#include <kdebug.h>
#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityManager>
#include <kstandarddirs.h>


KNNntpAccountIntervalChecking::KNNntpAccountIntervalChecking(KNNntpAccount* account) : t_imer(0) {
  a_ccount = account;
}



KNNntpAccountIntervalChecking::~KNNntpAccountIntervalChecking()
{
  if (t_imer) deinstallTimer();
  a_ccount = 0;
}



void KNNntpAccountIntervalChecking::installTimer()
{
  if (a_ccount->checkInterval() <= 0) return;
  if(!t_imer)
  {
    t_imer = new QTimer();
    connect(t_imer,SIGNAL(timeout()),this,SLOT(slotCheckNews()));
  }
  else
  {
    t_imer->stop();
  }
  t_imer->start(a_ccount->checkInterval()*60000);
}



void KNNntpAccountIntervalChecking::deinstallTimer()
{
  delete t_imer;
  t_imer = 0;
}



void KNNntpAccountIntervalChecking::slotCheckNews()
{
  knGlobals.groupManager()->checkAll( a_ccount->id(), true );
}



KNNntpAccount::KNNntpAccount()
  : KNCollection( KNCollection::Ptr() ), KNServerInfo(),
    mIdentityUoid( -1 ),
    f_etchDescriptions(true), w_asOpen(false), i_ntervalChecking(false), c_heckInterval(10)
{
  l_astNewFetch = QDate::currentDate();
  a_ccountIntervalChecking = new KNNntpAccountIntervalChecking(this);
  mCleanupConf = new KNode::Cleanup( false );
}


KNNntpAccount::~KNNntpAccount()
{
  delete a_ccountIntervalChecking;
  delete mCleanupConf;
}


// tries to read information, returns false if it fails to do so
bool KNNntpAccount::readInfo(const QString &confPath)
{
  KConfigGroup conf( KSharedConfig::openConfig(confPath, KConfig::SimpleConfig), QString() );

  n_ame = conf.readEntry("name");
  //u_nsentCount = conf.readEntry("unsentCnt", 0);
  f_etchDescriptions = conf.readEntry("fetchDescriptions", true);
  l_astNewFetch = conf.readEntry("lastNewFetch", QDateTime() ).date();
  w_asOpen = conf.readEntry("listItemOpen", false);
  u_seDiskCache = conf.readEntry("useDiskCache", false);
  i_ntervalChecking=conf.readEntry("intervalChecking", false);
  c_heckInterval=conf.readEntry("checkInterval", 10);
  KNServerInfo::readConf(conf);

  mIdentityUoid = conf.readEntry( "identity", -1 );

  startTimer();

  mCleanupConf->loadConfig( conf );

  if (n_ame.isEmpty() || s_erver.isEmpty() || i_d == -1)
    return false;
  else
    return true;
}


void KNNntpAccount::writeConfig()
{
  QString dir(path());
  if (dir.isNull())
    return;

  KConfigGroup conf(KSharedConfig::openConfig( dir+"info", KConfig::SimpleConfig), QString() );

  conf.writeEntry("name", n_ame);
  //conf.writeEntry("unsentCnt", u_nsentCount);
  conf.writeEntry("fetchDescriptions", f_etchDescriptions);
  conf.writeEntry("lastNewFetch", QDateTime(l_astNewFetch));
  if(l_istItem)
    conf.writeEntry("listItemOpen", l_istItem->isExpanded());
  conf.writeEntry("useDiskCache", u_seDiskCache);
  conf.writeEntry("intervalChecking", i_ntervalChecking);
  conf.writeEntry("checkInterval", c_heckInterval);
  conf.writeEntry( "identity", mIdentityUoid );

  KNServerInfo::saveConf(conf);      // save not KNNntpAccount specific settings

  mCleanupConf->saveConfig( conf );
}


/*void KNNntpAccount::syncInfo()
{
  QString dir(path());
  if (dir.isNull())
    return;
  KSimpleConfig conf(dir+"info");
  conf.writeEntry("unsentCnt", u_nsentCount);
}*/


QString KNNntpAccount::path()
{
  if ( i_d == -1 ) {
    return QString();
  }

  QString dir( KStandardDirs::locateLocal( "data", QString( "knode/nntp.%1/" ).arg( i_d ) ) );
  if (dir.isNull())
    KNHelper::displayInternalFileError();
  return (dir);
}


bool KNNntpAccount::editProperties(QWidget *parent)
{
  KNode::NntpAccountConfDialog *d = new KNode::NntpAccountConfDialog(this, parent);

  bool ret=false;
  if (d->exec()) {
    updateListItem();
    ret=true;
  }

  delete d;
  return ret;
}

void KNNntpAccount::startTimer()
{
  if ( (i_ntervalChecking == true) && (c_heckInterval > 0) )
  {
    a_ccountIntervalChecking->installTimer();
  }
  else
  {
    a_ccountIntervalChecking->deinstallTimer();
  }
}

void KNNntpAccount::setCheckInterval(int c)
{
  c_heckInterval = c;
  startTimer();
}

KNode::Cleanup *KNNntpAccount::activeCleanupConfig() const
{
  if (cleanupConfig()->useDefault())
    return knGlobals.configManager()->cleanup();
  return cleanupConfig();
}


const KPIMIdentities::Identity & KNNntpAccount::identity() const
{
  if ( mIdentityUoid < 0 ) {
    return KPIMIdentities::Identity::null();
  }
  return KNGlobals::self()->identityManager()->identityForUoid( mIdentityUoid );
}

void KNNntpAccount::setIdentity( const KPIMIdentities::Identity &identity )
{
  mIdentityUoid = ( identity.isNull() ? -1 : identity.uoid() );
}


KNCollection::Ptr KNNntpAccount::selfPtr()
{
  return KNGlobals::self()->accountManager()->account( id() );
}


