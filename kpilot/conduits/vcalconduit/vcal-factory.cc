/* KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the vcal-conduit plugin.
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

#include <kaboutdata.h>

#include "vcal-setup.h"
#include "vcal-conduit.h"
#include "vcal-factory.moc"
#include "vcalconduitSettings.h"

extern "C"
{

void *init_conduit_vcal()
{
	return new VCalConduitFactory;
}

}


VCalConduitSettings* VCalConduitFactory::fConfig=0L;

VCalConduitFactory::VCalConduitFactory(QObject *p, const char *n) :
	VCalConduitFactoryBase(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("vcalconduit");
	fAbout = new KAboutData("vcalConduit",
		I18N_NOOP("VCal Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the VCal Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Adriaan de Groot\n(C) 2002-2003, Reinhold Kainhofer");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Maintainer"),
		"reinhold@kainhofer.com",
		"http://reinhold.kainhofer.com/Linux/");
	fAbout->addAuthor("Dan Pilone",
		I18N_NOOP("Original Author"));
	fAbout->addAuthor("Preston Brown",
		I18N_NOOP("Original Author"));
	fAbout->addAuthor("Herwin-Jan Steehouwer",
		I18N_NOOP("Original Author"));
	fAbout->addCredit("Cornelius Schumacher",
		I18N_NOOP("iCalendar port"));
	fAbout->addCredit("Philipp Hullmann",
		I18N_NOOP("Bugfixer"));
}

VCalConduitFactory::~VCalConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
}

VCalConduitSettings* VCalConduitFactory::config()
{
	if (!fConfig) {
		fConfig = new VCalConduitSettings(CSL1("Calendar"));
		if (fConfig) fConfig->readConfig();
	}
	return fConfig;
}

/* virtual */ QObject *VCalConduitFactory::createObject( QObject *p,
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

	if (qstrcmp(c,"ConduitConfigBase")==0)
	{
		QWidget *w = dynamic_cast<QWidget *>(p);

		if (w)
		{
			return new VCalWidgetSetup(w,n);
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
	else
	if (qstrcmp(c,"SyncAction")==0)
	{
		KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

		if (d)
		{
			return new VCalConduit(d,n,a);
		}
		else
		{
			kdError() << k_funcinfo
				<< ": Couldn't cast to KPilotDeviceLink."
				<< endl;
		}
	}

	return 0L;
}
