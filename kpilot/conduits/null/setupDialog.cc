/* setupDialog.cc			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the NULL conduit, a conduit for KPilot that
** does nothing except add a log message to the Pilot's HotSync log.
** It is also intended as a programming example.
**
** This file defines the setup dialog for the conduit.
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
static const char *id="$Id$";



/* static */ const QString NullOptions::NullGroup("Null-conduit Options");

NullOptions::NullOptions(QWidget *parent) :
	setupDialog(parent, NullGroup,0L)
{
	FUNCTIONSETUP;
	KConfig& config=KPilotLink::getConfig(NullGroup);

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
}

  
int NullPage::commitChanges(KConfig& config)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	if (debug_level)
	{
		kdDebug() << fname << ": Wrote null-conduit message:\n" <<
			fname << ": " << textField->text() << endl;
	}
#endif
	config.writeEntry("Text", textField->text());

	return 0;
}


NullPage::NullPage(setupDialog *parent, KConfig& config) :
	setupDialogPage(i18n("Null Conduit"),parent)
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
	textField->setText(config.readEntry("Text","NULL conduit was here!"));
	textField->adjustSize();

	grid->addWidget(textFieldLabel,2,1);
	grid->addWidget(textField,2,2);

	dbLabel=new QLabel(i18n("Databases:"),this);
	dbLabel->adjustSize();
	dbField=new QLineEdit(this);
	dbField->setText(config.readEntry("DB"));

	grid->addWidget(dbLabel,3,1);
	grid->addWidget(dbField,3,2);

	grid->setRowStretch(4,100);
}


// $Log$
// Revision 1.11  2000/12/31 16:43:59  adridg
// Patched up the debugging stuff again
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
// Revision 1.1  2000/01/21 16:31:39  adridg
// Added null conduit to 3.1b11 (KDE 1.1.2)
