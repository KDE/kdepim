// null-conduit.cc
//
// Copyright (C) 2000 by Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//


// The NULL conduit is a conduit that does nothing.
// It's just a programming example, and maybe sometime
// we can attach it to databases as a means of *not*
// doing anything with those databases.
//
//



// KDE standard includes 
//
//
#include <qdir.h>
#include <kapp.h>
#include <kconfig.h>
#include <kmsgbox.h>
#include <ksock.h>

// Conduit standard includes
//
//
#include "conduitApp.h"
#include "kpilot.h"

// null-conduit specific includes
//
//
#include "null-conduit.h"
#include "setupDialog.h"


// This is a generic main() function, all
// conduits look basically the same,
// except for the name of the conduit.
//
//
int main(int argc, char* argv[])
{
	ConduitApp a(argc, argv, "null-conduit");

	if (a.getMode() != BaseConduit::Error)
	{
		NullConduit conduit(a.getMode());
		a.setConduit(&conduit);
		return a.exec();
	}
	return 1;
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

	KConfig* config = kapp->getConfig();
	config->setGroup(NullOptions::groupName());
	// pilotLink->addSyncLogEntry(config->readEntry("Text"));

	cerr << fname << ": Message from null-conduit:\n"
		<< fname << ": " << config->readEntry("Text");
}

// aboutAndSetup is pretty much the same
// on all conduits as well.
//
//
QWidget*
NullConduit::aboutAndSetup()
{
	FUNCTIONSETUP;

	return new NullOptions;
}

// $Log$
