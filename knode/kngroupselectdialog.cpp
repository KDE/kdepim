/***************************************************************************
                     kngroupselectdialog.cpp - description
 copyright            : (C) 1999 by Christian Thurner
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

#include <qlayout.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>

#include <kseparator.h>

#include "kngrouplistwidget.h"
#include "kngroupselectdialog.h"
#include "knglobals.h"
#include "knstringsplitter.h"
#include "utilities.h"

KNGroupSelectDialog::KNGroupSelectDialog(KNNntpAccount *a, QCString &groups, QWidget *parent)
	: QDialog(parent,0,true)
{
	KNStringSplitter split;
	bool splitOk;
	
	gb1=new QGroupBox(i18n("all groups"), this);
	gb2=new QGroupBox(i18n("selected groups"), this);
		
	glw=new KNGroupListWidget(a, gb1);
		
	add=new QPushButton(QString::null,this);
	add->setPixmap(UserIcon("arrow_right.xpm"));
	add->setFixedSize(50,35);
		
	del=new QPushButton(QString::null,this);
	del->setPixmap(UserIcon("arrow_left.xpm"));
	del->setFixedSize(50,35);
	
	lb=new QListBox(gb2);
		
	ok=new QPushButton(i18n("OK"), this);
	cancel=new QPushButton(i18n("Cancel"), this);
	KSeparator *sep=new KSeparator(this);
		
	QVBoxLayout *topL=new QVBoxLayout(this, 10);
	QHBoxLayout *listL=new QHBoxLayout(10);
	QVBoxLayout *btnL1=new QVBoxLayout(10);
	QHBoxLayout *btnL2=new QHBoxLayout(10);
	
	topL->addLayout(listL, 1);
	topL->addWidget(sep);
	topL->addLayout(btnL2);
	listL->addWidget(gb1, 1);
	listL->addLayout(btnL1);
	listL->addWidget(gb2, 1);
	btnL1->addStretch(1);
	btnL1->addWidget(add);
	btnL1->addStretch(1);
	btnL1->addWidget(del);
	btnL1->addStretch(1);
	btnL2->addStretch(1);
	btnL2->addWidget(ok);
	btnL2->addWidget(cancel);
	topL->activate();
		
	connect(glw, SIGNAL(itemSelected(const QString&)), this, SLOT(slotAdd(const QString&)));
	connect(add, SIGNAL(clicked()), this, SLOT(slotAddBtn()));
	connect(del, SIGNAL(clicked()), this, SLOT(slotRemoveBtn()));
	connect(lb, SIGNAL(selected(int)), this, SLOT(slotRemove(int)));
	connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
	
	if(!groups.isEmpty()) {
		split.init(groups, ",");
		splitOk=split.first();
		if(!splitOk) lb->insertItem(QString(groups));
		else {
  		while(splitOk) {
  			lb->insertItem(QString(split.string()));
  			splitOk=split.next();
  		}
  	}
	}
	
	setCaption(i18n("select newsgroups"));
	setDialogSize("gSelectDLG", this);
}



KNGroupSelectDialog::~KNGroupSelectDialog()
{
	saveDialogSize( "gSelectDLG", this->size());
}



void KNGroupSelectDialog::resizeEvent(QResizeEvent *)
{
	glw->setGeometry(20,30, gb1->width()-40, gb1->height()-50);
	lb->setGeometry(20,30, gb2->width()-40, gb2->height()-50);
}



void KNGroupSelectDialog::addToSelected(const QString &text)

{
	bool inList=false;
	for(uint idx=0; idx<lb->count(); idx++)
		if(lb->text(idx) == text) inList=true;
	if(!inList) lb->insertItem(text);
}



void KNGroupSelectDialog::slotAddBtn()
{
  if(!glw->currentText().isEmpty()) addToSelected(glw->currentText());
}


				
void KNGroupSelectDialog::slotRemoveBtn()
{
	if(lb->currentItem()>-1) lb->removeItem(lb->currentItem());
}


			
void KNGroupSelectDialog::slotAdd(const QString &text)
{
	addToSelected(text);
}



void KNGroupSelectDialog::slotRemove(int i)
{
	lb->removeItem(i);
}



QCString& KNGroupSelectDialog::selectedGroups()
{
	selGroups="";
	if(lb->count()>0) {
		selGroups=lb->text(0).local8Bit();
		if(lb->count()>1) {
  		for(uint i=1; i<lb->count(); i++){
  			selGroups+=",";
  			selGroups+=lb->text(i).local8Bit();
  		}
  	}
	}
	return selGroups;
}

// -----------------------------------------------------------------------------

#include "kngroupselectdialog.moc"
