/***************************************************************************
                          knpostcomsettings.cpp  -  description
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


#include "knpostcomsettings.h"
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>
#include <kconfig.h>
#include "utilities.h"

KNPostComSettings::KNPostComSettings(QWidget *p) : KNSettingsWidget(p)
{
	QGroupBox *ggb=new QGroupBox(i18n("General"), this);
	QGroupBox *rgb=new QGroupBox(i18n("Reply"), this);
	QLabel *l1, *l2, *l3;
	
	l1=new QLabel(i18n("max length of lines"), ggb);
	maxLen=new QSpinBox(20, 100, 1, ggb);
	ownSigCB=new QCheckBox(i18n("append signature automatically"), ggb);
	fontCB=new QCheckBox(i18n("use same font as in the article-view"), ggb);
	l2=new QLabel(i18n("Introduction:\n(%NAME=name,%DATE=date,%MSID=msgid)"), rgb);
	intro=new QLineEdit(rgb);
	l3=new QLabel(i18n("begin quoted lines with"), rgb);
	quot=new QLineEdit(rgb);
	authSigCB=new QCheckBox(i18n("include the authors signature"), rgb);
		
	QVBoxLayout *topL=new QVBoxLayout(this, 10);
	QGridLayout *ggbL=new QGridLayout(ggb, 3,2, 20,10);
	QGridLayout *rgbL=new QGridLayout(rgb, 4,2, 20,10);
	
	topL->addWidget(ggb);
	topL->addWidget(rgb);
	topL->addStretch(1);
	ggbL->addWidget(l1, 0,0);
	ggbL->addWidget(maxLen, 0,1);
	ggbL->addWidget(ownSigCB, 1,0);
	ggbL->addWidget(fontCB, 2,0);
	ggbL->setColStretch(0,1);
	rgbL->addMultiCellWidget(l2, 0,0, 0,1);
	rgbL->addMultiCellWidget(intro, 1,1, 0,1);
	rgbL->addWidget(l3, 2,0);
	rgbL->addWidget(quot, 2,1);
	rgbL->addMultiCellWidget(authSigCB, 3,3, 0,1);
	rgbL->setColStretch(0,1);
	topL->setResizeMode(QLayout::Minimum);
	topL->activate();
	
	init();
}



KNPostComSettings::~KNPostComSettings()
{
}



void KNPostComSettings::init()
{
	KConfig *conf=CONF();
	conf->setGroup("POSTNEWS");

  maxLen->setValue(conf->readNumEntry("maxLength", 76));
  ownSigCB->setChecked(conf->readBoolEntry("appSig",true));
  fontCB->setChecked(conf->readBoolEntry("useViewFont", false));
  intro->setText(conf->readEntry("Intro","%NAME wrote:"));
  quot->setText(conf->readEntry("QuotSign",">"));
  authSigCB->setChecked(conf->readBoolEntry("incSig",true));

}



void KNPostComSettings::apply()
{
	KConfig *conf=CONF();
	conf->setGroup("POSTNEWS");
	
	conf->writeEntry("maxLength", maxLen->value());
	conf->writeEntry("appSig", ownSigCB->isChecked());
	conf->writeEntry("useViewFont", fontCB->isChecked());
  conf->writeEntry("Intro", intro->text());
  conf->writeEntry("QuotSign",quot->text());
	conf->writeEntry("incSig", authSigCB->isChecked());
}
