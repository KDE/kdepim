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

#include <stream.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kconfig.h>

#include "kpilotConfig.h"
#include "setupDialog.moc"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *setupDialog_id="$Id$";

ExpenseCSVPage::ExpenseCSVPage(setupDialog *p,KConfig& c) :
	setupDialogPage(i18n("CSV Export"),p)
{
	FUNCTIONSETUP;

	QGridLayout *grid = new QGridLayout(this,3,3,0,SPACING);
	QLabel *l = new QLabel(i18n("CSV Filename:"),this);
	grid->addWidget(l,1,1);

	fCSVFileName = new QLineEdit(this);
	fCSVFileName->setText(c.readEntry("CSVFileName"));
	grid->addWidget(fCSVFileName,1,2);

	QVButtonGroup *g = new QVButtonGroup(i18n("Rotate Policy"),this);

	fOverWrite = new QRadioButton(i18n("&Overwrite"),g);
	fAppend = new QRadioButton(i18n("&Append"),g);
	fRotate = new QRadioButton(i18n("&Rotate"),g);

	int m = c.readNumEntry("CSVRotatePolicy",PolicyOverwrite);
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
		kdWarning() << __FUNCTION__
			<< ": Unknown rotate policy "
			<< m
			<< endl;
	}

	connect(g,SIGNAL(clicked(int)),
		this,SLOT(slotPolicyChanged()));


	QHBox *h = new QHBox(g);
	l = new QLabel(i18n("Rotate Depth:"),h);
	fRotateNumber = new QSpinBox(1,10,1,h);
	fRotateNumber->setValue(c.readNumEntry("CSVRotate",3));

	grid->addWidget(g,2,2);

	slotPolicyChanged();

	grid->addRowSpacing(0,SPACING);
	grid->addColSpacing(3,SPACING);
	grid->setRowStretch(4,100);
}

int ExpenseCSVPage::getPolicy() const
{
	int m = PolicyOverwrite;
	if (fAppend->isChecked()) m = PolicyAppend;
	if (fRotate->isChecked()) m = PolicyRotate;

	if ((m==PolicyOverwrite) && !fOverWrite->isChecked())
	{
		kdWarning() << __FUNCTION__
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
		kdWarning() << __FUNCTION__
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


/* static */ const char *ExpenseOptions::ExpenseGroup("conduitExpense");

ExpenseOptions::ExpenseOptions(QWidget *parent) :
	setupDialog(parent,ExpenseGroup,0L)
{
	FUNCTIONSETUP;
	KConfig& c = KPilotConfig::getConfig(ExpenseGroup);

	addPage(new ExpenseCSVPage(this,c));
	addPage(new setupInfoPage(this));
	setupWidget();

	(void) setupDialog_id;
}

  
// $Log$
