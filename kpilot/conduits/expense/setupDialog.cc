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
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "options.h"

static const char *setupDialog_id="$Id$";


#include <qtabwidget.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include <kconfig.h>
#include <kfiledialog.h>


#include "expenseConduit.h"
#include "expense-factory.h"

#include "setupDialog.moc"

ExpenseWidgetSetup::ExpenseWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigWidget = new ExpenseWidget(widget());
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false,ExpenseConduitFactory::about());
	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());

#if defined(DEBUG) && !defined(NDEBUG)
	DEBUGCONDUIT << fname
		<< "Size of tabw="
		<< fConfigWidget->tabWidget->size()
		<< endl;
	DEBUGCONDUIT << fname
		<< "Size of conw="
		<< fConfigWidget->size()
		<< endl;
#endif

	QObject::connect(fConfigWidget->fDatabaseType,SIGNAL(clicked(int)),
		this,SLOT(slotDBPolicyChanged()));
	QObject::connect(fConfigWidget->fRotatePolicy,SIGNAL(clicked(int)),
		this,SLOT(slotRotatePolicyChanged()));

	QSize s = fConfigWidget->size() + QSize(SPACING,SPACING);
	fConfigWidget->resize(s);
	fConfigWidget->setMinimumSize(s);
}

ExpenseWidgetSetup::~ExpenseWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void ExpenseWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,ExpenseConduitFactory::group());

	fConfig->writeEntry("CSVFileName",fConfigWidget->fCSVFilename->text());

	int m = getRotatePolicy();
	fConfig->writeEntry("CSVRotatePolicy",m);
	fConfig->writeEntry("CSVRotate",
		fConfigWidget->fRotateNumber->value());

	fConfig->writeEntry("DBServer",fConfigWidget->fDBServer->text());
	fConfig->writeEntry("DBlogin",fConfigWidget->fDBLogin->text());
	fConfig->writeEntry("DBpasswd",fConfigWidget->fDBPasswd->text());
	fConfig->writeEntry("DBname",fConfigWidget->fDBName->text());
	fConfig->writeEntry("DBtable",fConfigWidget->fDBTable->text());

	m = getDBPolicy();
	fConfig->writeEntry("DBTypePolicy",m);
}

/* virtual */ void ExpenseWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,ExpenseConduitFactory::group());

	fConfigWidget->fCSVFilename->setText(fConfig->readEntry("CSVFileName"));
	int m = fConfig->readNumEntry("CSVRotatePolicy",PolicyOverwrite);
	if (m == PolicyRotate)
	{
		m=PolicyAppend;
	}
	setRotatePolicy((RotatePolicy) m);
	fConfigWidget->fRotateNumber->setValue(fConfig->readNumEntry("CSVRotate",3));

	fConfigWidget->fDBServer ->setText(fConfig->readEntry("DBServer"));
	fConfigWidget->fDBLogin ->setText(fConfig->readEntry("DBlogin"));
	fConfigWidget->fDBPasswd ->setText(fConfig->readEntry("DBpasswd"));
	fConfigWidget->fDBName ->setText(fConfig->readEntry("DBname"));
	fConfigWidget->fDBTable ->setText(fConfig->readEntry("DBtable"));
	m = fConfig->readNumEntry("DBTypePolicy",PolicyNone);
	setDBPolicy((DBPolicy) m);
}

int ExpenseWidgetSetup::getRotatePolicy() const
{
	FUNCTIONSETUP;

	int m = PolicyOverwrite;
	if (fConfigWidget->fAppend->isChecked()) m = PolicyAppend;
	if (fConfigWidget->fRotate->isChecked()) m = PolicyRotate;

	if ((m==PolicyOverwrite) && !fConfigWidget->fOverWrite->isChecked())
	{
		kdWarning() << k_funcinfo
			<< ": Unknown policy button selected."
			<< endl;
		return -1;
	}

	return m;
}

void ExpenseWidgetSetup::setRotatePolicy(RotatePolicy m)
{
	FUNCTIONSETUP;

	switch(m)
	{
	case PolicyOverwrite :
		fConfigWidget->fOverWrite->setChecked(true);
		break;
	case PolicyAppend :
		fConfigWidget->fAppend->setChecked(true);
		break;
	case PolicyRotate :
		fConfigWidget->fRotate->setChecked(true);
		break;
	default :
		kdWarning() << k_funcinfo
			<< ": Unknown rotate policy "
			<< m
			<< endl;
	}
	slotRotatePolicyChanged();
}

