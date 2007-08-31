/* Time-setup.cc                      KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the setup dialog for the Time-conduit plugin.
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

#include <qtabwidget.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

#include <kapplication.h>
#include <kaboutdata.h>

#include "time-setup_dialog.h"

#include "time-setup.moc"
#include "timeConduitSettings.h"



static KAboutData *createAbout()
{
	KAboutData *fAbout = new KAboutData("Timeconduit",
		I18N_NOOP("Time Synchronization Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Synchronizes the Time on the Handheld and the PC"),
		KAboutData::License_GPL,
		"(C) 2002, Reinhold Kainhofer");
	fAbout->addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Primary Author"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com/");
	return fAbout;
}



TimeWidgetConfig::TimeWidgetConfig(QWidget *w, const char *n) :
	ConduitConfigBase(w,n),
	fConfigWidget(new TimeWidget(w))
{
	FUNCTIONSETUP;
	fAbout = createAbout();
	ConduitConfigBase::addAboutPage(fConfigWidget->tabWidget,fAbout);
	fWidget=fConfigWidget;
	fConduitName=i18n("Time");
}

void TimeWidgetConfig::commit()
{
	FUNCTIONSETUP;
	TimeConduitSettings::setDirection(
		fConfigWidget->directionGroup->id(fConfigWidget->directionGroup->selected()) );
	TimeConduitSettings::self()->writeConfig();
}

void TimeWidgetConfig::load()
{
	FUNCTIONSETUP;
	TimeConduitSettings::self()->readConfig();

	fConfigWidget->directionGroup->setButton( TimeConduitSettings::direction() );
}

