/***************************************************************************
                          knreadappsettings.cpp  -  description
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

#include <stdio.h>

#include <qlayout.h>
#include <qfontdatabase.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qvalidator.h>

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>

#include "knreadappsettings.h"
#include "utilities.h"

KNReadAppSettings::KNReadAppSettings(QWidget *p) : KNSettingsWidget(p)
{
	QGroupBox *cGrp=new QGroupBox(i18n("Colors"), this);
	QGroupBox *fGrp=new QGroupBox(i18n("Font"), this);
	cList=new QListBox(cGrp);
	colBtn=new KColorButton(cGrp);
	cList->setMinimumSize(200,100);
	colBtn->setMinimumSize(100,40);
		
	cList->insertItem(i18n("normal text"));
	cList->insertItem(i18n("links"));
	cList->insertItem(i18n("background"));
	cList->insertItem(i18n("foreground"));
	cList->insertItem(i18n("quoted text 1"));
	cList->insertItem(i18n("quoted text 2"));
	cList->insertItem(i18n("quoted text 3"));
	
	QLabel *l1=new QLabel(i18n("Family:"), fGrp);
	QLabel *l2=new QLabel(i18n("fontsize:"), fGrp);
				
	fntFam=new QComboBox(false, fGrp);
	fntFam->setSizeLimit(10);
  QFontDatabase fb;
  fntFam->insertStringList(fb.families());
	
	fntSize=new QComboBox(true, fGrp);
	fntSize->setInsertionPolicy(QComboBox::NoInsertion);
  fntSize->setValidator(new QIntValidator(2,200,this));
  static const char* sizes[] = { "4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","22","24","26","28","32","48","64",0 };
  fntSize->insertStrList(sizes);
  		
	QVBoxLayout *topL=new QVBoxLayout(this, 10,5);
	QVBoxLayout *colL=new QVBoxLayout(cGrp, 20,5);
	QGridLayout *fntL=new QGridLayout(fGrp, 2,3, 20,5);
	topL->addWidget(cGrp,1);
	topL->addWidget(fGrp);
	colL->addWidget(cList, 1);
	colL->addWidget(colBtn);
	fntL->addWidget(l1, 0,0);
	fntL->addMultiCellWidget(fntFam, 0,0, 1,2);
	fntL->addWidget(l2, 1,0);
	fntL->addWidget(fntSize, 1,1);
	fntL->setColStretch(2,1);
	topL->setResizeMode(QLayout::Minimum);
	topL->activate();
		
	connect(cList, SIGNAL(highlighted(int)), this, SLOT(slotCListChanged(int)));
	connect(colBtn, SIGNAL(changed(const QColor&)),
		this, SLOT(slotColorChanged(const QColor&)));
	
	init();
  cList->setCurrentItem(0);
}



KNReadAppSettings::~KNReadAppSettings()
{
}



void KNReadAppSettings::init()
{
	char col[10];
	KConfig *c=KGlobal::config();
	QColor w(white), b(black), g(gray);
	QColor *dflt;	
	
	c->setGroup("FONTS-COLORS");
	for(int i=0; i<7; i++) {
		sprintf(col,"color%d", i+1);
		if(i==2) dflt=&w;
		else if(i==3) dflt=&g;
		else dflt=&b;
		colors[i]=c->readColorEntry(col, dflt);
	}
	
	QString s=c->readEntry("family","helvetica");
	for(int i=0; i < fntFam->count(); i++)
		if(fntFam->text(i) == s) {
			fntFam->setCurrentItem(i);
			break;
		}
	
	fntSize->setEditText(QString::number(c->readNumEntry("size", 12)));
}



void KNReadAppSettings::apply()
{
	KConfig *c=KGlobal::config();
	char col[10];
	
	c->setGroup("FONTS-COLORS");
	for(int i=0; i<7; i++) {
		sprintf(col,"color%d", i+1);
		c->writeEntry(col, colors[i]);
	}
	c->writeEntry("family", fntFam->currentText());
	c->writeEntry("size", fntSize->currentText().toInt());
}



void KNReadAppSettings::slotCListChanged(int id)
{
	colBtn->setColor(colors[id]);
}



void KNReadAppSettings::slotColorChanged(const QColor &col)
{
	int idx=cList->currentItem();
	if(idx!=-1) colors[idx]=col;
}



//--------------------------------

#include "knreadappsettings.moc"




