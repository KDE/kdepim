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

#include <kconfig.h>
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


/* static */ const char *NullConduitFactory::fGroup = "Null-conduit";
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
			return new NullWidgetSetup(w,n,a);
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

NullWidgetSetup::NullWidgetSetup(QWidget *w, const char *n, 
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigWidget = new NullWidget(widget());
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false,NullConduitFactory::about());

	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
}

NullWidgetSetup::~NullWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void NullWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

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

	KConfigGroupSaver s(fConfig,NullConduitFactory::group());

	fConfig->writeEntry("LogMessage",fConfigWidget->fLogMessage->text());
	fConfig->writeEntry("Databases",fConfigWidget->fDatabases->text());
}

/* virtual */ void NullWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,NullConduitFactory::group());

	fConfigWidget->fLogMessage->setText(
		fConfig->readEntry("LogMessage",i18n("KPilot was here!")));
	fConfigWidget->fDatabases->setText(
		fConfig->readEntry("Databases"));

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Read Message="
		<< fConfigWidget->fLogMessage->text()
		<< endl;
	DEBUGCONDUIT << fname
		<< ": Read Database="
		<< fConfigWidget->fDatabases->text()
		<< endl;
#endif
}


// $Log$
// Revision 1.2  2001/10/08 22:25:41  adridg
// Moved to libkpilot and lib-based conduits
//
// Revision 1.1  2001/10/04 23:51:55  adridg
// Nope. One more really final commit to get the alpha to build. Dirk, otherwise just remove the conduits/ subdir from kdepim/kpilot/Makefile.am
//

