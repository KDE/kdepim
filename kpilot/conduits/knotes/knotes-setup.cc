/* knotes-setup.cc                      KPilot
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
#include "knotesconduitSettings.h"


KNotesConfigBase::KNotesConfigBase(QWidget *w, const char *n) :
	ConduitConfigBase(w,n),
	fConfigWidget(0L)
{
	fConfigWidget = new KNotesWidget(w);
	UIDialog::addAboutPage(fConfigWidget->tabWidget,KNotesConduitFactory::about());
	fWidget = fConfigWidget;
	QObject::connect(fConfigWidget->fDeleteNoteForMemo,SIGNAL(clicked()),
		this,SLOT(modified()));
	fConduitName=i18n("KNotes");
}

void KNotesConfigBase::commit()
{
	KNotesConduitSettings::setDeleteNoteForMemo( fConfigWidget->fDeleteNoteForMemo->isChecked() );
	KNotesConduitSettings::self()->writeConfig();
}

void KNotesConfigBase::load()
{
	KNotesConduitSettings::self()->readConfig();
	fConfigWidget->fDeleteNoteForMemo->setChecked(KNotesConduitSettings::deleteNoteForMemo() );
}

/* static */ ConduitConfigBase *KNotesConfigBase::create(QWidget *w, const char *n)
{
	return new KNotesConfigBase(w,n);
}

