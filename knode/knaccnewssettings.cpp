/***************************************************************************
                          knaccnewssettings.cpp  -  description
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
#include <qpushbutton.h>

#include <klocale.h>
#include <kconfig.h>

#include "knaccnewssettings.h"
#include "knlistbox.h"
#include "knaccountmanager.h"
#include "utilities.h"


KNAccNewsSettings::KNAccNewsSettings(QWidget *p, KNAccountManager *am)
	: KNSettingsWidget(p)
{
	QGroupBox *gb=new QGroupBox(i18n("Accounts"), this);
	QLabel *l1, *l2, *l3, *l4, *l5;
	lb=new KNListBox(gb);
	lb->setFocusPolicy(NoFocus);
	addBtn=new QPushButton(i18n("Add"), gb);
	delBtn=new QPushButton(i18n("Delete"), gb);
			
	n_ame=new QLineEdit(this);
	s_erver=new QLineEdit(this);
	p_ort=new QSpinBox(0, 99999, 1, this);
	u_ser=new QLineEdit(this);
	p_ass=new QLineEdit(this);
	p_ass->setEchoMode(QLineEdit::Password);
	okBtn=new QPushButton(i18n("Ok"), this);
	logonCB=new QCheckBox(i18n("logon required"), this);
	l1=new QLabel(i18n("Name"), this);
	l2=new QLabel(i18n("Server"), this);
	l3=new QLabel(i18n("Port"), this);
	l4=new QLabel(i18n("User"), this);
	l5=new QLabel(i18n("Pass"), this);
  	
	QVBoxLayout *topL=new QVBoxLayout(this, 10);
	QGridLayout *gbL=new QGridLayout(gb, 3,2, 20,10);
	QGridLayout *serL=new QGridLayout(6,4, 5);
	
	topL->addWidget(gb, 1);
	topL->addLayout(serL);
	gbL->addMultiCellWidget(lb, 0,2, 0,0);
	gbL->addWidget(addBtn, 0,1);
	gbL->addWidget(delBtn, 1,1);
	gbL->setRowStretch(2, 1);
	gbL->setColStretch(0, 1);
	serL->addWidget(l1, 0,0);
	serL->addMultiCellWidget(n_ame, 0,0, 1,2);
	serL->addWidget(okBtn, 0,3);
	serL->addWidget(l2, 1,0);
	serL->addMultiCellWidget(s_erver, 1,1, 1,2);
	serL->addWidget(l3, 2,0);
	serL->addWidget(p_ort, 2,1);
	serL->addMultiCellWidget(logonCB, 3,3, 0,3);
	serL->addWidget(l4, 4,0);
	serL->addMultiCellWidget(u_ser, 4,4, 1,2);
	serL->addWidget(l5, 5,0);
	serL->addMultiCellWidget(p_ass, 5,5, 1,2);
	serL->setColStretch(2,1);
	topL->setResizeMode(QLayout::Minimum);
	topL->activate();
	
	connect(lb, SIGNAL(highlighted(int)), this, SLOT(slotItemSelected(int)));
	connect(addBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
	connect(delBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
	connect(okBtn, SIGNAL(clicked()), this, SLOT(slotOkBtnClicked()));
	connect(logonCB, SIGNAL(toggled(bool)), this, SLOT(slotLogonChecked(bool)));
	
	pm=UserIcon("server");
	aManager=am;
	am->startConfig(this);
	currentItem=-1;
	enableEdit(false);
	slotItemSelected(-1);	
}



KNAccNewsSettings::~KNAccNewsSettings()
{
	aManager->endConfig();
}



void KNAccNewsSettings::addItem(KNNntpAccount *a)
{
	KNLBoxItem *it;
	it=new KNLBoxItem(a->name(), a, &pm);
	lb->insertItem(it);
}



void KNAccNewsSettings::removeItem(KNNntpAccount *a)
{
	KNLBoxItem *it;
	for(uint i=0; i<lb->count(); i++) {
		it=lb->itemAt(i);
		if(it && it->data()==a) {
			lb->removeItem(i);
			break;
		}
	}	
}



void KNAccNewsSettings::enableEdit(bool b)
{
	n_ame->clear();
	s_erver->clear();
	p_ort->setValue(119);
	u_ser->clear();
	p_ass->clear();
	
	if(n_ame->isEnabled()!=b) {
		n_ame->setEnabled(b);
		p_ort->setEnabled(b);
		okBtn->setEnabled(b);
		logonCB->setEnabled(b);
		u_ser->setEnabled(logonCB->isChecked());
		p_ass->setEnabled(logonCB->isChecked());
		if(!b) {
			logonCB->setChecked(false);
			currentItem=-1;
			lb->clearSelection();
		}
	}
}


void KNAccNewsSettings::slotItemSelected(int id)
{
	currentItem=id;
	KNNntpAccount *a;
	s_erver->setEnabled(false);
	if(id==-1) enableEdit(false);
	else {
		enableEdit(true);
		a=(KNNntpAccount*)lb->itemAt(id)->data();
		n_ame->setText(a->name());
		s_erver->setText(a->server());
		p_ort->setValue(a->port());
		logonCB->setChecked(a->needsLogon());
		u_ser->setText(a->user());
		p_ass->setText(a->pass());
	}		
}



void KNAccNewsSettings::slotAddBtnClicked()
{
	enableEdit(true);
	s_erver->setEnabled(true);
	currentItem=-1;
	p_ort->setValue(119);
}



void KNAccNewsSettings::slotDelBtnClicked()
{
	if (currentItem < 0)
		return;
	KNNntpAccount *a;
	a=(KNNntpAccount*)(lb->itemAt(currentItem)->data());
	aManager->removeAccount(a);
	enableEdit(false);		
}



void KNAccNewsSettings::slotOkBtnClicked()
{
	KNLBoxItem *it;
	KNNntpAccount *a;
	if(currentItem==-1) {
		aManager->newAccount();
		s_erver->setEnabled(false);
	}		
	else {
		a=(KNNntpAccount*)lb->itemAt(currentItem)->data();
		aManager->applySettings(a);
		it=new KNLBoxItem(a->name(), a, &pm);
		lb->changeItem(it, currentItem);
	}
	enableEdit(false);
}



void KNAccNewsSettings::slotLogonChecked(bool b)
{
	u_ser->setEnabled(b);
	p_ass->setEnabled(b);
}

//--------------------------------

#include "knaccnewssettings.moc"
