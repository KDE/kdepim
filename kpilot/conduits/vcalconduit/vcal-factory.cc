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
const char * const VCalConduitFactory::calendarFile = "CalFile" ;
const char * const VCalConduitFactory::firstTime = "FirstTime" ;
const char * const VCalConduitFactory::deleteOnPilot = "DeleteOnPilot" ;
const char * const VCalConduitFactory::group = "vcalOptions" ;


KAboutData *VCalConduitFactory::fAbout = 0L;
VCalConduitFactory::VCalConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
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
		I18N_NOOP("Primary Author"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
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

	return 0L;
}

// $Log$
// Revision 1.1  2001/12/13 21:40:40  adridg
// New files for move to .so
//

