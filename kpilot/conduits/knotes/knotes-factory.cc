/* KPilot
**
** Copyright (C) 2001,2003 by Dan Pilone
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

#include <dcopclient.h>

#include <time.h> // Needed by pilot-link include

#include <pi-memo.h>

#include "knotes-action.h"
#include "knotes-setup.h"

#include "knotes-factory.moc"


extern "C"
{

void *init_conduit_knotes()
{
	return new KNotesConduitFactory;
}

}


/* static */ KAboutData *KNotesConduitFactory::fAbout = 0L;

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

	if (qstrcmp(c,"ConduitConfigBase")==0)
	{
		QWidget *w = dynamic_cast<QWidget *>(p);
		if (w)
		{
			return new KNotesConfigBase(w,0L);
		}
		else
		{
			return 0L;
		}
	}
	else
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
