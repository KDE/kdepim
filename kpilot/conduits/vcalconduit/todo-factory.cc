/* todo-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the todo-conduit plugin.
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

#include "korganizertodoConduit.h"
#include "todo-factory.moc"


extern "C"
{

void *init_libtodoconduit()
{
	return new ToDoConduitFactory;
}

} ;


KAboutData *ToDoConduitFactory::fAbout = 0L;
ToDoConduitFactory::ToDoConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("todoconduit");
	fAbout = new KAboutData("todoConduit",
		I18N_NOOP("ToDo Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the ToDo Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Adriaan de Groot");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Primary Author"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
}

ToDoConduitFactory::~ToDoConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *ToDoConduitFactory::createObject( QObject *p,
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
			return new ToDoWidgetSetup(w,n,a);
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

	return 0L;
}

ToDoWidgetSetup::ToDoWidgetSetup(QWidget *w, const char *n, 
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigWidget = new ToDoWidget(widget());
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false,ToDoConduitFactory::about());

	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
}

ToDoWidgetSetup::~ToDoWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void ToDoWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,"Null-conduit");

#if 0
	fConfig->writeEntry("LogMessage",fConfigWidget->fLogMessage->text());
	fConfig->writeEntry("Databases",fConfigWidget->fDatabases->text());
#endif
}

/* virtual */ void ToDoWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,"Null-conduit");

#if 0
	fConfigWidget->fLogMessage->setText(
		fConfig->readEntry("LogMessage",i18n("KPilot was here!")));
	fConfigWidget->fDatabases->setText(
		fConfig->readEntry("Databases"));

#endif
}


// $Log$

