/***************************************************************************
                          knuserwidget.cpp  -  description
                             -------------------

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

#include <klocale.h>

#include "utilities.h"
#include "knuserwidget.h"
#include <qlayout.h>
#include <qlabel.h>
#include <kfiledialog.h>


KNUserWidget::KNUserWidget(QString title, QWidget *parent, const char *n)
	: QGroupBox(title, parent, n)
{
	entry=0;
	
	name=new QLineEdit(this);
	email=new QLineEdit(this);
	replyTo=new QLineEdit(this);
	orga=new QLineEdit(this);
	sig=new QLineEdit(this);
	sigBtn=new QPushButton(i18n("Browse..."), this);
	sigBtn->setFixedSize(sigBtn->sizeHint());
	connect(sigBtn, SIGNAL(clicked()), this, SLOT(slotSigButton()));
		
	QLabel *l1=new QLabel(i18n("Name"), this);
	QLabel *l2=new QLabel(i18n("Email"), this);
	QLabel *l3=new QLabel(i18n("Reply-To"), this);
	QLabel *l4=new QLabel(i18n("Organization"), this);
  QLabel *l5=new QLabel(i18n("Signature-File"), this);
	
	
	QGridLayout *topL=new QGridLayout(this,  6, 3, 20,5);
		
	topL->addWidget(l1, 0,0);
	topL->addMultiCellWidget(name, 0,0, 1,2);
	topL->addWidget(l2, 1,0);
	topL->addMultiCellWidget(email, 1,1, 1,2);
	topL->addWidget(l3, 2,0);
	topL->addMultiCellWidget(replyTo, 2,2, 1,2);
	topL->addWidget(l4, 3,0);
	topL->addMultiCellWidget(orga, 3,3, 1,2);
	topL->addWidget(l5, 4,0);
	topL->addWidget(sig, 4,1);
	topL->addWidget(sigBtn, 4,2);
	topL->setColStretch(1,1);
	topL->setRowStretch(5,1);
	topL->setResizeMode(QLayout::Minimum);	
	topL->activate();
}



KNUserWidget::~KNUserWidget()
{
}



void KNUserWidget::setData(KNUserEntry *user)
{
	
	entry=user;
	
	name->setText(user->name());
	email->setText(user->email());
	replyTo->setText(user->replyTo());
	orga->setText(user->orga());
	sig->setText(user->sigPath());
}



void KNUserWidget::applyData()
{
	if(entry) {
		entry->setName(name->text().local8Bit());
		entry->setEmail(email->text().local8Bit());
		entry->setReplyTo(replyTo->text().local8Bit());
		entry->setOrga(orga->text().local8Bit());
		entry->setSigPath(sig->text().local8Bit());
	}
}



void KNUserWidget::slotSigButton()
{
	QString tmp=KFileDialog::getOpenFileName();
	if(!tmp.isEmpty()) sig->setText(tmp);
}

// -----------------------------------------------------------------------------

#include "knuserwidget.moc"
