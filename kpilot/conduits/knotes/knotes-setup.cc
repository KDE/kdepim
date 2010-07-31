/* KPilot
**
** Copyright (C) 2001,2003 by Dan Pilone
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <tqtabwidget.h>
#include <tqcheckbox.h>
#include <tqmap.h>
#include <tqtimer.h>

#include <kapplication.h>
#include <kconfig.h>

#include "setup_base.h"

#include "knotes-factory.h"
#include "knotes-setup.h"
#include "knotesconduitSettings.h"


KNotesConfigBase::KNotesConfigBase(TQWidget *w, const char *n) :
	ConduitConfigBase(w,n),
	fConfigWidget(0L)
{
	fConfigWidget = new KNotesWidget(w);
	ConduitConfigBase::addAboutPage(fConfigWidget->tabWidget,KNotesConduitFactory::about());
	fWidget = fConfigWidget;
	TQObject::connect(fConfigWidget->fDeleteNoteForMemo,TQT_SIGNAL(clicked()),
		this,TQT_SLOT(modified()));
	TQObject::connect(fConfigWidget->fSuppressConfirm,TQT_SIGNAL(clicked()),
		this,TQT_SLOT(modified()));
	TQObject::connect(fConfigWidget->fDeleteNoteForMemo,TQT_SIGNAL(toggled(bool)),
		fConfigWidget->fSuppressConfirm,TQT_SLOT(setEnabled(bool)));
	fConduitName=i18n("KNotes");
}

void KNotesConfigBase::commit()
{
	KNotesConduitSettings::setDeleteNoteForMemo( fConfigWidget->fDeleteNoteForMemo->isChecked() );
	KNotesConduitSettings::setSuppressKNotesConfirm(fConfigWidget->fSuppressConfirm->isChecked());
	KNotesConduitSettings::self()->writeConfig();
	unmodified();
}

void KNotesConfigBase::load()
{
	KNotesConduitSettings::self()->readConfig();
	fConfigWidget->fDeleteNoteForMemo->setChecked(KNotesConduitSettings::deleteNoteForMemo() );
	fConfigWidget->fSuppressConfirm->setChecked(KNotesConduitSettings::suppressKNotesConfirm() );
	fConfigWidget->fSuppressConfirm->setEnabled(KNotesConduitSettings::deleteNoteForMemo());
	unmodified();
}

/* static */ ConduitConfigBase *KNotesConfigBase::create(TQWidget *w, const char *n)
{
	return new KNotesConfigBase(w,n);
}

