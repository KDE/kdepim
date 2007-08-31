/* Time-factory.cc                      KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the MAL-conduit plugin.
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
**
**
** Specific permission is granted for this code to be linked to libmal
** (this is necessary because the libmal license is not GPL-compatible).
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h" 

#include <kapplication.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include <time.h> // Needed by pilot-link include
#include "mal-conduit.h"
#include "mal-setup.h"

#include "mal-factory.moc"


extern "C"
{

void *init_conduit_mal()
{
	return new MALConduitFactory;
}

unsigned long version_conduit_mal = Pilot::PLUGIN_API;

}


// A number of static variables
//
KAboutData *MALConduitFactory::fAbout = 0L;

MALConduitFactory::MALConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("MALconduit");
	fAbout = new KAboutData("MALconduit",
		I18N_NOOP("MAL Synchronization Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Synchronizes the content from MAL Servers like AvantGo to the Handheld"),
		KAboutData::License_GPL,
		"(C) 2002, Reinhold Kainhofer");
	fAbout->addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Primary Author"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com/");
	fAbout->addCredit("Jason Day",
		I18N_NOOP("Author of libmal and the JPilot AvantGo conduit"), "jasonday@worldnet.att.net");
	fAbout->addCredit("Tom Whittaker",
		I18N_NOOP("Author of syncmal"), "tom@tomw.org", "http://www.tomw.org/");
	fAbout->addCredit("AvantGo, Inc.",
		I18N_NOOP("Authors of the malsync library (c) 1997-1999"), "", "http://www.avantgo.com/");
}

MALConduitFactory::~MALConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *MALConduitFactory::createObject( QObject *p,
	const char *n,
	const char *c,
	const QStringList &a)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Creating object of class "
		<< c
		<< endl;
#endif

	if (qstrcmp(c,"ConduitConfigBase")==0)
	{
		QWidget *w = dynamic_cast<QWidget *>(p);

		if (w)
		{
			return new MALWidgetSetup(w,n);
		}
		else 
		{
			WARNINGKPILOT
				<< "Couldn't cast parent to widget."
				<< endl;
			return 0L;
		}
	}

	if (qstrcmp(c,"SyncAction")==0)
	{ 
		KPilotLink *d = dynamic_cast<KPilotLink *>(p);

		if (d)
		{
			return new MALConduit(d,n,a);
		}
		else
		{
			WARNINGKPILOT
				<< "Couldn't cast parent to KPilotLink"
				<< endl;
			return 0L;
		}
	}

	return 0L;
}

