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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qcombobox.h>

#include <kurlrequester.h>

#include "korganizerConduit.h"
#include "vcalconduitSettings.h"
#include "vcal-setupbase.h"

VCalWidgetSetupBase::VCalWidgetSetupBase(QWidget *w, const char *n) :
	ConduitConfigBase(w,n),
	fConfigWidget(new VCalWidget(w))
{
	FUNCTIONSETUP;
	fWidget=fConfigWidget;

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
	DEBUGCONDUIT << fname << ": Selected type="
		<< fConfigWidget->fSyncDestination->selected()
		<< " with id="
		<< fConfigWidget->fSyncDestination->id(fConfigWidget->fSyncDestination->selected())
		<< endl;
#endif
	config()->setCalendarType( fConfigWidget->fSyncDestination->id(
			fConfigWidget->fSyncDestination->selected()));
	config()->setCalendarFile( fConfigWidget->fCalendarFile->url());

	config()->setSyncArchived( fConfigWidget->fArchive->isChecked() );

	// Conflicts page
	config()->setConflictResolution(
		fConfigWidget->fConflictResolution->currentItem()+SyncAction::eCROffset);

	config()->writeConfig();
	unmodified();
}

/* virtual */ void VCalWidgetSetupBase::load()
{
	FUNCTIONSETUP;
	config()->readConfig();

	// General page
	fConfigWidget->fSyncDestination->setButton( config()->calendarType());
	fConfigWidget->fCalendarFile->setURL( config()->calendarFile() );

	fConfigWidget->fArchive->setChecked( config()->syncArchived() );

	// Conflicts page
	fConfigWidget->fConflictResolution->setCurrentItem(
		config()->conflictResolution() - SyncAction::eCROffset);

	config()->writeConfig();
	unmodified();
}

