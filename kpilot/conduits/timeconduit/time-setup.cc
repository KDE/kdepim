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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <kapplication.h>

#include "time-setup_dialog.h"

#include "time-factory.h"
#include "time-setup.moc"
#include "timeConduitSettings.h"

#include "uiDialog.h"

TimeWidgetConfig::TimeWidgetConfig(QWidget *w, const char *n) :
	ConduitConfigBase(w,n),
	fConfigWidget(new TimeWidget(w))
{
	FUNCTIONSETUP;
	UIDialog::addAboutPage(fConfigWidget->tabWidget,TimeConduitFactory::about());
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

