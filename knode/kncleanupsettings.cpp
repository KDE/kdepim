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


#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <knuminput.h>
#include <klocale.h>
#include <kconfig.h>
#include <kseparator.h>
#include <kglobal.h>

#include "kncleanupsettings.h"


KNCleanupSettings::KNCleanupSettings(QWidget *p) : KNSettingsWidget(p)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  // === groups ===========================================================

  QGroupBox *groupsB=new QGroupBox(i18n("Groups"), this);
  topL->addWidget(groupsB);
  QGridLayout *groupsL=new QGridLayout(groupsB, 6,2, 8,5);

  groupsL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  groupCB=new QCheckBox(i18n("remove old articles from newsgroups"), groupsB);
  connect(groupCB, SIGNAL(toggled(bool)), this, SLOT(slotGroupCBtoggled(bool)));
  groupsL->addMultiCellWidget(groupCB,1,1,0,1);

  groupDaysL=new QLabel(i18n("purge groups every"), groupsB);
  groupsL->addWidget(groupDaysL,2,0);
  groupDays=new KIntSpinBox(0, 999, 1, 0, 10, groupsB);
  groupDays->setSuffix(i18n(" days"));
  groupsL->addWidget(groupDays,2,1,Qt::AlignRight);

  readDaysL=new QLabel(i18n("keep read articles"), groupsB);
  groupsL->addWidget(readDaysL,3,0);
  readDays=new KIntSpinBox(0, 999, 1, 0, 10, groupsB);
  readDays->setSuffix(i18n(" days"));
  groupsL->addWidget(readDays,3,1,Qt::AlignRight);

  unreadDaysL=new QLabel(i18n("keep unread articles"), groupsB);
  groupsL->addWidget(unreadDaysL,4,0);
  unreadDays=new KIntSpinBox(0, 999, 1, 0, 10, groupsB);
  unreadDays->setSuffix(i18n(" days"));
  groupsL->addWidget(unreadDays,4,1,Qt::AlignRight);

  thrCB=new QCheckBox(i18n("preserve threads"), groupsB);
  groupsL->addWidget(thrCB,5,0);

  groupsL->setColStretch(1,1);

  // === folders =========================================================

  QGroupBox *foldersB=new QGroupBox(i18n("Folders"), this);
  topL->addWidget(foldersB);
  QGridLayout *foldersL=new QGridLayout(foldersB, 3,2, 8,5);

  foldersL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  folderCB=new QCheckBox(i18n("compact folders"), foldersB);
  connect(folderCB, SIGNAL(toggled(bool)), this, SLOT(slotFolderCBtoggled(bool)));
  foldersL->addMultiCellWidget(folderCB,1,1,0,1);

  folderDaysL=new QLabel(i18n("purge folders every"), foldersB);
  foldersL->addWidget(folderDaysL,2,0);
  folderDays=new KIntSpinBox(0, 999, 1, 0, 10, foldersB);
  folderDays->setSuffix(i18n(" days"));
  foldersL->addWidget(folderDays,2,1,Qt::AlignRight);

  foldersL->setColStretch(1,1);

  topL->addStretch(1);

  init();
}



KNCleanupSettings::~KNCleanupSettings()
{
}



void KNCleanupSettings::init()
{
  KConfig *conf=KGlobal::config();
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
  KConfig *conf=KGlobal::config();
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
  groupDaysL->setEnabled(b);
  groupDays->setEnabled(b);
  readDaysL->setEnabled(b);
  readDays->setEnabled(b);
  unreadDaysL->setEnabled(b);
  unreadDays->setEnabled(b);
  thrCB->setEnabled(b);
}



void KNCleanupSettings::slotFolderCBtoggled(bool b)
{
  folderDaysL->setEnabled(b);
  folderDays->setEnabled(b);
}


//--------------------------------

#include "kncleanupsettings.moc"

    
