/* setupDialog.cc			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the KNotes conduit, a conduit for KPilot that
** synchronises the Pilot's memo pad application with KNotes.
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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
#include "options.h"

#include <stream.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <kconfig.h>
#include <kdebug.h>
#include "kpilotlink.h"
#include "setupDialog.moc"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *id="$Id$";

KNotesGeneralPage::KNotesGeneralPage(setupDialog *p,KConfig& c) :
	setupDialogPage(i18n("General"),p)
{
	FUNCTIONSETUP;

	QGridLayout *grid = new QGridLayout(this,3,3,0,SPACING);

	fDeleteNoteForMemo = new QCheckBox(
		i18n("Delete KNote when Pilot memo is deleted"),
		this);
	fDeleteNoteForMemo -> setChecked(
		c.readBoolEntry("DeleteNoteForMemo",false));
	grid->addWidget(fDeleteNoteForMemo,1,1);

	grid->addRowSpacing(0,SPACING);
	grid->addColSpacing(2,SPACING);
	grid->setRowStretch(2,100);
}

int KNotesGeneralPage::commitChanges(KConfig& c)
{
	FUNCTIONSETUP;

	c.writeEntry("DeleteNoteForMemo",
		(bool)fDeleteNoteForMemo->isChecked());

	return 0;
}


/* static */ const QString KNotesOptions::KNotesGroup("conduitKNote");

KNotesOptions::KNotesOptions(QWidget *parent) :
	setupDialog(parent,KNotesGroup,0L)
{
	FUNCTIONSETUP;
	KConfig& c = KPilotLink::getConfig(KNotesGroup);

	addPage(new KNotesGeneralPage(this,c));
	addPage(new setupInfoPage(this));
	setupWidget();

	(void) id;
}

  
// $Log$
// Revision 1.2  2000/11/26 01:43:21  adridg
// Two-way syncs
//
// Revision 1.1  2000/11/20 00:22:28  adridg
// New KNotes conduit
//
