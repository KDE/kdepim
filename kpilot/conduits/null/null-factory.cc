/* null-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the null-conduit plugin.
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

#include <kinstance.h>
#include <kaboutdata.h>

#include "setup_base.h"

#include "null-factory.moc"


extern "C"
{

void *init_libnullconduit()
{
	return new NullConduitFactory;
}

} ;


KAboutData *NullConduitFactory::fAbout = 0L;
NullConduitFactory::NullConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("nullconduit");
	fAbout = new KAboutData("nullConduit",
		I18N_NOOP("Null Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Null Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Adriaan de Groot");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Primary Author"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
}

NullConduitFactory::~NullConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *NullConduitFactory::createObject( QObject *p,
	const char *n,
	const char *c,
	const QStringList &a)
{
	FUNCTIONSETUP;

	if (qstrcmp(c,"QObject")==0)
	{
		// Conduit objects
		//
		return 0L;
	}
	else if (qstrcmp(c,"QWidget")==0)
	{
		QWidget *w = dynamic_cast<QWidget *>(p);

		if (w)
		{
			return new NullWidgetSetup(w,n,
				!a.contains("nonmodal"));
		}
		else 
		{
			return 0L;
		}
	}

	return 0L;
}

NullWidgetSetup::NullWidgetSetup(QWidget *w, const char *n, bool b) :
	UIDialog(w,n,b)
{
	FUNCTIONSETUP;

	fConfigWidget = new NullWidget(widget());
	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
	setTabWidget(fConfigWidget->tabWidget);

	addAboutPage(NullConduitFactory::about());
}

NullWidgetSetup::~NullWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void NullWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Message="
		<< fConfigWidget->fLogMessage->text()
		<< endl;
	DEBUGCONDUIT << fname
		<< ": Databases="
		<< fConfigWidget->fDatabases->text()
		<< endl;
#endif
}


// $Log:$

