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

#include <q3buttongroup.h>
#include <kaboutdata.h>

#include "ui_setup_base.h"
#include "vcal-conduit.h"
#include "vcal-setup.h"


VCalWidgetSetup::VCalWidgetSetup(QWidget *w) :
	VCalWidgetSetupBase(w)
{
	KAboutData *fAbout = new KAboutData("vcalConduit", 0,
		ki18n("VCal Conduit for KPilot"),
		KPILOT_VERSION,
		ki18n("Configures the VCal Conduit for KPilot"),
		KAboutData::License_GPL,
		ki18n("(C) 2001, Adriaan de Groot\n(C) 2002-2003, Reinhold Kainhofer"));
	fAbout->addAuthor(ki18n("Adriaan de Groot"),
		ki18n("Maintainer"),
		"groot@kde.org",
		"http://www.kpilot.org/");
	fAbout->addAuthor(ki18n("Reinhold Kainhofer"),
		ki18n("Maintainer"),
		"reinhold@kainhofer.com",
		"http://reinhold.kainhofer.com/Linux/");
	fAbout->addAuthor(ki18n("Dan Pilone"),
		ki18n("Original Author"));
	fAbout->addAuthor(ki18n("Preston Brown"),
		ki18n("Original Author"));
	fAbout->addAuthor(ki18n("Herwin-Jan Steehouwer"),
		ki18n("Original Author"));
	fAbout->addCredit(ki18n("Cornelius Schumacher"),
		ki18n("iCalendar port"));
	fAbout->addCredit(ki18n("Philipp Hullmann"),
		ki18n("Bugfixer"));

	ConduitConfigBase::addAboutPage(fConfigWidget->tabWidget, fAbout);
	fConfigWidget->fSyncDestination->setTitle(i18n("Calendar Destination"));
	fConduitName=i18n("Calendar");

}

/* static */ ConduitConfigBase *VCalWidgetSetup::create(QWidget *w,const char *n)
{
	ConduitConfigBase *t = new VCalWidgetSetup(w);
	t->setObjectName(n);
	return t;
}
VCalConduitSettings*VCalWidgetSetup::config() { return VCalConduit::theConfig(); }
