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

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>

#include "knreadgensettings.h"
#include "utilities.h"


KNReadGenSettings::KNReadGenSettings(QWidget *p) : KNSettingsWidget(p)
{
	QGroupBox *mgb=new QGroupBox(i18n("Misc"), this);
	QGroupBox *vgb=new QGroupBox(i18n("View"), this);
	QGroupBox *agb=new QGroupBox(i18n("Attachements"), this);
	QGroupBox *bgb=new QGroupBox(i18n("Browser"), this);
	QLabel *l1, *l2,*l3;
	
	autoCB=new QCheckBox(i18n("check for new articles automatically"), mgb);
	l1=new QLabel(i18n("max to fetch"), mgb);
	maxFetch=new QSpinBox(0, 9999, 1, mgb);
	markCB=new QCheckBox(i18n("mark article as read after"), mgb);
	markSecs=new QSpinBox(0, 9999, 1, mgb);
	l2=new QLabel(i18n("secs"), mgb);
	l3=new QLabel(i18n("Open links with"), bgb);
	browser=new QComboBox(bgb);
	browser->insertItem("Konquerer");	
	browser->insertItem("Netscape");
	expCB=new QCheckBox(i18n("show whole thread on expanding"), vgb);
	sigCB=new QCheckBox(i18n("show signature"), vgb);
	inlineCB=new QCheckBox(i18n("show attachements inline if possible"), agb);
	openAttCB=new QCheckBox(i18n("open attachements on click"), agb);
	altAttCB=new QCheckBox(i18n("show alternative contents as attachements"), agb);
	QVBoxLayout *topL=new QVBoxLayout(this, 10);
	QGridLayout *mgbL=new QGridLayout(mgb, 4,3, 15,10);
	QVBoxLayout *vgbL=new QVBoxLayout(vgb, 15, 10);
	QVBoxLayout *agbL=new QVBoxLayout(agb, 15, 10);
	QHBoxLayout *bgbL=new QHBoxLayout(bgb, 15, 10);
		
	topL->addWidget(mgb);
	topL->addWidget(vgb);
	topL->addWidget(agb);
	topL->addWidget(bgb);
	topL->addStretch(1);
	mgbL->addWidget(autoCB, 0,0);
	mgbL->addWidget(l1, 1,0);
	mgbL->addWidget(maxFetch, 1,1);
	mgbL->addWidget(markCB, 2,0);
	mgbL->addWidget(markSecs, 2,1);
	mgbL->addWidget(l2, 2,2);
	mgbL->setColStretch(0,1);
	vgbL->addWidget(expCB);
	vgbL->addWidget(sigCB);
	agbL->addWidget(inlineCB);
	agbL->addWidget(openAttCB);
	agbL->addWidget(altAttCB);
	bgbL->addWidget(l3);
	bgbL->addWidget(browser, 1);
	topL->setResizeMode(QLayout::Minimum);
	topL->activate();
	
	init();
}



KNReadGenSettings::~KNReadGenSettings()
{
}



void KNReadGenSettings::init()
{
	KConfig *conf=KGlobal::config();
	
	conf->setGroup("READNEWS");
	maxFetch->setValue(conf->readNumEntry("maxFetch", 300));
	markCB->setChecked(conf->readBoolEntry("autoMark", true));
	markSecs->setValue(conf->readNumEntry("markSecs", 5));
	sigCB->setChecked(conf->readBoolEntry("showSig", true));
	browser->setCurrentItem(conf->readNumEntry("Browser", 0));
	autoCB->setChecked(conf->readBoolEntry("autoCheck", true));
	expCB->setChecked(conf->readBoolEntry("totalExpand", true));
	inlineCB->setChecked(conf->readBoolEntry("inlineAtt", false));
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