void ExpenseWidgetSetup::slotRotatePolicyChanged()
{
	FUNCTIONSETUP;

	int m = getRotatePolicy();
	switch(m)
	{
	case PolicyOverwrite:
	case PolicyAppend:
		fConfigWidget->fRotateNumber->setEnabled(false);
		break;
	case PolicyRotate:
		fConfigWidget->fRotateNumber->setEnabled(true);
		break;
	default:
		kdWarning() << k_funcinfo
			<< ": Unknown policy button selected -- "
			<< m
			<< endl;
	}
}

void ExpenseWidgetSetup::slotCSVBrowse()
{
        FUNCTIONSETUP;

	QString fileName = KFileDialog::getOpenFileName(0L, "*.csv");
	if(fileName.isNull()) return;
	fConfigWidget->fCSVFilename->setText(fileName);
}

int ExpenseWidgetSetup::getDBPolicy() const
{
	int m = PolicyNone;
	if (fConfigWidget->fpostgresql->isChecked()) m = PolicyPostgresql;
	if (fConfigWidget->fmysql->isChecked()) m = PolicyMysql;

	if ((m==PolicyNone) && !fConfigWidget->fnone->isChecked())
	{
		kdWarning() << k_funcinfo
			<< ": Unknown policy button selected."
			<< endl;
		return -1;
	}

	return m;
}

void ExpenseWidgetSetup::setDBPolicy(DBPolicy m)
{
	switch(m)
	{
	case PolicyNone :
		fConfigWidget->fnone->setChecked(true);
		break;
	case PolicyMysql :
		fConfigWidget->fmysql->setChecked(true);
		break;
	case PolicyPostgresql :
		fConfigWidget->fpostgresql->setChecked(true);
		break;
	default :
		kdWarning() << k_funcinfo
			<< ": Unknown policy "
			<< m
			<< " for db."
			<< endl;
	}
}



void ExpenseWidgetSetup::slotDBPolicyChanged()
{
	FUNCTIONSETUP;

	int m = getDBPolicy();
	switch(m)
	{
	case PolicyOverwrite:
	case PolicyAppend:
		fConfigWidget->fRotateNumber->setEnabled(false);
		break;
	case PolicyRotate:
		fConfigWidget->fRotateNumber->setEnabled(true);
		break;
	default:
		kdWarning() << k_funcinfo
			<< ": Unknown policy button selected -- "
			<< m
			<< endl;
	}
}

// $Log$
// Revision 1.9  2001/12/18 07:40:49  adridg
// Enable conduit's config & back out danimo's work
//
// Revision 1.8  2001/12/08 16:29:41  mlaurent
// Fix compilation.
// Dirk could you recreate a tarball for kde3.0beta1
// we can't compile it without these fix.
// Thanks
//
// Revision 1.7  2001/12/02 22:03:07  adridg
// Expense conduit finally works
//
// Revision 1.6  2001/11/25 22:03:44  adridg
// Port expense conduit to new arch. Doesn't compile yet.
//
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

// $Log$
// Revision 1.9  2001/12/18 07:40:49  adridg
// Enable conduit's config & back out danimo's work
//
// Revision 1.3  2001/12/13 21:35:12  adridg
// Gave all conduits a config dialog
//
// Revision 1.2  2001/12/02 22:03:07  adridg
// Expense conduit finally works
//
// Revision 1.1  2001/11/18 16:55:51  adridg
// Moving expenses conduit to new arch.
//
// Revision 1.2  2001/10/08 22:25:41  adridg
// Moved to libkpilot and lib-based conduits
//
// Revision 1.1  2001/10/04 23:51:55  adridg
// Nope. One more really final commit to get the alpha to build. Dirk, otherwise just remove the conduits/ subdir from kdepim/kpilot/Makefile.am
//

// $Log$
// Revision 1.9  2001/12/18 07:40:49  adridg
// Enable conduit's config & back out danimo's work
//
//
// Revision 1.2  2001/03/14 16:56:02  molnarc
//
// CJM - Added browse button on csv export tab.
// CJM - Added database export tab and required information.
//
// Revision 1.1  2001/03/04 21:47:04  adridg
// New expense conduit, non-functional but it compiles
//
