/* null-conduit.cc			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the NULL conduit, a conduit for KPilot that
** does nothing except add a log message to the Pilot's HotSync log.
** It is also intended as a programming example.
**
** This file does the actual conduit work.
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

// Only include what we really need:
// First UNIX system stuff, then std C++, 
// then Qt, then KDE, then local includes.
//
//
#include <iostream.h>

#ifndef _KMESSAGEBOX_H
#include <kmessagebox.h>
#endif

#ifndef _KCONFIG_H
#include <kconfig.h>
#endif

#ifndef _KDEBUG_H
#include <kdebug.h>
#endif


#ifndef _KPILOT_CONDUITAPP_H
#include "conduitApp.h"
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif

#ifndef _NULL_NULL_CONDUIT_H
#include "null-conduit.h"
#endif

#ifndef _NULL_SETUPDIALOG_H
#include "setupDialog.h"
#endif



// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *null_conduit_id=
	"$Id$";


// This is a generic main() function, all
// conduits look basically the same,
// except for the name of the conduit.
//
//
int main(int argc, char* argv[])
{
	ConduitApp a(argc,argv,"null-conduit",
		I18N_NOOP("NULL Conduit"),
		KPILOT_VERSION);

	a.addAuthor("Adriaan de Groot",
		"NULL Conduit author",
		"adridg@sci.kun.nl");

	NullConduit conduit(a.getMode());
	a.setConduit(&conduit);
	return a.exec();

	/* NOTREACHED */
	/* Avoid const char *id not used warnings */
	(void) null_conduit_id;
}

// A conduit that does nothing has a very
// simple constructor and destructor.
//
//
NullConduit::NullConduit(eConduitMode mode)
	: BaseConduit(mode)
{
	FUNCTIONSETUP;

}

NullConduit::~NullConduit()
{
	FUNCTIONSETUP;

}

// doSync should add a line to the logfile
// with the indicated text, but the 
// addSyncLogEntry() doesn't work (not
// in any other conduit, either).
//
// Just print the message to error.
//
//
void
NullConduit::doSync()
{
	FUNCTIONSETUP;

	KConfig& config = KPilotConfig::getConfig();
	config.setGroup(NullOptions::NullGroup);

	QString m=config.readEntry("Text");
	addSyncLogMessage(m.latin1());

	kdDebug() << fname << ": Message from null-conduit:\n"
		<< fname << ": " << m
		<< endl;
}

// aboutAndSetup is pretty much the same
// on all conduits as well.
//
//
QWidget*
NullConduit::aboutAndSetup()
{
	FUNCTIONSETUP;

	return new NullOptions(0L);
}

const char *
NullConduit::dbInfo()
{
	KConfig& config = KPilotConfig::getConfig(NullOptions::NullGroup);

	QString m = config.readEntry("DB");
	if (m.isNull())
	{
		return "<none>";
	}
	else
	{
		return m.ascii();
	}
}


// $Log$
// Revision 1.19  2001/04/01 17:31:11  adridg
// --enable-final and #include fixes
//
// Revision 1.18  2001/03/27 11:10:38  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
// Revision 1.17  2001/03/09 09:46:14  adridg
// Large-scale #include cleanup
//
// Revision 1.16  2001/03/05 23:57:53  adridg
// Added KPILOT_VERSION
//
// Revision 1.15  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.14  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.13  2001/02/07 15:46:31  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
// Revision 1.12  2000/12/21 00:42:50  adridg
// Mostly debugging changes -- added EFUNCTIONSETUP and more #ifdefs. KPilot should now compile -DNDEBUG or with DEBUG undefined
//
// Revision 1.5  2000/07/27 23:07:16  pilone
// 	Ported the conduits.  They build.  Don't know if they work, but they
// build.
