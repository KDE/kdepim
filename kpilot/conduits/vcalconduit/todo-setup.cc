/* todo-setup.cc                        KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the todo-conduit plugin.
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

#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>

#include <kconfig.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <kfiledialog.h>

#include "korganizertodoConduit.h"
#include "todo-factory.h"
#include "todo-setup.moc"



ToDoWidgetSetup::ToDoWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigWidget = new ToDoWidget(widget());
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false,ToDoConduitFactory::about());

	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());

	// This is a little hack to force the config dialog to the
	// correct size, since the designer dialog is so small.
	//
	//
//	QSize s = fConfigWidget->size() + QSize(SPACING,SPACING);
//	fConfigWidget->resize(s);
//	fConfigWidget->setMinimumSize(s);

	QObject::connect(fConfigWidget->fCalBrowse,SIGNAL(clicked()),
		this,SLOT(slotBrowseCalendar()));
}

ToDoWidgetSetup::~ToDoWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void ToDoWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig, ToDoConduitFactory::group);

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

/* virtual */ void ToDoWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,ToDoConduitFactory::group);

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

void ToDoWidgetSetup::slotBrowseCalendar()
{
	FUNCTIONSETUP;

	QString fileName = KFileDialog::getOpenFileName(CSL1("::calendar"), CSL1("*.vcs *ics|ICalendars"),this);
	if(fileName.isNull()) return;
	fConfigWidget->fCalendarFile->setText(fileName);
}
