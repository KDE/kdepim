/* expense.cc			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the Expense conduit, a conduit for KPilot that
** synchronises the Pilot's expense application with .. something?
** Actually it just writes a CSV file.
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
#include <qdir.h>
#include <qmap.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <kdebug.h>

#include <time.h>
#include <pi-expense.h>

#include "conduitApp.h"
#include "kpilotConfig.h"
#include "expense.h"
#include "setupDialog.h"



// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *expense_id =
	"$Id$";


// This is a generic main() function, all
// conduits look basically the same,
// except for the name of the conduit.
//
//
int main(int argc, char* argv[])
{
	ConduitApp a(argc,argv,"conduitExpense",
		I18N_NOOP("Expense Conduit"),
		KPILOT_VERSION);

	a.addAuthor("Adriaan de Groot",
		"Expense Conduit author",
		"adridg@cs.kun.nl");

	ExpenseConduit conduit(a.getMode());
	a.setConduit(&conduit);
	return a.exec();
}




ExpenseConduit::ExpenseConduit(eConduitMode mode) : 
	BaseConduit(mode)
{
	FUNCTIONSETUP;

}

ExpenseConduit::~ExpenseConduit()
{
	FUNCTIONSETUP;

}

void
ExpenseConduit::doSync()
{
	FUNCTIONSETUP;
}











// aboutAndSetup is pretty much the same
// on all conduits as well.
//
//
QWidget*
ExpenseConduit::aboutAndSetup()
{
	FUNCTIONSETUP;

	return new ExpenseOptions(0L);
}

const char *
ExpenseConduit::dbInfo()
{
	return "ExpenseDB";
}



/* virtual */ void
ExpenseConduit::doTest()
{
	FUNCTIONSETUP;

	(void) expense_id;
}

// $Log$
// Revision 1.1  2001/03/04 21:45:30  adridg
// New expense conduit, non-functional but it compiles
//
