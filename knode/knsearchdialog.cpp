/***************************************************************************
                          knsearchdialog.cpp  -  description
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

#include "knsearchdialog.h"
#include "knarticlefilter.h"
#include "utilities.h"
#include "knglobals.h"
#include <kseparator.h>
#include <qlayout.h>
#include <qgroupbox.h>

KNSearchDialog::KNSearchDialog(searchType /*t*/) : QWidget()
{
	QGroupBox *bg=new QGroupBox(this);
	startBtn=new QPushButton(i18n("Start search"), bg);
	newBtn=new QPushButton(i18n("New search"), bg);
	closeBtn=new QPushButton(i18n("Close"), bg);
	SIZE(startBtn); SIZE(newBtn); SIZE(closeBtn);
	
	fcw=new KNFilterConfigWidget(this);
	
	QHBoxLayout *topL=new QHBoxLayout(this, 10);
	QVBoxLayout *btnL=new QVBoxLayout(bg, 10);
	
	topL->addWidget(fcw, 1);
	topL->addWidget(bg);
	
	btnL->addWidget(startBtn);
	btnL->addWidget(newBtn);
	btnL->addStretch(1);
	btnL->addWidget(closeBtn);	
	topL->activate();
		
	connect(startBtn, SIGNAL(clicked()), this, SLOT(slotStartClicked()));
	connect(newBtn, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
	connect(closeBtn, SIGNAL(clicked()), this, SLOT(slotCloseClicked()));
	
	f_ilter=new KNArticleFilter();
	f_ilter->setLoaded(true);
	//if(t==STfolderSearch) fcw->setLimited();
	
	setCaption(i18n("Search articles"));
  restoreWindowSize("searchDlg", this, sizeHint());
}



KNSearchDialog::~KNSearchDialog()
{
  delete f_ilter;
  saveWindowSize("searchDlg", size());
}



void KNSearchDialog::closeEvent(QCloseEvent *e)
{
	e->accept();
	emit dialogDone();
}



void KNSearchDialog::slotStartClicked()
{
	f_ilter->status=fcw->status->filter();
	f_ilter->score=fcw->score->filter();
	f_ilter->age=fcw->age->filter();
	f_ilter->lines=fcw->lines->filter();
	f_ilter->subject=fcw->subject->filter();
	f_ilter->from=fcw->from->filter();
	emit doSearch(f_ilter);
}



void KNSearchDialog::slotNewClicked()
{
	fcw->reset();
}



void KNSearchDialog::slotCloseClicked()
{
	emit dialogDone();
}



//--------------------------------

#include "knsearchdialog.moc"

