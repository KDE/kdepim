/* knotes-setup.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the setup dialog for the knotes-conduit plugin.
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
#include <qmap.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kconfig.h>

#include "setup_base.h"

#include "knotes-factory.h"
#include "knotes-setup.moc"


KNotesWidgetSetup::KNotesWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigWidget = new KNotesWidget(widget());
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false,KNotesConduitFactory::about());

	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
}

KNotesWidgetSetup::~KNotesWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void KNotesWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,KNotesConduitFactory::group);

	fConfig->writeEntry(KNotesConduitFactory::matchDeletes,
		fConfigWidget->fDeleteNoteForMemo->isChecked());
}

/* virtual */ void KNotesWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,KNotesConduitFactory::group);

	fConfigWidget->fDeleteNoteForMemo->setChecked(
		fConfig->readBoolEntry(KNotesConduitFactory::matchDeletes,false));
}

