// setupDialog.cc
//
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
//
// $Revision$


// This is the setup dialog for null-conduit.
// Because null-conduit does nothing, the
// setup is fairly simple.
//
//


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
// null-conduit specific includes
//
//
#include "setupDialog.moc"

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

// groupName returns the group that
// our configuration options go in.
//
//
/* virtual */ const char *NullOptions::groupName() 
{
	return configGroup();
}

/* static */ const char *NullOptions::configGroup() 
{
	return "NULL Conduit";
}

NullOptions::NullOptions(QWidget *parent) :
	setupDialog(parent, "Null-conduit Options",0L)
{
	FUNCTIONSETUP;
	KConfig *config=kapp->getConfig();

	addPage(new NullPage(this,config));
	addPage(new setupInfoPage(this,
		"NULL Conduit",
		"Adriaan de Groot",
		i18n("A totally useless conduit used "
			"as a programming example.\n"
			"You can attach it to databases you don't want "
			"to synchronize.")));
	setupDialog::setupWidget();
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
	setupDialogPage(parent,config)
{
	FUNCTIONSETUP;

	generalLabel=new QLabel(klocale->translate(
		"The NULL conduit doesn't actually do anything."),
		this);
	generalLabel->adjustSize();
	generalLabel->move(10,14);

	textFieldLabel=new QLabel(klocale->translate("Log message:"),
		this);
	textFieldLabel->adjustSize();
	textFieldLabel->move(10,BELOW(generalLabel));

	textField=new QLineEdit(this);
	textField->setText(config->readEntry("Text","NULL conduit was here!"));
	textField->resize(200,textField->height());
	textField->move(RIGHT(textFieldLabel),BELOW(generalLabel));

}

/* virtual */ const char *NullPage::tabName()
{
	return "Null Conduit";
}

// $Log$
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
