/***************************************************************************
                          knscoredialog.cpp  -  description
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

#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qpushbutton.h>

#include <klocale.h>

#include "knscoredialog.h"


KNScoreDialog::KNScoreDialog(short sc, QWidget *parent, const char *name ) :
	QDialog(parent,name, true)
{
	bg=new QButtonGroup(this);
	
	iBtn=new QRadioButton("0", bg);
	nBtn=new QRadioButton("50", bg);
	wBtn=new QRadioButton("100", bg);
	cBtn=new QRadioButton(i18n("custom"), bg);
	spin=new QSpinBox(0,100,1,bg);
	okBtn=new QPushButton(i18n("OK"), this);
  okBtn->setMinimumSize(okBtn->sizeHint());
	cancelBtn=new QPushButton(i18n("Cancel"), this);
	cancelBtn->setMinimumSize(cancelBtn->sizeHint());
		
	//spin->setFixedHeight(spin->sizeHint().height());
	//spin->setMinimumWidth(spin->sizeHint().width());
		
	QVBoxLayout *topL=new QVBoxLayout(this, 10);
	QHBoxLayout *btnL=new QHBoxLayout(1);
	QGridLayout *bgL=new QGridLayout(bg, 4,2,10);
		
	bgL->addWidget(iBtn, 0,0);
	bgL->addWidget(nBtn, 1,0);
	bgL->addWidget(wBtn, 2,0);
	bgL->addWidget(cBtn, 3,0); bgL->addWidget(spin, 3,1);
	topL->addWidget(bg, 1);
	topL->addLayout(btnL);
	btnL->addWidget(okBtn);
	btnL->addWidget(cancelBtn);
	topL->activate();
	
	this->resize(this->minimumSize());
	
	connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
	connect(cBtn, SIGNAL(toggled(bool)), spin, SLOT(setEnabled(bool)));
	
	int b;
	
	switch(sc) {
	
		case 0: 	b=0; break;
		case 50: 	b=1; break;
		case 100:	b=2; break;
		default:	b=3; break;
	}
	
	spin->setValue(sc);
	bg->setButton(b);
	spin->setEnabled(cBtn->isChecked());
	
	setCaption(i18n("score"));
}



KNScoreDialog::~KNScoreDialog()
{
}



short KNScoreDialog::score()
{
	short ret;
	
	if(iBtn->isChecked()) 			ret=0;
	else if(nBtn->isChecked()) 	ret=50;
	else if(wBtn->isChecked()) 	ret=100;
	else if(cBtn->isChecked()) 	ret=spin->value();
	
	return ret;
}



//--------------------------------

#include "knscoredialog.moc"



