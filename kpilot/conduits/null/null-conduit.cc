// null-conduit.cc
//
// Copyright (C) 2000 by Dan Pilone, Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// This is the version of null-conduit.cc for KDE 2 / KPilot 4


// The NULL conduit is a conduit that does nothing.
// It's just a programming example, and maybe sometime
// we can attach it to databases as a means of *not*
// doing anything with those databases.
//
//



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
#ifdef KDE2
	ConduitApp a(argc,argv,"null-conduit",
		I18N_NOOP("NULL Conduit"),
		"4.0b");

	a.addAuthor("Adriaan de Groot",
		"NULL Conduit author",
		"adridg@sci.kun.nl");
#else
	ConduitApp a(argc, argv, "null-conduit",
		"\t\tNull-Conduit -- A conduit for KPilot\n"
		"Copyright (C) 2000 Adriaan de Groot");
#endif

	NullConduit conduit(a.getMode());
	a.setConduit(&conduit);
	return a.exec();
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

	KConfig* config = KPilotLink::getConfig();
	config->setGroup(NullOptions::NullGroup);

	QString m=config->readEntry("Text");
	addSyncLogMessage(m.latin1());

	kdDebug() << fname << ": Message from null-conduit:\n"
		<< fname << ": " << m
		<< endl;

	delete config;
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
	KConfig *config = KPilotLink::getConfig(NullOptions::NullGroup);

	QString m = config->readEntry("DB");
	if (m.isNull())
	{
		return "";
	}
	else
	{
		return m.ascii();
	}
}


// $Log$
// Revision 1.8  2000/09/27 18:41:21  adridg
// Added author info and new QT layout code.
//
// Revision 1.7  2000/09/05 07:13:57  adridg
// Updated to KCmdLineArgs
//
// Revision 1.6  2000/08/28 12:22:03  pilone
// 	KDE 2.0 Cleanup patches.  Start of adding conduits as kpilot
// services.
//
// Revision 1.5  2000/07/27 23:07:16  pilone
// 	Ported the conduits.  They build.  Don't know if they work, but they
// build.
//
// Revision 1.8  2000/07/20 21:29:42  adridg
// Minor KDE1 & KDE2 interoperability issues
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
// Revision 1.4  2000/05/21 00:42:53  adridg
// Changed to reflect new debug guidelines and usage()
//
// Revision 1.3  2000/01/23 23:13:59  adridg
// Unified dialog layout
//
// Revision 1.2  2000/01/22 23:01:27  adridg
// Minor ID stuff
//
// Revision 1.1  2000/01/21 16:31:38  adridg
// Added null conduit to 3.1b11 (KDE 1.1.2)
//
// Revision 1.2  2000/01/17 13:50:21  adridg
// Fixed resize bugs; log null-conduit message; lots of comments added as example
//
