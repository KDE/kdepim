/***************************************************************************
                          knsenderrordialog.cpp  -  description
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

#include <klocale.h>

#include <qlayout.h>
#include "knsenderrordialog.h"
#include "knglobals.h"
#include "utilities.h"

KNSendErrorDialog::KNSendErrorDialog() : QSemiModal(xTop, 0, true)
{
	jobList.setAutoDelete(true);
	jobs=new KNListBox(this);
	error=new QLabel(this);
	error->setFrameStyle(QFrame::Box | QFrame::Sunken);
	closeBtn=new QPushButton(i18n("Close"), this);
		
	connect(jobs, SIGNAL(highlighted(int)), this, SLOT(slotJobHighlighted(int)));
	connect(closeBtn, SIGNAL(clicked()), this, SLOT(slotCloseBtnClicked()));
	
	QVBoxLayout *topL=new QVBoxLayout(this, 5,5);
	QHBoxLayout *btnL=new QHBoxLayout();
	topL->addWidget(jobs, 1);
	topL->addWidget(error);
	topL->addSpacing(20);
	topL->addLayout(btnL);
	btnL->addStretch(1);
	btnL->addWidget(closeBtn);
	topL->activate();
	
	setCaption(i18n("Errors while sending"));
	setDialogSize("sendDlg", this);
}



KNSendErrorDialog::~KNSendErrorDialog()
{
	saveDialogSize("sendDlg", this->size());
}



void KNSendErrorDialog::appendJob(KNJobData *job)
{
	static QPixmap pm=UserIcon("snderr.xpm");
	KNLBoxItem *it;
	KNSavedArticle *art=(KNSavedArticle*)job->data();
	jobList.append(job);
	it=new KNLBoxItem(art->subject(), job, &pm);
	jobs->insertItem(it);	
}



void KNSendErrorDialog::slotJobHighlighted(int idx)
{
	KNLBoxItem *it=jobs->itemAt(idx);
	error->setText(((KNJobData*)it->data())->errorString());	
}



void KNSendErrorDialog::slotCloseBtnClicked()
{
	emit dialogDone();
}
