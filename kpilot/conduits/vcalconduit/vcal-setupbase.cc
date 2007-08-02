/* vcal-setup.cc                        KPilot
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

#include "vcal-setupbase.h"

#include "options.h"

#include <qcheckbox.h>
#include <q3buttongroup.h>
#include <qcombobox.h>

#include <kurlrequester.h>

#include "ui_setup_base.h"
#include "vcalconduitSettings.h"
//#include "ui_korganizerConduit.h"

VCalWidgetSetupBase::VCalWidgetSetupBase(QWidget *w) :
	ConduitConfigBase(w),
	fConfigWidget(new Ui::VCalWidget())
{
	FUNCTIONSETUP;
	fConfigWidget->setupUi( fWidget );

	fConfigWidget->fCalendarFile->setMode(KFile::File);
	fConfigWidget->fCalendarFile->setFilter(CSL1("*.vcs *.ics|ICalendars\n*.*|All Files (*.*)"));

#define CM(a,b) connect(fConfigWidget->a,b,this,SLOT(modified()));
	CM(fSyncDestination,SIGNAL(clicked(int)));
	CM(fCalendarFile,SIGNAL(textChanged(const QString &)));
	CM(fArchive,SIGNAL(toggled(bool)));
	CM(fConflictResolution,SIGNAL(activated(int)));
#undef CM
}

VCalWidgetSetupBase::~VCalWidgetSetupBase()
{
	FUNCTIONSETUP;
}

/* virtual */ void VCalWidgetSetupBase::commit()
{
	FUNCTIONSETUP;
	config()->readConfig();

	// General page
#ifdef DEBUG
	DEBUGKPILOT << fname <<": Selected type="
		<< fConfigWidget->fSyncDestination->selected()
		<< " with id="
		<< fConfigWidget->fSyncDestination->id(fConfigWidget->fSyncDestination->selected())
		<< endl;
#endif
	config()->setCalendarType( fConfigWidget->fSyncDestination->id(
			fConfigWidget->fSyncDestination->selected()));
	config()->setCalendarFile( fConfigWidget->fCalendarFile->url().url());

	config()->setSyncArchived( fConfigWidget->fArchive->isChecked() );

	// Conflicts page
	config()->setConflictResolution(
		fConfigWidget->fConflictResolution->currentIndex()+SyncAction::eCROffset);

	config()->writeConfig();
	unmodified();
}

/* virtual */ void VCalWidgetSetupBase::load()
{
	FUNCTIONSETUP;
	config()->readConfig();

	// General page
	fConfigWidget->fSyncDestination->setButton( config()->calendarType());
	fConfigWidget->fCalendarFile->setUrl( config()->calendarFile() );

	fConfigWidget->fArchive->setChecked( config()->syncArchived() );

	// Conflicts page
	fConfigWidget->fConflictResolution->setCurrentIndex(
		config()->conflictResolution() - SyncAction::eCROffset);

	config()->writeConfig();
	unmodified();
}

