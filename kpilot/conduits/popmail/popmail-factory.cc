/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the popmail-conduit plugin.
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

#include <qtabwidget.h>
#include <qlineedit.h>

#include <kconfig.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include "setupDialog.h"
#include "popmail-conduit.h"
#include "popmail-factory.moc"


extern "C"
{

void *init_conduit_popmail()
{
	return new PopMailConduitFactory;
}

}


KAboutData *PopMailConduitFactory::fAbout = 0L;
PopMailConduitFactory::PopMailConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("popmailconduit");
	fAbout = new KAboutData("popmailConduit",
		I18N_NOOP("Mail Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Mail Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Dan Pilone, Michael Kropfberger, Adriaan de Groot");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addAuthor("Dan Pilone",
		I18N_NOOP("Original Author"));
	fAbout->addCredit("Michael Kropfberger",
		I18N_NOOP("POP3 code"));
	fAbout->addCredit("Marko Gr&ouml;nroos",
		I18N_NOOP("SMTP support and redesign"),
		"magi@iki.fi",
		"http://www/iki.fi/magi/");
}

PopMailConduitFactory::~PopMailConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *PopMailConduitFactory::createObject( QObject *p,
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
			return new PopMailWidgetConfig(w,n);
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
			return new PopMailConduit(d,n,a);
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

