/***************************************************************************
                          kncleanupsettings.cpp  -  description
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


#include "kncleanupsettings.h"
#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>
#include <kconfig.h>
#include <kseparator.h>
#include "utilities.h"

KNCleanupSettings::KNCleanupSettings(QWidget *p) : KNSettingsWidget(p)
{
	QLabel *l1, *l2, *l3, *l4, *l5, *l6;
	folderCB=new QCheckBox(i18n("compact folders every"), this);
	groupCB=new QCheckBox(i18n("purge groups every"), this);
	folderDays=new QSpinBox(0, 999, 1, this);
	groupDays=new QSpinBox(0, 999, 1, this);
	readDays=new QSpinBox(0, 999, 1, this);
	unreadDays=new QSpinBox(0, 999, 1, this);
	thrCB=new QCheckBox(i18n("save threads"), this);
	l1=new QLabel(i18n("days"), this);
	l2=new QLabel(i18n("keep read articles"), this);
	l3=new QLabel(i18n("days"), this);
	l4=new QLabel(i18n("keep unread articles"), this);
	l5=new QLabel(i18n("days"), this);
	l6=new QLabel(i18n("days"), this);
	KSeparator *sep=new KSeparator(this);
		
	QGridLayout *topL=new QGridLayout(this, 7, 3, 10);
	topL->addWidget(groupCB, 0,0);
	topL->addWidget(groupDays, 0,1);
	topL->addWidget(l1, 0,2);
	topL->addWidget(l2, 1,0);
	topL->addWidget(readDays, 1,1);
	topL->addWidget(l3, 1,2);
	topL->addWidget(l4, 2,0);
	topL->addWidget(unreadDays, 2,1);
	topL->addWidget(l5, 2,2);
	topL->addWidget(thrCB, 3,0);
	topL->addMultiCellWidget(sep, 4,4, 0,2);
	topL->addWidget(folderCB, 5,0);
	topL->addWidget(folderDays, 5,1);
	topL->addWidget(l6, 5,2);
	topL->setColStretch(0,1);
	topL->setRowStretch(6,1);
	topL->setResizeMode(QLayout::Minimum);
	topL->activate();
	
	connect(groupCB, SIGNAL(toggled(bool)), this, SLOT(slotGroupCBtoggled(bool)));
	connect(folderCB, SIGNAL(toggled(bool)), this, SLOT(slotFolderCBtoggled(bool)));
	
	init();
}



KNCleanupSettings::~KNCleanupSettings()
{
}



void KNCleanupSettings::init()
{
	KConfig *conf=CONF();
	conf->setGroup("EXPIRE");
	groupCB->setChecked(conf->readBoolEntry("doExpire", true));
	slotGroupCBtoggled(groupCB->isChecked());
	groupDays->setValue(conf->readNumEntry("expInterval", 5));
	readDays->setValue(conf->readNumEntry("readDays",10));
	unreadDays->setValue(conf->readNumEntry("unreadDays",15));
	thrCB->setChecked(conf->readBoolEntry("saveThreads",true));
	folderCB->setChecked(conf->readBoolEntry("doCompact", true));
	slotFolderCBtoggled(folderCB->isChecked());
	folderDays->setValue(conf->readNumEntry("comInterval", 5));
}



void KNCleanupSettings::apply()
{
	KConfig *conf=CONF();
	conf->setGroup("EXPIRE");
	conf->writeEntry("doExpire", groupCB->isChecked());
	conf->writeEntry("expInterval", groupDays->value());
	conf->writeEntry("unreadDays", unreadDays->value());
	conf->writeEntry("readDays", readDays->value());
	conf->writeEntry("saveThreads", thrCB->isChecked());
	conf->writeEntry("doCompact", folderCB->isChecked());
	conf->writeEntry("comInterval", folderDays->value());
}



void KNCleanupSettings::slotGroupCBtoggled(bool b)
{
	groupDays->setEnabled(b);
	readDays->setEnabled(b);
	unreadDays->setEnabled(b);
	thrCB->setEnabled(b);
}



void KNCleanupSettings::slotFolderCBtoggled(bool b)
{
	folderDays->setEnabled(b);
}



//--------------------------------

#include "kncleanupsettings.moc"

		
