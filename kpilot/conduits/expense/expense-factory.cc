/* expense-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the expense-conduit plugin.
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

#include <qtabwidget.h>
#include <qlineedit.h>

#include <kconfig.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include "setupDialog.h"
#include "expense.h"

#include "expense-factory.moc"


extern "C"
{

void *init_libexpenseconduit()
{
	return new ExpenseConduitFactory;
}

} ;


KAboutData *ExpenseConduitFactory::fAbout = 0L;
ExpenseConduitFactory::ExpenseConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("expenseconduit");
	fAbout = new KAboutData("expenseConduit",
		I18N_NOOP("Expense Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Expense Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Adriaan de Groot and Chris Molnar");
	fAbout->addAuthor("Christopher Molnar",
		I18N_NOOP("Primary Author"));
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
}

ExpenseConduitFactory::~ExpenseConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *ExpenseConduitFactory::createObject( QObject *p,
	const char *n,
	const char *c,
	const QStringList &a)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Creating object of class "
		<< c
		<< endl;
#endif

	if (qstrcmp(c,"ConduitConfig")==0)
	{
		QWidget *w = dynamic_cast<QWidget *>(p);

		if (w)
		{
			return new ExpenseWidgetSetup(w,n,a);
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname
				<< ": Couldn't cast parent to widget."
				<< endl;
#endif
			return 0L;
		}
	}

	if (qstrcmp(c,"SyncAction")==0)
	{
		KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

		if (d)
		{
			return new ExpenseConduit(d,n,a);
		}
		else
		{
			kdError() << k_funcinfo
				<< ": Couldn't cast to KPilotDeviceLink"
				<< endl;
			return 0L;
		}
	}
	return 0L;
}

ExpenseWidgetSetup::ExpenseWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	QTabWidget *t = new QTabWidget(widget());
	fCSVPage = new ExpenseCSVPage(t);
	t->addTab(fCSVPage,i18n("CSV Export"));
	fDBPage = new ExpenseDBPage(t);
	t->addTab(fDBPage,i18n("DB Export"));

	setTabWidget(t);
	addAboutPage(false,ExpenseConduitFactory::about());

	t->adjustSize();
}

ExpenseWidgetSetup::~ExpenseWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void ExpenseWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,"Expense-conduit");

	fCSVPage->commitChanges(*fConfig);
	fDBPage->commitChanges(*fConfig);
}

/* virtual */ void ExpenseWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,"Expense-conduit");

	fCSVPage->readSettings(*fConfig);
	fDBPage->readSettings(*fConfig);
}


// $Log$
// Revision 1.1  2001/11/18 16:55:51  adridg
// Moving expenses conduit to new arch.
//
// Revision 1.2  2001/10/08 22:25:41  adridg
// Moved to libkpilot and lib-based conduits
//
// Revision 1.1  2001/10/04 23:51:55  adridg
// Nope. One more really final commit to get the alpha to build. Dirk, otherwise just remove the conduits/ subdir from kdepim/kpilot/Makefile.am
//

