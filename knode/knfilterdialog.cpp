/***************************************************************************
                          knfilterdialog.cpp  -  description
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
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kapp.h>

#include "knfilterconfigwidget.h"
#include "knarticlefilter.h"
#include "knfilterdialog.h"
#include "utilities.h"


KNFilterDialog::
	KNFilterDialog(QWidget *parent, const char *name, KNArticleFilter *f)
	: QSemiModal(parent, name, true)
{
	fltr=f;
	if(!f->name().isEmpty()) savedName=f->name().copy();
	
	QGroupBox *gb=new QGroupBox(this);
	QLabel *l1=new QLabel(i18n("Name"), gb);
	QLabel *l2=new QLabel(i18n("apply on"), gb);
	fname=new QLineEdit(gb);
	enabled=new QCheckBox(i18n("show in menu"), gb);
	apon=new QComboBox(gb);
	apon->insertItem(i18n("single articles"));
	apon->insertItem(i18n("whole threads"));	
  		
	fw=new KNFilterConfigWidget(this);
	
	QPushButton *ok=new QPushButton(i18n("OK"), this);
	QPushButton *cancel=new QPushButton(i18n("Cancel"), this);
	QPushButton *help=new QPushButton(i18n("Help"), this);
	help->setEnabled(false);
		
	QGridLayout *gbL=new QGridLayout(gb, 2,4,10);
	gbL->addWidget(l1, 0,0);
	gbL->addMultiCellWidget(fname, 0,0,1,3);
	gbL->addWidget(enabled, 1,0);
	gbL->addWidget(l2, 1,2);
	gbL->addWidget(apon, 1,3);
	gbL->setColStretch(1,1);
	
	QVBoxLayout *topL=new QVBoxLayout(this,10);
	QHBoxLayout *btnL=new QHBoxLayout(10);
	
	topL->addWidget(gb);
	topL->addWidget(fw,1);
	topL->addLayout(btnL);
	
	btnL->addWidget(help);
	btnL->addStretch(1);
	btnL->addWidget(ok);
	btnL->addWidget(cancel);
	
	topL->activate();
		
	connect(ok, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
	connect(help, SIGNAL(clicked()), this, SLOT(slotHelp()));
	
	if(f->name().isEmpty()) setCaption(i18n("new filter"));
	else setCaption(f->name());
	
	enabled->setChecked(f->isEnabled());
	apon->setCurrentItem((int) f->applyOn());
	fname->setText(f->name());
	
	fw->status->setFilter(f->status);
	fw->lines->setFilter(f->lines);
	fw->age->setFilter(f->age);
	fw->score->setFilter(f->score);
	fw->subject->setFilter(f->subject);
	fw->from->setFilter(f->from);
		
	setDialogSize("filterDLG", this);
}



KNFilterDialog::~KNFilterDialog()
{
	saveDialogSize("filterDLG", this->size());
}



void KNFilterDialog::apply()
{
	fltr->setName(fname->text());
	
	fltr->setEnabled(enabled->isChecked());
	
	fltr->status=fw->status->filter();
	fltr->score=fw->score->filter();
	fltr->age=fw->age->filter();
	fltr->lines=fw->lines->filter();
	fltr->subject=fw->subject->filter();
	fltr->from=fw->from->filter();
	
	fltr->setApplyOn(apon->currentItem());
	
}



void KNFilterDialog::slotOK()
{
	if(!fname->text().isEmpty()) {
		apply();
		emit editDone(this);
	}
		
	else KMessageBox::error(0, i18n("Please provide a name for this filter."));
}



void KNFilterDialog::slotCancel()
{
	if(fltr->id()==-1) delete fltr;
	else fltr->setName(savedName);
	
	delete this;	 	
}



void KNFilterDialog::slotHelp()
{
	kapp->invokeHTMLHelp("working-1.html", "4.5");
}




//--------------------------------

#include "knfilterdialog.moc"
