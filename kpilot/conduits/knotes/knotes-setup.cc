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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
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


// $Log$
// Revision 1.2  2001/12/20 22:55:44  adridg
// Making conduits save their configuration and doing syncs
//
// Revision 1.1  2001/10/16 21:44:53  adridg
// Split up some files, added behavior
//
// Revision 1.4  2001/10/10 22:39:49  adridg
// Some UI/Credits/About page patches
//
// Revision 1.3  2001/10/10 21:42:09  adridg
// Actually do part of a sync now
//
// Revision 1.2  2001/10/10 13:40:07  cschumac
// Compile fixes.
//
// Revision 1.1  2001/10/08 22:27:42  adridg
// New ui, moved to lib-based conduit
//
//

