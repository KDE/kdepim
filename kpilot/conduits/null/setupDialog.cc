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
#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <iostream.h>



#ifndef QLABEL_H
#include <qlabel.h>
#endif
#ifndef QLINEEDIT_H
#include <qlineedit.h>
#endif
#ifndef QLAYOUT_H
#include <qlayout.h>
#endif
#ifndef QTOOLTIP_H
#include <qtooltip.h>
#endif


#ifndef _KCONFIG_H
#include <kconfig.h>
#endif
#ifndef _KLOCALE_H
#include <klocale.h>
#endif
#ifndef _KDEBUG_H
#include <kdebug.h>
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif

#include "setupDialog.moc"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *setupdialog_id="$Id$";



/* static */ const QString NullOptions::NullGroup("Null-conduit Options");

NullOptions::NullOptions(QWidget *parent) :
	setupDialog(parent, NullGroup,0L)
{
	FUNCTIONSETUP;
	KConfig& config=KPilotConfig::getConfig(NullGroup);

	addPage(new NullPage(this,config));
	addPage(new setupInfoPage(this));
	setupDialog::setupWidget();
}

  
int NullPage::commitChanges(KConfig& config)
{
	FUNCTIONSETUP;

	DEBUGCONDUIT << fname << ": Wrote null-conduit message:" << endl;
	DEBUGCONDUIT << fname << ": " << textField->text() << endl;

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

	textFieldLabel=new QLabel(i18n("&Log message:"),this);
	textField=new QLineEdit(this);
	textField->setText(config.readEntry("Text",
		i18n("NULL conduit was here!")));
	QToolTip::add(textField,
		i18n("Enter a message here. This message will be entered into\n"
		     "the Pilot's HotSync log when the NULL conduit runs."));
	textFieldLabel->setBuddy(textField);

	grid->addWidget(textFieldLabel,2,1);
	grid->addWidget(textField,2,2);

	dbLabel=new QLabel(i18n("&Databases:"),this);
	dbLabel->adjustSize();
	dbField=new QLineEdit(this);
	dbField->setText(config.readEntry("DB"));
	QToolTip::add(dbField,
		i18n("Enter a list of database names here. The NULL conduit\n"
		     "will be run for each of these databases, effectively\n"
		     "preventing them from being HotSynced."));
	dbLabel->setBuddy(dbField);

	grid->addWidget(dbLabel,3,1);
	grid->addWidget(dbField,3,2);

	grid->setRowStretch(4,100);
}


// $Log$
// Revision 1.17  2001/04/16 13:36:03  adridg
// Removed --enable-final borkage
//
// Revision 1.16  2001/04/01 17:31:11  adridg
// --enable-final and #include fixes
//
// Revision 1.15  2001/03/27 11:10:38  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
// Revision 1.14  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.13  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.12  2001/02/07 15:46:31  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
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
