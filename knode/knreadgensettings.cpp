/***************************************************************************
                          knreadgensettings.cpp  -  description
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
#include <qcombobox.h>

#include <knuminput.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>

#include "knreadgensettings.h"


KNReadGenSettings::KNReadGenSettings(QWidget *p) : KNSettingsWidget(p)
{
  QGroupBox *mgb=new QGroupBox(i18n("Misc"), this);
  QGroupBox *vgb=new QGroupBox(i18n("View"), this);
  QGroupBox *agb=new QGroupBox(i18n("Attachments"), this);
  QGroupBox *bgb=new QGroupBox(i18n("Browser"), this);
  QLabel *l1,*l3;
  
  autoCB=new QCheckBox(i18n("check for new articles automatically"), mgb);
  l1=new QLabel(i18n("max to fetch"), mgb);
  maxFetch=new KIntSpinBox(0, 9999, 1, 0, 10, mgb);
  markCB=new QCheckBox(i18n("mark article as read after"), mgb);
  markSecs=new KIntSpinBox(0, 9999, 1, 0, 10, mgb);
  connect(markCB, SIGNAL(toggled(bool)), markSecs, SLOT(setEnabled(bool)));
  markSecs->setSuffix(i18n(" sec"));
  l3=new QLabel(i18n("Open links with"), bgb);
  browser=new QComboBox(bgb);
  browser->insertItem("Konqueror"); 
  browser->insertItem("Netscape");
  expCB=new QCheckBox(i18n("show whole thread on expanding"), vgb);
  sigCB=new QCheckBox(i18n("show signature"), vgb);
  inlineCB=new QCheckBox(i18n("show attachments inline if possible"), agb);
  openAttCB=new QCheckBox(i18n("open attachments on click"), agb);
  altAttCB=new QCheckBox(i18n("show alternative contents as attachments"), agb);
  QVBoxLayout *topL=new QVBoxLayout(this, 5);
  QGridLayout *mgbL=new QGridLayout(mgb, 4,2, 8,5);
  QVBoxLayout *vgbL=new QVBoxLayout(vgb, 8, 5);
  QVBoxLayout *agbL=new QVBoxLayout(agb, 8, 5);
  QGridLayout *bgbL=new QGridLayout(bgb, 2,2, 8,5);

  topL->addWidget(mgb);
  topL->addWidget(vgb);
  topL->addWidget(agb);
  topL->addWidget(bgb);
  topL->addStretch(1);
  mgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  mgbL->addWidget(autoCB, 1,0);
  mgbL->addWidget(l1, 2, 0);
  mgbL->addWidget(maxFetch, 2,1);
  mgbL->addWidget(markCB, 3,0);
  mgbL->addWidget(markSecs, 3,1);
  mgbL->setColStretch(0,1);
  vgbL->addSpacing(fontMetrics().lineSpacing()-4);
  vgbL->addWidget(expCB);
  vgbL->addWidget(sigCB);
  agbL->addSpacing(fontMetrics().lineSpacing()-4);
  agbL->addWidget(inlineCB);
  agbL->addWidget(openAttCB);
  agbL->addWidget(altAttCB);
  bgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  bgbL->addWidget(l3, 1,0);
  bgbL->addWidget(browser, 1,1);
  topL->setResizeMode(QLayout::Minimum);

  init();
}



KNReadGenSettings::~KNReadGenSettings()
{
}



void KNReadGenSettings::init()
{
  KConfig *conf=KGlobal::config();
  
  conf->setGroup("READNEWS");
  maxFetch->setValue(conf->readNumEntry("maxFetch", 1000));
  markCB->setChecked(conf->readBoolEntry("autoMark", true));
  markSecs->setValue(conf->readNumEntry("markSecs", 5));
  markSecs->setEnabled(markCB->isChecked());
  sigCB->setChecked(conf->readBoolEntry("showSig", true));
  browser->setCurrentItem(conf->readNumEntry("Browser", 0));
  autoCB->setChecked(conf->readBoolEntry("autoCheck", true));
  expCB->setChecked(conf->readBoolEntry("totalExpand", true));
  inlineCB->setChecked(conf->readBoolEntry("inlineAtt", true));
  openAttCB->setChecked(conf->readBoolEntry("openAtt", false));
  altAttCB->setChecked(conf->readBoolEntry("showAlts", false));
}


void KNReadGenSettings::apply()
{
  KConfig *conf=KGlobal::config();  
  conf->setGroup("READNEWS");
  conf->writeEntry("autoCheck", autoCB->isChecked());
  conf->writeEntry("showSig",sigCB->isChecked());
  conf->writeEntry("totalExpand",expCB->isChecked());
  conf->writeEntry("maxFetch", maxFetch->value());
  conf->writeEntry("autoMark", markCB->isChecked());
  conf->writeEntry("markSecs", markSecs->value());
  conf->writeEntry("inlineAtt", inlineCB->isChecked());
  conf->writeEntry("openAtt", openAttCB->isChecked());
  conf->writeEntry("showAlts", altAttCB->isChecked());
  conf->writeEntry("Browser", browser->currentItem());
  
}


