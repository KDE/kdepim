// setupDialog.cc
//
// Copyright (C) 2000 Dan Pilone, Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
//
// This is setupDialog.cc for KDE 2 / KPilot 4


// This is the setup dialog for null-conduit.
// Because null-conduit does nothing, the
// setup is fairly simple.
//
//

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
// Revision 1.1  2000/11/20 00:22:28  adridg
// New KNotes conduit
//
