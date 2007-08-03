/* testaddresses			KPilot
**
** Copyright (C) 2006 by Adriaan de Groot <groot@kde.org)
**
** Test the functions related to address database handling.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "pilot.h"
#include "pilotAddress.h"
#include "pilotLocalDatabase.h"



int main(int argc, char **argv)
{
	FUNCTIONSETUP;

	KAboutData aboutData("testaddress", 0,ki18n("Test Addresses"),"0.1");
	KCmdLineArgs::init(argc,argv,&aboutData);

	KCmdLineOptions options;
	options.add("verbose", ki18n("Verbose output"));
	options.add("data-dir <path>", ki18n("Set data directory"), ".");
	KCmdLineArgs::addCmdLineOptions( options );

	KApplication app( false );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

#ifdef DEBUG
	debug_level= (args->isSet("verbose")) ? 4 : 0;
#endif
	QString datadir = args->getOption("data-dir");

	DEBUGKPILOT <<"### testaddresses\n#\n#";

	Pilot::setupPilotCodec( CSL1("Latin1") );

	PilotLocalDatabase db( datadir, "AddressDB" );
	PilotAddressInfo appinfo( &db );

	appinfo.dump();

	for (unsigned int i=0; i<db.recordCount(); ++i)
	{
		PilotRecord *r = db.readRecordByIndex( i );

		if (r)
		{
			DEBUGKPILOT <<"# Record @" << (void *)r <<" ID=" << r->id();
			PilotAddress a( r );
			DEBUGKPILOT <<"# Text Representation:" << endl << a.getTextRepresentation(&appinfo,Qt::PlainText);
			DEBUGKPILOT <<"# Category#" << a.category();
			DEBUGKPILOT <<"# Category Label" << appinfo.categoryName(a.category());
			DEBUGKPILOT <<"# ID" << a.id();

			// With the given address database, where all the
			// categories are already filled, this should fail
			// (and give a useful error message).
			//
			a.setCategory( appinfo.findCategory(CSL1("Fake Cat")) );
			DEBUGKPILOT <<"# Category#" << a.category();
			DEBUGKPILOT <<"# Category Label" << appinfo.categoryName(a.category());
			// This category exists, so it should succeed
			//
			a.setCategory( appinfo.findCategory(CSL1("Business")) );
			DEBUGKPILOT <<"# Category#" << a.category();
			DEBUGKPILOT <<"# Category Label" << appinfo.categoryName(a.category());
		}
	}

	return 0;
}

