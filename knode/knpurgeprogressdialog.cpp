/***************************************************************************
                          knpurgeprogressdialog.cpp  -  description
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


#include "knpurgeprogressdialog.h"
#include "utilities.h"
#include "knglobals.h"

#include <kseparator.h>
#include <qlayout.h>


KNPurgeProgressDialog::KNPurgeProgressDialog()
	: QFrame(0,0, WStyle_Customize | WStyle_Tool | WStyle_NoBorder)
{
	QFont fnt;
	int x, y;
	s_teps=0;
	p_rogress=0;
  	
	text=new QLabel("M", this);
	fnt=text->font();
	fnt.setBold(true);
	text->setFont(fnt);
	info=new QLabel("M", this);
	KSeparator *sep=new KSeparator(this);
	pb=new QProgressBar(this);
	//SIZE(text); SIZE(info); SIZE(sep); SIZE(pb);
	
	QVBoxLayout *topL=new QVBoxLayout(this, 10);
	topL->addWidget(text);
	topL->addWidget(sep);
	topL->addWidget(info);
	topL->addWidget(pb);
	topL->activate();
	
	setFrameStyle(WinPanel | Raised);
	setFixedSize(400,120);
	
	if(xTop->isVisible()) {
		x=(xTop->width()-400)/2;
		y=(xTop->height()-120)/2;
		if(x<0 || y<0) {
			x=0;
			y=0;
		}
		x+=xTop->x();
		y+=xTop->y();
		move(x,y);
	}	
}



KNPurgeProgressDialog::~KNPurgeProgressDialog()
{
}


void KNPurgeProgressDialog::init(const QString& txt, int st)
{
	p_rogress=0;
	s_teps=st;
	
	pb->reset();
	pb->setTotalSteps(100*s_teps);
	pb->setProgress(1);
	
	text->setText(txt);	
}

void KNPurgeProgressDialog::progress()
{
	p_rogress++;
	pb->setProgress(p_rogress*100);
}
