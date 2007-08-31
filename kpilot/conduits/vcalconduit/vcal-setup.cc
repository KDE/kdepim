/* KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the setup dialog for the vcal-conduit plugin.
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
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qbuttongroup.h>
#include <kaboutdata.h>

#include "korganizerConduit.h"
#include "vcal-conduit.h"
#include "vcal-setup.h"


VCalWidgetSetup::VCalWidgetSetup(QWidget *w, const char *n) :
	VCalWidgetSetupBase(w,n)
{
	KAboutData *fAbout = new KAboutData("vcalConduit",
		I18N_NOOP("VCal Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the VCal Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Adriaan de Groot\n(C) 2002-2003, Reinhold Kainhofer");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org",
		"http://www.kpilot.org/");
	fAbout->addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Maintainer"),
		"reinhold@kainhofer.com",
		"http://reinhold.kainhofer.com/Linux/");
	fAbout->addAuthor("Dan Pilone",
		I18N_NOOP("Original Author"));
	fAbout->addAuthor("Preston Brown",
		I18N_NOOP("Original Author"));
	fAbout->addAuthor("Herwin-Jan Steehouwer",
		I18N_NOOP("Original Author"));
	fAbout->addCredit("Cornelius Schumacher",
		I18N_NOOP("iCalendar port"));
	fAbout->addCredit("Philipp Hullmann",
		I18N_NOOP("Bugfixer"));

	ConduitConfigBase::addAboutPage(fConfigWidget->tabWidget, fAbout);
	fConfigWidget->fSyncDestination->setTitle(i18n("Calendar Destination"));
	fConduitName=i18n("Calendar");

}

/* static */ ConduitConfigBase *VCalWidgetSetup::create(QWidget *w,const char *n)
{
	return new VCalWidgetSetup(w,n);
}
VCalConduitSettings*VCalWidgetSetup::config() { return VCalConduit::theConfig(); }
