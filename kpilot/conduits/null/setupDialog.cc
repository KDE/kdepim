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
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include "kpilotlink.h"
#include "setupDialog.moc"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static char *id="$Id$";



/* static */ const QString NullOptions::NullGroup("Null-conduit Options");

NullOptions::NullOptions(QWidget *parent) :
	setupDialog(parent, NullGroup,0L)
{
	FUNCTIONSETUP;
	KConfig *config=KPilotLink::getConfig(NullGroup);

	addPage(new NullPage(this,config));
	/*
	   addPage(new setupInfoPage(this,
	     "NULL Conduit",
	     "Adriaan de Groot",
	     i18n("A totally useless conduit used "
	          "as a programming example.\n"
	          "You can attach it to databases you don't want "
	          "to synchronize.")));
	 */
	addPage(new setupInfoPage(this));
	setupDialog::setupWidget();

	delete config;
}

  
int NullPage::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;

	if (debug_level)
	{
		kdDebug() << fname << ": Wrote null-conduit message:\n" <<
			fname << ": " << textField->text() << endl;
	}
	config->writeEntry("Text", textField->text());

	return 0;
}


NullPage::NullPage(setupDialog *parent, KConfig *config) :
	setupDialogPage(i18n("Null Conduit"),parent,config)
{
	FUNCTIONSETUP;
	QGridLayout *grid=new QGridLayout(this,4,4,0,SPACING);
	grid->addRowSpacing(0,SPACING);
	grid->addColSpacing(0,SPACING);
	grid->addColSpacing(3,SPACING);

	generalLabel=new QLabel(i18n(
		"The NULL conduit doesn't actually do anything.\n"
		"Fill in databases you don't want to sync in\n"
		"the database field, separated by commas."),
		this);
	generalLabel->adjustSize();

	grid->addMultiCellWidget(generalLabel,1,1,1,2);

	textFieldLabel=new QLabel(i18n("Log message:"),this);
	textFieldLabel->adjustSize();

	textField=new QLineEdit(this);
	textField->setText(config->readEntry("Text","NULL conduit was here!"));
	textField->adjustSize();

	grid->addWidget(textFieldLabel,2,1);
	grid->addWidget(textField,2,2);

	dbLabel=new QLabel(i18n("Databases:"),this);
	dbLabel->adjustSize();
	dbField=new QLineEdit(this);
	dbField->setText(config->readEntry("DB"));

	grid->addWidget(dbLabel,3,1);
	grid->addWidget(dbField,3,2);

	grid->setRowStretch(4,100);
}


// $Log$
// Revision 1.8  2000/09/27 18:41:21  adridg
// Added author info and new QT layout code.
//
// Revision 1.7  2000/08/28 12:22:03  pilone
// 	KDE 2.0 Cleanup patches.  Start of adding conduits as kpilot
// services.
//
// Revision 1.6  2000/08/08 02:22:30  matz
// As rikkus did not disable compilation of kpilot anymore I can even make
// it compilable:
// - works now with blddir != srcdir (I might have broken bld==src ;) test it)
// - the setupInfobla interface has changed to now use the info from KInstance
//   I only disabled the old call, but did not include the KInstance call, so
//   conduits have no about data right now (how can one show that at all?)
//   the author of kpilot would need to add it
// - some C++ comments in C files
// - some runtime fixes
// - it even starts, but as I have no Pilot I can't really test it
// - I'm very tired now ;)
//
// Revision 1.5  2000/07/27 23:07:16  pilone
// 	Ported the conduits.  They build.  Don't know if they work, but they
// build.
//
// Revision 1.7  2000/07/19 20:12:06  adridg
// Added KDE2 code
//
// Revision 1.6  2000/07/13 18:08:42  adridg
// Restructuring and sanitation of config files
//
// Revision 1.5  2000/07/10 21:23:33  adridg
// Adjusted to changes in setupDialog class
//
// Revision 1.4  2000/05/21 00:40:36  adridg
// Changed to reflect new debug guidelines
//
// Revision 1.3  2000/01/23 23:13:59  adridg
// Unified dialog layout
//
// Revision 1.2  2000/01/22 23:01:27  adridg
// Minor ID stuff
//
// Revision 1.1  2000/01/21 16:31:39  adridg
// Added null conduit to 3.1b11 (KDE 1.1.2)
//
// Revision 1.2  2000/01/17 13:50:21  adridg
// Fixed resize bugs; log null-conduit message; lots of comments added as example
//
