/*
    knnntpaccount.cpp

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
#include <kstandarddirs.h>
#include <kdebug.h>

#include "utilities.h"
#include "kncollectionviewitem.h"
#include "knnntpaccount.h"
#include "knconfigmanager.h"


KNNntpAccount::KNNntpAccount()
  : KNCollection(0), KNServerInfo(), i_dentity(0), f_etchDescriptions(true), w_asOpen(false)
{
  l_astNewFetch = QDate::currentDate();
}


KNNntpAccount::~KNNntpAccount()
{
}


// trys to read information, returns false if it fails to do so
bool KNNntpAccount::readInfo(const QString &confPath)
{
  KSimpleConfig conf(confPath);

  n_ame = conf.readEntry("name");
  //u_nsentCount = conf.readNumEntry("unsentCnt", 0);
  f_etchDescriptions = conf.readBoolEntry("fetchDescriptions", true);
  l_astNewFetch = conf.readDateTimeEntry("lastNewFetch").date();
  w_asOpen = conf.readBoolEntry("listItemOpen", false);
  u_seDiskCache = conf.readBoolEntry("useDiskCache", false);
  KNServerInfo::readConf(&conf);


  i_dentity=new KNConfig::Identity(false);
  i_dentity->loadConfig(&conf);
  if(!i_dentity->isEmpty()) {
    kdDebug(5003) << "KNGroup::readInfo(const QString &confPath) : using alternative user for " << n_ame << endl;
  } else {
    delete i_dentity;
    i_dentity=0;
  }

  if (n_ame.isEmpty() || s_erver.isEmpty() || i_d == -1)
    return false;
  else
    return true;
}


void KNNntpAccount::saveInfo()
{
  QString dir(path());
  if (dir.isNull())
    return;
    
  KSimpleConfig conf(dir+"info");
  
  conf.writeEntry("name", n_ame);
  //conf.writeEntry("unsentCnt", u_nsentCount);
  conf.writeEntry("fetchDescriptions", f_etchDescriptions);
  conf.writeEntry("lastNewFetch", QDateTime(l_astNewFetch));
  if(l_istItem)
    conf.writeEntry("listItemOpen", l_istItem->isOpen());
  conf.writeEntry("useDiskCache", u_seDiskCache);

  KNServerInfo::saveConf(&conf);      // save not KNNntpAccount specific settings

  if(i_dentity)
    i_dentity->saveConfig(&conf);
  else if(conf.hasKey("Email")) {
    conf.deleteEntry("Name", false);
    conf.deleteEntry("Email", false);
    conf.deleteEntry("Reply-To", false);
    conf.deleteEntry("Mail-Copies-To", false);
    conf.deleteEntry("Org", false);
    conf.deleteEntry("UseSigFile", false);
    conf.deleteEntry("UseSigGenerator", false);
    conf.deleteEntry("sigFile", false);
    conf.deleteEntry("sigText", false);
  }
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
  QString dir(KGlobal::dirs()->saveLocation("appdata",QString("nntp.%1/").arg(i_d)));
  if (dir.isNull())
    KNHelper::displayInternalFileError();
  return (dir);
}


bool KNNntpAccount::editProperties(QWidget *parent)
{
  if(!i_dentity) i_dentity=new KNConfig::Identity(false);
  KNConfig::NntpAccountConfDialog *d = new KNConfig::NntpAccountConfDialog(this, parent);

  bool ret=false;
  if (d->exec()) {
    updateListItem();
    ret=true;
  }

  if(i_dentity->isEmpty()) {
    delete i_dentity;
    i_dentity=0;
  }

  delete d;
  return ret;
}
