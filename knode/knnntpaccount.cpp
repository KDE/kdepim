/*
    knnntpaccount.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
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
#include <kstddirs.h>

#include "utilities.h"
#include "kncollectionviewitem.h"
#include "knnntpaccount.h"


KNNntpAccount::KNNntpAccount()
  : KNCollection(0), KNServerInfo(), f_etchDescriptions(true), w_asOpen(false)
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
  KNServerInfo::readConf(&conf);

  if (n_ame.isEmpty() || s_erver.isEmpty() || i_d == -1)
    return false;
  else
    return true;
}



void KNNntpAccount::saveInfo()
{
  QString dir(path());
  if (dir == QString::null)
    return;
    
  KSimpleConfig conf(dir+"info");
  
  conf.writeEntry("name", n_ame);
  //conf.writeEntry("unsentCnt", u_nsentCount);
  conf.writeEntry("fetchDescriptions", f_etchDescriptions);
  conf.writeEntry("lastNewFetch", QDateTime(l_astNewFetch));
  if (l_istItem)
    conf.writeEntry("listItemOpen", l_istItem->isOpen());
  KNServerInfo::saveConf(&conf);      // save not KNNntpAccount specific settings
}



/*void KNNntpAccount::syncInfo()
{
  QString dir(path());
  if (dir == QString::null)
    return;
  KSimpleConfig conf(dir+"info");
  conf.writeEntry("unsentCnt", u_nsentCount);
}*/


    
QString KNNntpAccount::path()
{
  QString dir(KGlobal::dirs()->saveLocation("appdata",QString("nntp.%1/").arg(i_d)));
  if (dir==QString::null)
    displayInternalFileError();
  return (dir);
}



