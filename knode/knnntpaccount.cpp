/***************************************************************************
                          knnntpaccount.cpp  -  description
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

#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "utilities.h"
#include "knnntpaccount.h"


KNNntpAccount::KNNntpAccount() : KNCollection(0), KNServerInfo()
{
	u_nsentCount=0;
}



KNNntpAccount::~KNNntpAccount()
{
	
}



void KNNntpAccount::saveInfo()
{
	QString dir(path());
	if (dir == QString::null)
		return;
		
	KSimpleConfig conf(dir+"info");
	
	conf.writeEntry("id", i_d);
	conf.writeEntry("name", n_ame);
	conf.writeEntry("server", QString(s_erver));
	conf.writeEntry("port", p_ort);
	conf.writeEntry("user", QString(u_ser));
	conf.writeEntry("pass", QString(encryptStr(p_ass)));
	conf.writeEntry("unsentCnt", u_nsentCount);
}



void KNNntpAccount::syncInfo()
{
	QString dir(path());
	if (dir == QString::null)
		return;
	KSimpleConfig conf(dir+"info");
	conf.writeEntry("unsentCnt", u_nsentCount);
}


		
QString KNNntpAccount::path()
{
	QString dir(KGlobal::dirs()->saveLocation("appdata",QString("nntp.%1/").arg(i_d)));
	if (dir==QString::null)
		displayInternalFileError();
  return (dir);
}



