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

#include <qtabwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>

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
	QSize s = fConfigWidget->size() + QSize(SPACING,SPACING);
	fConfigWidget->resize(s);
	fConfigWidget->setMinimumSize(s);

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

	KConfigGroupSaver s(fConfig,ToDoConduitFactory::group);

	fConfig->writeEntry(ToDoConduitFactory::calendarFile,
		fConfigWidget->fCalendarFile->text());
	fConfig->writeEntry(ToDoConduitFactory::firstTime,
		fConfigWidget->fPromptFirstTime->isChecked());
	fConfig->writeEntry(ToDoConduitFactory::deleteOnPilot,
		fConfigWidget->fDeleteOnPilot->isChecked());

}

/* virtual */ void ToDoWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,ToDoConduitFactory::group);

	fConfigWidget->fCalendarFile->setText(
		fConfig->readEntry(ToDoConduitFactory::calendarFile,QString::null));

	fConfigWidget->fPromptFirstTime->setChecked(
		fConfig->readBoolEntry(ToDoConduitFactory::firstTime,false));
	fConfigWidget->fDeleteOnPilot->setChecked(
		fConfig->readBoolEntry(ToDoConduitFactory::deleteOnPilot,false));
}

void ToDoWidgetSetup::slotBrowseCalendar()
{
	FUNCTIONSETUP;

	QString fileName = KFileDialog::getOpenFileName("::calendar", "*.vcs *ics|ICalendars",this);
	if(fileName.isNull()) return;
	fConfigWidget->fCalendarFile->setText(fileName);
}

// $Log$
// Revision 1.1  2001/12/13 21:40:40  adridg
// New files for move to .so
//

