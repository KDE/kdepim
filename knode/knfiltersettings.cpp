/***************************************************************************
                          knfiltersettings.cpp  -  description
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


#include "knfiltersettings.h"
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <kiconloader.h>
#include <klocale.h>
#include "utilities.h"

KNFilterSettings::KNFilterSettings(KNFilterManager *fm, QWidget *p) : KNSettingsWidget(p)
{
	QGroupBox *fgb=new QGroupBox(i18n("Filters"), this);
	QGroupBox *mgb=new QGroupBox(i18n("Menu"), this);
	
	flb=new KNListBox(fgb);
	addBtn=new QPushButton(i18n("Add"), fgb);
	delBtn=new QPushButton(i18n("Delete"), fgb);
	editBtn=new QPushButton(i18n("Edit"), fgb);
	mlb=new KNListBox(mgb);
	upBtn=new QPushButton(i18n("Up"), mgb);
	downBtn=new QPushButton(i18n("Down"), mgb);
	sepAddBtn=new QPushButton(i18n("Add\nSeparator"), mgb);
	sepRemBtn=new QPushButton(i18n("Remove\nSeparator"), mgb);
	SIZE(addBtn); SIZE(editBtn);
	SIZE(upBtn); SIZE(downBtn); SIZE(delBtn);
	SIZE(sepAddBtn); SIZE(sepRemBtn);
	
	QVBoxLayout *topL=new QVBoxLayout(this, 10);
	QGridLayout *fgbL=new QGridLayout(fgb, 4,2, 20,5);
	QGridLayout *mgbL=new QGridLayout(mgb, 5,2, 20,5);
	
	topL->addWidget(fgb);
	topL->addWidget(mgb);
	fgbL->addMultiCellWidget(flb, 0,3, 0,0);
	fgbL->addWidget(addBtn ,0,1);
	fgbL->addWidget(delBtn ,1,1);
	fgbL->addWidget(editBtn ,2,1);
	fgbL->setRowStretch(3,1);
	fgbL->setColStretch(0,1);
	mgbL->addMultiCellWidget(mlb, 0,4, 0,0);
	mgbL->addWidget(upBtn ,0,1);
	mgbL->addWidget(downBtn ,1,1);
	mgbL->addWidget(sepAddBtn ,2,1);
	mgbL->addWidget(sepRemBtn ,3,1);
	mgbL->setRowStretch(4,1);
	mgbL->setColStretch(0,1);
	topL->activate();

	connect(addBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
	connect(delBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
	connect(editBtn, SIGNAL(clicked()), this, SLOT(slotEditBtnClicked()));
	connect(upBtn, SIGNAL(clicked()), this, SLOT(slotUpBtnClicked()));
	connect(downBtn, SIGNAL(clicked()), this, SLOT(slotDownBtnClicked()));
	connect(sepAddBtn, SIGNAL(clicked()), this, SLOT(slotSepAddBtnClicked()));
	connect(sepRemBtn, SIGNAL(clicked()), this, SLOT(slotSepRemBtnClicked()));
	connect(flb, SIGNAL(selected(int)), this, SLOT(slotItemSelected(int)));
	fiManager=fm;
	fiManager->startConfig(this);
}



KNFilterSettings::~KNFilterSettings()
{
	fiManager->endConfig();
}



void KNFilterSettings::addItem(KNArticleFilter *f)
{
	QPixmap pm;
	if(f->isEnabled()) pm=UserIcon("fltrblue");
	else pm=UserIcon("fltrgrey");
	
	KNLBoxItem *it=new KNLBoxItem(f->name(),f,&pm);
	flb->insertItem(it);
}



void KNFilterSettings::removeItem(KNArticleFilter *f)
{
	int i=findItem(flb, f);
  if(i!=-1) flb->removeItem(i);
}



void KNFilterSettings::updateItem(KNArticleFilter *f)
{
	int i=findItem(flb, f);
	
	if(i!=-1) {
		QPixmap pm;
		if(f->isEnabled()) pm=UserIcon("fltrblue");	
	  else pm=UserIcon("fltrgrey");
	  flb->changeItem(new KNLBoxItem(f->name(),f,&pm), i);
	}

	if(f->isEnabled())	
		mlb->changeItem(new KNLBoxItem(f->name(),f,0), findItem(mlb, f));			
}



void KNFilterSettings::addMenuItem(KNArticleFilter *f)
{
	if(f) {
		if(findItem(mlb, f)==-1)
			mlb->insertItem(new KNLBoxItem(f->name(), f,0));
	}
	else mlb->insertItem(new KNLBoxItem("===", 0,0));
}



void KNFilterSettings::removeMenuItem(KNArticleFilter *f)
{
	int i=findItem(mlb, f);
	if(i!=-1) mlb->removeItem(i);		
}



QValueList<int> KNFilterSettings::menuOrder()
{
	KNArticleFilter *f;
	QValueList<int> lst;
	
	for(uint i=0; i<mlb->count(); i++) {
		f=(KNArticleFilter*)(mlb->itemAt(i)->data());
		if(f)
			lst << f->id();
		else
			lst << -1;
	}
 return lst;
}



int KNFilterSettings::findItem(KNListBox *l, KNArticleFilter *f)
{
	int idx=0;
	bool found=false;
	while(!found && idx < (int) l->count()) {
		found=(l->itemAt(idx)->data()==f);
		if(!found) idx++;
	}
	if(found) return idx;
	else return -1;	
}



void KNFilterSettings::slotAddBtnClicked()
{
	fiManager->newFilter();
}



void KNFilterSettings::slotDelBtnClicked()
{
	int c=flb->currentItem();
	KNArticleFilter *f=0;
	
	if(c!=-1) {
		f=(KNArticleFilter*) flb->itemAt(c)->data();
		fiManager->deleteFilter(f);
	}	
	
}



void KNFilterSettings::slotEditBtnClicked()
{
	int c=flb->currentItem();
	KNArticleFilter *f=0;
	
	if(c!=-1) {
		f=(KNArticleFilter*) flb->itemAt(c)->data();
		fiManager->editFilter(f);
	}	
}



void KNFilterSettings::slotUpBtnClicked()
{
	int c=mlb->currentItem();
	KNArticleFilter *f=0;
	
	if(c==0 || c==-1) return;
	f=(KNArticleFilter*) mlb->itemAt(c)->data();
	if(f) mlb->insertItem(new KNLBoxItem(f->name(), f,0), c-1);
	else mlb->insertItem(new KNLBoxItem("===", 0,0), c-1);
	mlb->removeItem(c+1);
	mlb->setCurrentItem(c-1);
}



void KNFilterSettings::slotDownBtnClicked()
{
	int c=mlb->currentItem();
	KNArticleFilter *f=0;
	
	if(c==-1 || c==(int) mlb->count()-1) return;
	f=(KNArticleFilter*) mlb->itemAt(c)->data();
	if(f) mlb->insertItem(new KNLBoxItem(f->name(), f,0), c+2);
  else mlb->insertItem(new KNLBoxItem("===", 0,0), c+2);
  mlb->removeItem(c);
  mlb->setCurrentItem(c+1);
}



void KNFilterSettings::slotSepAddBtnClicked()
{
	int c=mlb->currentItem();
	if(c!=-1) mlb->insertItem(new KNLBoxItem("===", 0,0), c);
}



void KNFilterSettings::slotSepRemBtnClicked()
{
	int c=mlb->currentItem();
	KNArticleFilter *f=0;
	
	if(c!=-1) f=(KNArticleFilter*) mlb->itemAt(c)->data();
	if(c!=-1 && f==0) mlb->removeItem(c);
}



void KNFilterSettings::slotItemSelected(int i)
{
	fiManager->editFilter((KNArticleFilter*) flb->itemAt(i)->data());	
}



//--------------------------------

#include "knfiltersettings.moc"
