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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>

#include <kconfig.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include "expense.h"
#include "setupDialog.h"

#include "expense-factory.moc"


extern "C"
{

void *init_conduit_expense()
{
	return new ExpenseConduitFactory;
}

} ;


const char *ExpenseConduitFactory::fGroup="Expense-conduit";
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

	KPILOT_DELETE(fAbout);
	KPILOT_DELETE(fInstance);
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
