/* setupDialog.cc			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the Expense conduit.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
#include "options.h"

#include <iostream.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>

#include "setupDialog.moc"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *setupDialog_id="$Id$";

ExpenseCSVPage::ExpenseCSVPage(QWidget *parent) :
	QWidget(parent,"ExpenseCSV")
{
	FUNCTIONSETUP;

	QGridLayout *grid = new QGridLayout(this,3,3,0,SPACING);
	QLabel *l = new QLabel(i18n("CSV Filename:"),this);
	grid->addWidget(l,1,1);

	fBrowseButton = new QPushButton(i18n("Browse"), this);
	fBrowseButton->adjustSize();
  	connect(fBrowseButton, SIGNAL(clicked()), this, SLOT(slotBrowse()));
	grid->addWidget(fBrowseButton,1,3);

	fCSVFileName = new QLineEdit(this);
	grid->addWidget(fCSVFileName,1,2);

	QVButtonGroup *g = new QVButtonGroup(i18n("Rotate Policy"),this);

	fOverWrite = new QRadioButton(i18n("&Overwrite"),g);
	fAppend = new QRadioButton(i18n("&Append"),g);
	fRotate = new QRadioButton(i18n("&Rotate"),g);



	connect(g,SIGNAL(clicked(int)),
		this,SLOT(slotPolicyChanged()));


	QHBox *h = new QHBox(g);
	l = new QLabel(i18n("Rotate Depth:"),h);
	fRotateNumber = new QSpinBox(1,10,1,h);

	grid->addMultiCellWidget(g,2,2,1,3);

	grid->addColSpacing(0,SPACING);
	grid->addRowSpacing(0,SPACING);
	grid->addColSpacing(4,SPACING);
	grid->setRowStretch(4,100);
}

void ExpenseCSVPage::slotBrowse()
{
        FUNCTIONSETUP;
 
	QString fileName = KFileDialog::getOpenFileName(0L, "*.csv");
	if(fileName.isNull()) return;
	fCSVFileName->setText(fileName);
}

int ExpenseCSVPage::getPolicy() const
{
	FUNCTIONSETUP;

	int m = PolicyOverwrite;
	if (fAppend->isChecked()) m = PolicyAppend;
	if (fRotate->isChecked()) m = PolicyRotate;

	if ((m==PolicyOverwrite) && !fOverWrite->isChecked())
	{
		kdWarning() << k_funcinfo
			<< ": Unknown policy button selected."
			<< endl;
		return -1;
	}

	return m;
}

void ExpenseCSVPage::setPolicy(RotatePolicy m)
{
	FUNCTIONSETUP;

	switch(m)
	{
	case PolicyOverwrite :
		fOverWrite->setChecked(true);
		break;
	case PolicyAppend :
		fAppend->setChecked(true);
		break;
	case PolicyRotate :
		fRotate->setChecked(true);
		break;
	default :
		kdWarning() << k_funcinfo
			<< ": Unknown rotate policy "
			<< m
			<< endl;
	}
	slotPolicyChanged();
}

void ExpenseCSVPage::readSettings(KConfig &c)
{
	fCSVFileName->setText(c.readEntry("CSVFileName"));
	int m = c.readNumEntry("CSVRotatePolicy",PolicyOverwrite);
	setPolicy((RotatePolicy) m);
	fRotateNumber->setValue(c.readNumEntry("CSVRotate",3));
}

int ExpenseDBPage::getPolicy() const
{
	int m = PolicyNone;
	if (fpostgresql->isChecked()) m = PolicyPostgresql;
	if (fmysql->isChecked()) m = PolicyMysql;

	if ((m==PolicyNone) && !fnone->isChecked())
	{
		kdWarning() << k_funcinfo
			<< ": Unknown policy button selected."
			<< endl;
		return -1;
	}

	return m;
}

void ExpenseCSVPage::slotPolicyChanged()
{
	FUNCTIONSETUP;

	int m = getPolicy();
	switch(m)
	{
	case PolicyOverwrite:
	case PolicyAppend:
		fRotateNumber->setEnabled(false);
		break;
	case PolicyRotate:
		fRotateNumber->setEnabled(true);
		break;
	default:
		kdWarning() << k_funcinfo
			<< ": Unknown policy button selected -- "
			<< m
			<< endl;
	}
}

int ExpenseCSVPage::commitChanges(KConfig& c)
{
	FUNCTIONSETUP;

	c.writeEntry("CSVFileName",fCSVFileName->text());
	
	int m = getPolicy();
	c.writeEntry("CSVRotatePolicy",m);
	c.writeEntry("CSVRotate",
		fRotateNumber->value());

	return 0;
}

int ExpenseDBPage::commitChanges(KConfig& c)
{
	FUNCTIONSETUP;

	c.writeEntry("DBServer",fDBServer->text());
	c.writeEntry("DBlogin",fDBlogin->text());
	c.writeEntry("DBpasswd",fDBpasswd->text());
	c.writeEntry("DBname",fDBname->text());
	c.writeEntry("DBtable",fDBtable->text());
	
	int m = getPolicy();
	c.writeEntry("DBTypePolicy",m);

	return 0;
}


ExpenseDBPage::ExpenseDBPage(QWidget *parent) :
	QWidget(parent,"ExpenseDB")
{
	FUNCTIONSETUP;


	QGridLayout *grid = new QGridLayout(this,3,3,0,SPACING);
	
	QLabel *ld1 = new QLabel(i18n("DB Server:"),this);
	grid->addWidget(ld1,2,1);

	fDBServer = new QLineEdit(this);
	grid->addWidget(fDBServer,2,2);

	QLabel *ld2 = new QLabel(i18n("DB Login:"),this);
	grid->addWidget(ld2,3,1);

	fDBlogin = new QLineEdit(this);
	grid->addWidget(fDBlogin,3,2);

	QLabel *ld3 = new QLabel(i18n("DB Passwd:"),this);
	grid->addWidget(ld3,4,1);

	fDBpasswd = new QLineEdit(this);
	grid->addWidget(fDBpasswd,4,2);

	QLabel *ld4 = new QLabel(i18n("DB Name:"),this);
	grid->addWidget(ld4,5,1);

	fDBname = new QLineEdit(this);
	grid->addWidget(fDBname,5,2);


	QLabel *ld5 = new QLabel(i18n("DB Table:"),this);
	grid->addWidget(ld5,6,1);

	fDBtable = new QLineEdit(this);
	grid->addWidget(fDBtable,6,2);


	QVButtonGroup *gt = new QVButtonGroup(i18n("Database Type"),this);

	fnone = new QRadioButton(i18n("&None"),gt);
	fpostgresql = new QRadioButton(i18n("&PostgreSQL"),gt);
	fmysql = new QRadioButton(i18n("&MySQL"),gt);

	grid->addMultiCellWidget(gt,1,1,1,2);


	connect(gt,SIGNAL(clicked(int)),
		this,SLOT(slotPolicyChanged()));

	grid->addRowSpacing(0,SPACING);
	grid->addColSpacing(0,SPACING);
	grid->addColSpacing(4,SPACING);
	grid->addRowSpacing(7,SPACING);
	grid->setRowStretch(7,100);
}

void ExpenseDBPage::readSettings(KConfig &c)
{
	fDBServer ->setText(c.readEntry("DBServer"));
	fDBlogin ->setText(c.readEntry("DBlogin"));
	fDBpasswd ->setText(c.readEntry("DBpasswd"));
	fDBname ->setText(c.readEntry("DBname"));
	fDBtable ->setText(c.readEntry("DBtable"));
	int m = c.readNumEntry("DBTypePolicy",PolicyNone);
	setPolicy((DBTypePolicy) m);
}

void ExpenseDBPage::setPolicy(DBTypePolicy m)
{
	switch(m)
	{
	case PolicyNone :
		fnone->setChecked(true);
		break;
	case PolicyPostgresql :
		fpostgresql->setChecked(true);
		break;
	case PolicyMysql :
		fmysql->setChecked(true);
		break;
	default :
		kdWarning() << k_funcinfo
			<< ": Unknown rotate policy "
			<< m
			<< endl;
	}

	slotPolicyChanged();
}

void ExpenseDBPage::slotPolicyChanged()
{
	FUNCTIONSETUP;

	int m = getPolicy();
	switch(m)
	{
	case PolicyNone:
	case PolicyPostgresql:
	case PolicyMysql:
		break;
	default:
		kdWarning() << k_funcinfo
			<< ": Unknown policy button selected -- "
			<< m
			<< endl;
	}
}

  
// $Log$
// Revision 1.5  2001/10/10 17:01:15  mueller
// CVS_SILENT: fixincludes
//
// Revision 1.4  2001/03/27 11:10:38  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
// Revision 1.3  2001/03/24 16:10:11  adridg
// Minor beautification
//
// Revision 1.2  2001/03/14 16:56:02  molnarc
//
// CJM - Added browse button on csv export tab.
// CJM - Added database export tab and required information.
//
// Revision 1.1  2001/03/04 21:47:04  adridg
// New expense conduit, non-functional but it compiles
//
