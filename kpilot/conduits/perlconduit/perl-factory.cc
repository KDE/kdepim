/* perl-factory.cc                      KPilot
**
** Copyright (C) 2004 by Adriaan de Groot
**
** This file defines the factory for the perl-conduit plugin.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <kconfig.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include "perl-conduit.h"
#include "perl-factory.moc"


extern "C"
{

void *init_conduit_perl()
{
	return new PerlConduitFactory;
}

}

KAboutData *PerlConduitFactory::fAbout = 0L;
PerlConduitFactory::PerlConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("perlconduit");
	fAbout = new KAboutData("perlConduit",
		I18N_NOOP("Perl Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Perl Conduit for KPilot"),
		KAboutData::License_LGPL,
		"(C) 2004, Adriaan de Groot");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Primary Author"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/");
}

PerlConduitFactory::~PerlConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *PerlConduitFactory::createObject( QObject *p,
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

	if (qstrcmp(c,"SyncAction")==0)
	{
		KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

		if (d)
		{
			return new PerlConduit(d,n,a);
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


