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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <kapplication.h>
#include <kconfig.h>

#include "time-setup_dialog.h"

#include "time-factory.h"
#include "time-setup.moc"


TimeWidgetSetup::TimeWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigWidget = new TimeWidget(widget());
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false,TimeConduitFactory::about());

	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
}

TimeWidgetSetup::~TimeWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void TimeWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,TimeConduitFactory::group());

	fConfig->writeEntry(TimeConduitFactory::direction(),
		fConfigWidget->directionGroup->id(fConfigWidget->directionGroup->selected()));
}

/* virtual */ void TimeWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,TimeConduitFactory::group());

	fConfigWidget->directionGroup->setButton(fConfig->readNumEntry(TimeConduitFactory::direction(), DIR_PCToPalm) );
}

