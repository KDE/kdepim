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
#include "knotes-setup.h"


KNotesConfigBase::KNotesConfigBase(QWidget *w, const char *n) :
	ConduitConfigBase(w,n),
	fConfigWidget(0L)
{
	fConfigWidget = new KNotesWidget(w);
	UIDialog::addAboutPage(fConfigWidget->tabWidget,KNotesConduitFactory::about());
	fWidget = fConfigWidget;
}

void KNotesConfigBase::commit(KConfig *fConfig)
{
	KConfigGroupSaver s(fConfig,KNotesConduitFactory::group);

	fConfig->writeEntry(KNotesConduitFactory::matchDeletes,
		fConfigWidget->fDeleteNoteForMemo->isChecked());
}

void KNotesConfigBase::load(KConfig *fConfig)
{
	KConfigGroupSaver s(fConfig,KNotesConduitFactory::group);

	fConfigWidget->fDeleteNoteForMemo->setChecked(
		fConfig->readBoolEntry(KNotesConduitFactory::matchDeletes,false));
}

KNotesWidgetSetup::KNotesWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;
	fConfigBase = new KNotesConfigBase(widget(),"KNotesConfig");
}

KNotesWidgetSetup::~KNotesWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void KNotesWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;
	fConfigBase->commit(fConfig);
}

/* virtual */ void KNotesWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;
	fConfigBase->load(fConfig);
}

