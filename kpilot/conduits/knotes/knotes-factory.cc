/* knotes-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the knotes-conduit plugin.
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

#if KDE_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif
#include <kinstance.h>
#include <kaboutdata.h>

#include <dcopclient.h>

#include <time.h> // Needed by pilot-link include

#include <pi-memo.h>

#include "knotes-action.h"
#include "knotes-setup.h"

#include "knotes-factory.moc"


extern "C"
{

void *init_libknotesconduit()
{
	return new KNotesConduitFactory;
}

} ;


/* static */ KAboutData *KNotesConduitFactory::fAbout = 0L;

const char * const KNotesConduitFactory::group = "KNotes-conduit";
const char * const KNotesConduitFactory::matchDeletes = "DeleteNoteForMemo";

KNotesConduitFactory::KNotesConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("knotesconduit");
	fAbout = new KAboutData("knotesconduit",
		I18N_NOOP("KNotes Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the KNotes Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Adriaan de Groot");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Primary Author"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addCredit("David Bishop",
		I18N_NOOP("UI"));
}

KNotesConduitFactory::~KNotesConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *KNotesConduitFactory::createObject( QObject *p,
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
			return new KNotesWidgetSetup(w,n,a);
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
			return new KNotesAction(d,n,a);
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
// Revision 1.7  2001/10/31 23:46:51  adridg
// CVS_SILENT: Ongoing conduits ports
//
// Revision 1.6  2001/10/16 21:44:53  adridg
// Split up some files, added behavior
//
// Revision 1.5  2001/10/11 10:13:27  cschumac
// Compile fixes.
//
// Revision 1.4  2001/10/10 22:39:49  adridg
// Some UI/Credits/About page patches
//
// Revision 1.3  2001/10/10 21:42:09  adridg
// Actually do part of a sync now
//
// Revision 1.2  2001/10/10 13:40:07  cschumac
// Compile fixes.
//
// Revision 1.1  2001/10/08 22:27:42  adridg
// New ui, moved to lib-based conduit
//
//

