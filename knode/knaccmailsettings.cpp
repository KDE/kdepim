/***************************************************************************
                          knaccmailsettings.cpp  -  description
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

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpixmap.h>

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <knumvalidator.h>

#include "knserverinfo.h"
#include "knaccmailsettings.h"


KNAccMailSettings::KNAccMailSettings(QWidget *p) : KNSettingsWidget(p)
{
  QGridLayout *topL=new QGridLayout(this, 5, 3, 5);

  QLabel *l=new QLabel(i18n("Server:"), this);  
  topL->addWidget(l, 0,0);
  s_erver=new QLineEdit(this);  
  topL->addMultiCellWidget(s_erver, 0, 0, 1, 2);
  
  l=new QLabel(i18n("Port:"), this);  
  topL->addWidget(l, 1,0);
  p_ort=new QLineEdit(this);  
  p_ort->setValidator(new KIntValidator(0,65536,this));
  topL->addWidget(p_ort, 1,1);

  l = new QLabel(i18n("Hold connection for:"), this);
  topL->addWidget(l,2,0);
  h_old = new QSpinBox(0,300,5,this);
  h_old->setSuffix(i18n(" sec"));
  topL->addWidget(h_old,2,1);

  l = new QLabel(i18n("Timeout:"), this);
  topL->addWidget(l,3,0);
  t_imeout = new QSpinBox(15,300,5,this);
  t_imeout->setSuffix(i18n(" sec"));
  topL->addWidget(t_imeout,3,1);

  topL->setColStretch(1,1);
  topL->setColStretch(2,1);

  serverInfo=new KNServerInfo();
  serverInfo->setType(KNServerInfo::STsmtp);
    
  init(); 
}



KNAccMailSettings::~KNAccMailSettings()
{
  delete serverInfo;
}



void KNAccMailSettings::init()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("MAILSERVER");
  serverInfo->readConf(conf);
  
  s_erver->setText(serverInfo->server());
  p_ort->setText(QString::number(serverInfo->port()));  
  h_old->setValue(serverInfo->hold());
  t_imeout->setValue(serverInfo->timeout());
}



void KNAccMailSettings::apply()
{
  serverInfo->setServer(s_erver->text());
  serverInfo->setPort(p_ort->text().toInt());
  serverInfo->setHold(h_old->value());
  serverInfo->setTimeout(t_imeout->value());

  KConfig *conf=KGlobal::config();
  conf->setGroup("MAILSERVER");
  serverInfo->saveConf(conf);
}



