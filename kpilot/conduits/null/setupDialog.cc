// setupDialog.cc
//
// Copyright (C) 2000 Adriaan de Groot
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

#ifdef KDE2
#include <stream.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <kconfig.h>
#include <klocale.h>
#include "kpilotlink.h"
#include "setupDialog.moc"
#else
// KDE standard includes
//
//
#include <kconfig.h>
#include <kapp.h>
#include <stream.h>



// KPilot standard includes
//
//
#include "kpilot.h"
#include "options.h"



// null-conduit specific includes
//
//
#include "setupDialog.moc"
#include "null-conduit.h"
#endif

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static char *id="$Id$";


// I couldn't find a max() function so this is
// it, for the null conduit. It's used in the widget
// layout section.
//
//
#define max(a,b) ((a)>(b) ? (a) : (b))

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
		cerr << fname << ": Wrote null-conduit message:\n" <<
			fname << ": " << textField->text() << endl;
	}
	config->writeEntry("Text", textField->text());

	return 0;
}


NullPage::NullPage(setupDialog *parent, KConfig *config) :
	setupDialogPage(i18n("Null Conduit"),parent,config)
{
	FUNCTIONSETUP;

	generalLabel=new QLabel(i18n(
		"The NULL conduit doesn't actually do anything."),
		this);
	generalLabel->adjustSize();
	generalLabel->move(10,14);

	textFieldLabel=new QLabel(i18n("Log message:"),
		this);
	textFieldLabel->adjustSize();
	textFieldLabel->move(10,BELOW(generalLabel));

	textField=new QLineEdit(this);
	textField->setText(config->readEntry("Text","NULL conduit was here!"));
	textField->resize(200,textField->height());
	textField->move(RIGHT(textFieldLabel),BELOW(generalLabel));

}


// $Log$
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
