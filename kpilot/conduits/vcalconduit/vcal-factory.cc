/* vcal-factory.cc                      KPilot
**
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <kinstance.h>
#include <kaboutdata.h>

#include "vcal-setup.h"
#include "vcal-conduit.h"
#include "vcal-factory.moc"


extern "C"
{

void *init_libvcalconduit()
{
	return new VCalConduitFactory;
}

} ;

// Configuration keys
//
//
const char * const VCalConduitFactory::group = "vcalOptions" ;


KAboutData *VCalConduitFactory::fAbout = 0L;
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
		"(C) 2001, Adriaan de Groot");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addAuthor("Dan Pilone",
		I18N_NOOP("Original Author"));
	fAbout->addAuthor("Preston Brown",
		I18N_NOOP("Original Author"));
	fAbout->addAuthor("Herwin-Jan Steehouwer",
		I18N_NOOP("Original Author"));
	fAbout->addAuthor("Cornelius Schumacher",
		I18N_NOOP("iCalendar port"));
	fAbout->addCredit("Philipp Hullmann",
		I18N_NOOP("Bugfixer"));
}

VCalConduitFactory::~VCalConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
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

	if (qstrcmp(c,"ConduitConfig")==0)
	{
		QWidget *w = dynamic_cast<QWidget *>(p);

		if (w)
		{
			return new VCalWidgetSetup(w,n,a);
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

// $Log$
// Revision 1.5.2.1  2002/04/28 12:58:54  kainhofe
// Calendar conduit now works, no memory leaks, timezone still shifted. Todo conduit mostly works, for my large list it crashes when saving the calendar file.
//
// Revision 1.6  2002/04/20 14:21:26  kainhofe
// Alarms are now written to the palm. Some bug fixes, extensive testing. Exceptions still crash the palm ;-(((
//
// Revision 1.5  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.4  2001/12/31 09:25:05  adridg
// Cleanup, various fixes for runtime loading
//
// Revision 1.3  2001/12/28 12:56:46  adridg
// Added SyncAction, it may actually do something now.
//
// Revision 1.2  2001/12/27 16:43:36  adridg
// Fixup configuration
//
// Revision 1.1  2001/12/13 21:40:40  adridg
// New files for move to .so
//

