/***************************************************************************
                     kngroupdialog.cpp - description
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

#include "kngroupdialog.h"
#include "kngrouplistwidget.h"
#include "knglobals.h"
#include "utilities.h"

KNGroupDialog::KNGroupDialog(KNNntpAccount *a, QWidget *parent) :
	QSemiModal(parent, 0, true)
{
	mSub=new QStrList(true);
	mSub->setAutoDelete(true);
	
	mUnsub=new QStrList(true);
	mUnsub->setAutoDelete(true);
			
	glw=new KNGroupListWidget(a, this);
	mActive=glw->activeList();
	
	ok=new QPushButton(i18n("OK"), this);
	cancel=new QPushButton(i18n("Cancel"), this);
	newList=new QPushButton(i18n("new list"), this);
	help=new QPushButton(i18n("Help"), this);
	 	
	QVBoxLayout *topLayout=new QVBoxLayout(this, 10);
	QHBoxLayout *buttons=new QHBoxLayout(10);
	
	topLayout->addWidget(glw,1);
	topLayout->addLayout(buttons,0);
	
	buttons->addWidget(help);
	buttons->addStretch(1);
	buttons->addWidget(newList);
	buttons->addSpacing(10);
	buttons->addWidget(ok);
	buttons->addWidget(cancel);
	
	topLayout->activate();
	
	connect(ok, SIGNAL(clicked()), this, SLOT(slotOk()));
	connect(cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
	connect(help, SIGNAL(clicked()), this, SLOT(slotHelp()));
	connect(newList, SIGNAL(clicked()), this, SLOT(slotNewList()));
	connect(glw, SIGNAL(itemSelected(const QString&)), this, SLOT(slotItemSelected(const QString&)));
  	
	setCaption(i18n("Newsgroups"));
	setDialogSize("groupDLG", this);
}



KNGroupDialog::~KNGroupDialog()
{
	delete mSub;
	delete mUnsub;
	
	saveDialogSize("groupDLG", this->size());
}



void KNGroupDialog::slotItemSelected(const QString &text)
{
		
	if(mActive->remove(text.latin1())) {
  	if(!mSub->remove(text.latin1())) mUnsub->append(text.latin1());
//  	glw->setPixmap(-1,false);
  } else {
		mActive->append(text.latin1());
		if(!mUnsub->remove(text.latin1())) mSub->append(text.latin1());
//		glw->setPixmap(-1,true);
	}	
}



void KNGroupDialog::slotHelp()
{
	kapp->invokeHTMLHelp("knode/working-1.html", "4.2");
}



void KNGroupDialog::slotOk()
{
	emit dialogDone(true);
}



void KNGroupDialog::slotCancel()
{
  emit dialogDone(false);
}



void KNGroupDialog::slotNewList()
{
	emit getNewList(glw->account());
}
