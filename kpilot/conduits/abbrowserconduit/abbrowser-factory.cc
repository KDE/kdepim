/* abbrowser-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
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

#include <kapplication.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include <dcopclient.h>

#include <time.h> // Needed by pilot-link include

#include <pi-memo.h>

#include "abbrowser-conduit.h"
#include "abbrowser-setup.h"

#include "abbrowser-factory.moc"


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

// TODO: Get rid of these:
//const char *AbbrowserConduitFactory::fOtherMap = "PilotOther";
//const char *AbbrowserConduitFactory::fCloseAbbrowser = "CloseAbbrowser";
//const char *AbbrowserConduitFactory::fFormatName = "FormatName";



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
		"(C) 2001, Dan Pilone");
	fAbout->addAuthor("Greg Stern",
		I18N_NOOP("Primary Author"));
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addCredit("David Bishop",
		I18N_NOOP("UI"));
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


// $Log$
// Revision 1.5  2002/05/15 17:15:32  gioele
// kapp.h -> kapplication.h
// I have removed KDE_VERSION checks because all that files included "options.h"
// which #includes <kapplication.h> (which is present also in KDE_2).
// BTW you can't have KDE_VERSION defined if you do not include
// - <kapplication.h>: KDE3 + KDE2 compatible
// - <kdeversion.h>: KDE3 only compatible
//
// Revision 1.4  2002/04/16 18:22:12  adridg
// Wishlist fix from David B: handle formatted names when syncing
//
// Revision 1.3  2001/12/20 22:55:21  adridg
// Making conduits save their configuration and doing syncs
//
// Revision 1.2  2001/12/10 22:10:17  adridg
// Make the conduit compile, for Danimo, but it may not work
//
// Revision 1.1  2001/10/31 23:54:45  adridg
// CVS_SILENT: Ongoing conduits ports
//
