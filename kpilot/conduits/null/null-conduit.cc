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


#include "options.h"

// Only include what we really need:
// First UNIX system stuff, then std C++, 
// then Qt, then KDE, then local includes.
//
//
#include <stream.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kdebug.h>

#include "conduitApp.h"
#include "kpilotlink.h"
#include "null-conduit.h"
#include "setupDialog.h"


// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *id=
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
		"4.0b");

	a.addAuthor("Adriaan de Groot",
		"NULL Conduit author",
		"adridg@sci.kun.nl");

	NullConduit conduit(a.getMode());
	a.setConduit(&conduit);
	return a.exec();

	/* NOTREACHED */
	/* Avoid const char *id not used warnings */
	(void) id;
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

	KConfig& config = KPilotLink::getConfig();
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
	KConfig& config = KPilotLink::getConfig(NullOptions::NullGroup);

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
// Revision 1.12  2000/12/21 00:42:50  adridg
// Mostly debugging changes -- added EFUNCTIONSETUP and more #ifdefs. KPilot should now compile -DNDEBUG or with DEBUG undefined
//
// Revision 1.5  2000/07/27 23:07:16  pilone
// 	Ported the conduits.  They build.  Don't know if they work, but they
// build.
