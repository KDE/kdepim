/* Time-factory.cc                      KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the Time-conduit plugin.
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

#include <kapplication.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include <time.h> // Needed by pilot-link include
#include "time-conduit.h"
#include "time-setup.h"

#include "time-factory.moc"


extern "C"
{

void *init_conduit_time()
{
	return new TimeConduitFactory;
}

} ;


// A number of static variables; except for fAbout, they're 
// all KConfig group or entry keys.
//
//
KAboutData *TimeConduitFactory::fAbout = 0L;
const char *TimeConduitFactory::fGroup = "Time-conduit";
const char *TimeConduitFactory::fDirection = "Direction";


TimeConduitFactory::TimeConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("Timeconduit");
	fAbout = new KAboutData("Timeconduit",
		I18N_NOOP("Time Synchronization Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Synchronizes the Time on the Handheld and the PC"),
		KAboutData::License_GPL,
		"(C) 2002, Reinhold Kainhofer");
	fAbout->addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Primary Author"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com/");
}

TimeConduitFactory::~TimeConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *TimeConduitFactory::createObject( QObject *p,
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
			return new TimeWidgetConfig(w,"ConduitConfigBase");
		}
		else
		{
			return 0L;
		}
	}
	else
	if (qstrcmp(c,"ConduitConfig")==0)
	{
		QWidget *w = dynamic_cast<QWidget *>(p);

		if (w)
		{
			return new TimeWidgetSetup(w,n,a);
		}
		else 
		{
			kdError() << k_funcinfo
				<< ": Couldn't cast parent to widget."
				<< endl;
			return 0L;
		}
	}

	if (qstrcmp(c,"SyncAction")==0)
	{
		KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

		if (d)
		{
			return new TimeConduit(d,n,a);
		}
		else
		{
			kdError() << k_funcinfo
				<< ": Couldn't cast parent to KPilotDeviceLink"
				<< endl;
			return 0L;
		}
	}

	return 0L;
}

