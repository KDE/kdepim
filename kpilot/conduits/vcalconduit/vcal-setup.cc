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


#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qcombobox.h>

#include <kconfig.h>
#include <kurlrequester.h>

#include "korganizerConduit.h"
#include "vcal-factory.h"
#include "vcal-setup.h"


VCalWidgetSetupBase::VCalWidgetSetupBase(QWidget *w, const char *n) :
	ConduitConfigBase(w,n),
	fConfigWidget(new VCalWidget(w))
{
	FUNCTIONSETUP;
	fWidget=fConfigWidget;

	fConfigWidget->fCalendarFile->setMode( KFile::File | KFile::LocalOnly );
	fConfigWidget->fCalendarFile->setFilter("*.vcs *.ics|ICalendars\n*.*|All Files (*.*)");

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

/* virtual */ void VCalWidgetSetupBase::commit(KConfig *fConfig)
{
	FUNCTIONSETUP;
	if (!fConfig) return;
	KConfigGroupSaver s(fConfig,configGroup());
	// General page
	fConfig->writeEntry(VCalConduitFactoryBase::calendarType,
		fConfigWidget->fSyncDestination->id(
			fConfigWidget->fSyncDestination->selected()));
	fConfig->writePathEntry(VCalConduitFactoryBase::calendarFile,
		fConfigWidget->fCalendarFile->url());

	fConfig->writeEntry(VCalConduitFactoryBase::archive,
		fConfigWidget->fArchive->isChecked());

	// Conflicts page
	fConfig->writeEntry(VCalConduitFactoryBase::conflictResolution,
		fConfigWidget->fConflictResolution->currentItem()+SyncAction::eCROffset);

	unmodified();
}

/* virtual */ void VCalWidgetSetupBase::load(KConfig *fConfig)
{
	FUNCTIONSETUP;

	if (!fConfig) return;
	KConfigGroupSaver s(fConfig, configGroup());

	// General page
	fConfigWidget->fSyncDestination->setButton(
		fConfig->readNumEntry(VCalConduitFactoryBase::calendarType, 0));
	fConfigWidget->fCalendarFile->setURL( fConfig->readPathEntry(
		VCalConduitFactoryBase::calendarFile));

	fConfigWidget->fArchive->setChecked(
		fConfig->readBoolEntry(VCalConduitFactoryBase::archive, true));

	// Conflicts page
	fConfigWidget->fConflictResolution->setCurrentItem(
		fConfig->readNumEntry(VCalConduitFactoryBase::conflictResolution,
		SyncAction::eUseGlobalSetting)-SyncAction::eCROffset);

	unmodified();
}

VCalWidgetSetup::VCalWidgetSetup(QWidget *w, const char *n) :
	VCalWidgetSetupBase(w,n)
{
	UIDialog::addAboutPage(fConfigWidget->tabWidget,VCalConduitFactory::about());
	fConfigWidget->fSyncDestination->setTitle(i18n("Calendar Destination"));
	fConduitName=i18n("Calendar");
	fGroupName=QString::fromLatin1(VCalConduitFactory::group);

}

/* static */ ConduitConfigBase *VCalWidgetSetup::create(QWidget *w,const char *n)
{
	return new VCalWidgetSetup(w,n);
}


