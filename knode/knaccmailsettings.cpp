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

#include "knaccmailsettings.h"


KNAccMailSettings::KNAccMailSettings(QWidget *p) : KNSettingsWidget(p)
{
	QGroupBox *sgb=new QGroupBox(i18n("SMTP"), this);
	QLabel *l1, *l2;
	smtp=new QLineEdit(sgb);
	sPort=new QSpinBox(0, 99999, 1, sgb);
	l1=new QLabel(i18n("Server"), sgb);
	l2=new QLabel(i18n("Port"), sgb);
		
	QVBoxLayout *topL=new QVBoxLayout(this, 10);
	QGridLayout *sgbL=new QGridLayout(sgb, 2,3, 20,10);
	
	topL->addWidget(sgb);
	topL->addStretch(1);
	sgbL->addWidget(l1, 0,0);
	sgbL->addMultiCellWidget(smtp, 0,0, 1,2);
	sgbL->addWidget(l2, 1,0);
	sgbL->addWidget(sPort, 1,1);
  sgbL->setColStretch(2,1);
  topL->setResizeMode(QLayout::Minimum);
  topL->activate();
	
  init();	
}



KNAccMailSettings::~KNAccMailSettings()
{
}



void KNAccMailSettings::init()
{
	KConfig *conf=KGlobal::config();
	conf->setGroup("SERVER");
	
	smtp->setText(conf->readEntry("Smtp",""));
	sPort->setValue(conf->readNumEntry("sPort",25));
}



void KNAccMailSettings::apply()
{
	KConfig *conf=KGlobal::config();
	conf->setGroup("SERVER");
	conf->writeEntry("Smtp", smtp->text());
	conf->writeEntry("sPort", sPort->value());
}



