/* abbrowser-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002-2003 Reinhold Kainhofer
**
** This file defines the factory for the abbrowser-conduit plugin.
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
#include "abbrowser-factory.moc"

#include <kinstance.h>
#include <kaboutdata.h>

#include "abbrowser-conduit.h"
#include "abbrowser-setup.h"


extern "C"
{

void *init_libaddressconduit()
{
	return new AbbrowserConduitFactory;
}

} ;


// A number of static variables; except for fAbout, they're 
// all KConfig group or entry keys.
//
//
KAboutData *AbbrowserConduitFactory::fAbout = 0L;
const char *AbbrowserConduitFactory::fGroup = "Abbrowser-conduit";

const char *AbbrowserConduitFactory::fSmartMerge = "SmartMerge";
const char *AbbrowserConduitFactory::fResolution = "ConflictResolve";
const char *AbbrowserConduitFactory::fArchive = "ArchiveDeleted";
const char *AbbrowserConduitFactory::fStreetType = "PilotStreet";
const char *AbbrowserConduitFactory::fFaxType = "PilotFax";
const char *AbbrowserConduitFactory::fSyncMode = "SyncMode";
const char *AbbrowserConduitFactory::fFirstSync = "FirstSync";
const char *AbbrowserConduitFactory::fFullSyncOnPCChange = "FullSyncOnPCChange";
const char *AbbrowserConduitFactory::fOtherField = "PilotOther";
const char *AbbrowserConduitFactory::fCustom = "Custom %1";


AbbrowserConduitFactory::AbbrowserConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("abbrowserconduit");
	fAbout = new KAboutData("abbrowserconduit",
		I18N_NOOP("Abbrowser Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Abbrowser Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Dan Pilone\n(C) 2002-2003, Reinhold Kainhofer");
	fAbout->addAuthor("Greg Stern",
		I18N_NOOP("Primary Author"));
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addAuthor("Reinhold Kainhofer", I18N_NOOP("Maintainer"),
		"reinhold@kainhofer.com", "http://reinhold.kainhofer.com");
	fAbout->addCredit("David Bishop", I18N_NOOP("UI"));
}

AbbrowserConduitFactory::~AbbrowserConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *AbbrowserConduitFactory::createObject( QObject *p,
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
			return new AbbrowserWidgetSetup(w,n,a);
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
			return new AbbrowserConduit(d,n,a);
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

