/* vcal-setup.cc                        KPilot
**
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>

#include <kconfig.h>
#include <kfiledialog.h>

#include "korganizerConduit.h"
#include "vcal-factory.h"
#include "vcal-setup.moc"


VCalWidgetSetup::VCalWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigWidget = new VCalWidget(widget());
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false,VCalConduitFactory::about());

	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());

	// This is a little hack to force the config dialog to the
	// correct size, since the designer dialog is so small.
	//
	//
//	QSize s = fConfigWidget->size() + QSize(SPACING,SPACING);
//	fConfigWidget->resize(s);
//	fConfigWidget->setMinimumSize(s);

	QObject::connect((QObject*)fConfigWidget->fCalBrowse,SIGNAL(clicked()),
		this,SLOT(slotBrowseCalendar()));
}

VCalWidgetSetup::~VCalWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void VCalWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,VCalConduitFactory::group);

	fConfig->writeEntry(VCalConduitFactoryBase::calendarFile, fConfigWidget->fCalendarFile->text());
	fConfig->writeEntry(VCalConduitFactoryBase::archive, fConfigWidget->fArchive->isChecked());
	fConfig->writeEntry(VCalConduitFactoryBase::conflictResolution,
		fConfigWidget->conflictResolution->id(fConfigWidget->conflictResolution->selected()));

	int act=fConfigWidget->syncAction->id(fConfigWidget->syncAction->selected())+1;
	if (act>SYNC_MAX)
	{
		fConfig->writeEntry(VCalConduitFactoryBase::nextSyncAction, act-SYNC_MAX);
	}
	else
	{
		fConfig->writeEntry(VCalConduitFactoryBase::nextSyncAction, 0);
		fConfig->writeEntry(VCalConduitFactoryBase::syncAction, act);
	}
}

/* virtual */ void VCalWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,VCalConduitFactory::group);
	fConfigWidget->fCalendarFile->setText( fConfig->readEntry(VCalConduitFactoryBase::calendarFile,QString::null));
	fConfigWidget->fArchive->setChecked( fConfig->readBoolEntry(VCalConduitFactoryBase::archive, true));
	fConfigWidget->conflictResolution->setButton( fConfig->readNumEntry(VCalConduitFactoryBase::conflictResolution, RES_ASK));

	int nextAction=fConfig->readNumEntry(VCalConduitFactoryBase::nextSyncAction, 0);
	if (nextAction)
	{
		fConfigWidget->syncAction->setButton( SYNC_MAX+nextAction-1);
	}
	else
	{
		fConfigWidget->syncAction->setButton( fConfig->readNumEntry(VCalConduitFactoryBase::syncAction, SYNC_FAST)-1);
	}

}

void VCalWidgetSetup::slotBrowseCalendar()
{
	FUNCTIONSETUP;

	QString fileName = KFileDialog::getOpenFileName("::calendar", "*.vcs *ics|ICalendars",this);
	if(fileName.isNull()) return;
	fConfigWidget->fCalendarFile->setText(fileName);
}

// $Log$
// Revision 1.18.2.1  2002/05/01 21:11:49  kainhofe
// Reworked the settings dialog, added various different sync options
//
// Revision 1.18  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.17  2002/01/02 12:21:16  bero
// Fix build
//
// Revision 1.16  2001/12/28 12:56:46  adridg
// Added SyncAction, it may actually do something now.
//
// Revision 1.1  2001/12/13 21:40:40  adridg
// New files for move to .so
//

