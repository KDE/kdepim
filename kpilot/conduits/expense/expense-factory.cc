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

void *init_libexpenseconduit()
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


// $Log$
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

