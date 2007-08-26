/* importaddresses			KPilot
**
** Copyright (C) 2006 by Adriaan de Groot <groot@kde.org)
**
** Create an address database (for the handheld) from PC data.
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

#include <kabc/addressbook.h>
#include <kabc/resourcefile.h>

#include "pilot.h"
#include "pilotLocalDatabase.h"
#include "pilotAddress.h"
#include "../conduits/abbrowserconduit/kabcRecord.h"
#include "../conduits/abbrowserconduit/kabcRecord.cc"

static const KCmdLineOptions options[] =
{
	{"verbose", "Verbose output", 0},
	{"data-dir <path>","Set data directory", "."},
	{"address-file <path>","Set addressbook file", 0},
	KCmdLineLastOption
};



int main(int argc, char **argv)
{
	KAboutData aboutData("importaddresses","Import Address Book","0.1");
	KCmdLineArgs::init(argc,argv,&aboutData);
	KCmdLineArgs::addCmdLineOptions( options );

	//  KApplication app( false, false );
	KApplication app;

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

#ifdef DEBUG
	debug_level= (args->isSet("verbose")) ? 4 : 0;
#endif
	QString datadir = args->getOption("data-dir");
	QString addressfile = args->getOption("address-file");

	if (datadir.isEmpty())
	{
		kdWarning() << "! Must provide a data-directory." << endl;
	}
	if (addressfile.isEmpty())
	{
		kdWarning() << "! Must provide an address-file to read." << endl;
	}
	if (datadir.isEmpty() || addressfile.isEmpty())
	{
		return 1;
	}

	KABC::ResourceFile *file = new KABC::ResourceFile( addressfile );
	KABC::AddressBook book;
	book.addResource( file );
	if (!book.load())
	{
		kdWarning() << "! Failed to load the address-file <" << addressfile << ">" << endl;
		return 1;
	}

	Pilot::setupPilotCodec( CSL1("Latin1") );

	PilotLocalDatabase db( datadir, "AddressDB" );
	db.createDatabase( 0xdead, 0xbeef );
	PilotAddressInfo info(0L);
	info.resetToDefault();
	info.writeTo(&db);

	KABCSync::Settings settings;

	kdDebug() << "# Printing address book." << endl;
	unsigned int count = 1;
	KABC::AddressBook::ConstIterator it = book.begin();
	while (it != book.end())
	{
		const KABC::Addressee &a = *it;
		kdDebug() << "# Entry #" << count << endl;
		kdDebug() << "#  " << a.name() << endl;
		kdDebug() << "#  " << a.formattedName() << endl;
		PilotAddress *p = new PilotAddress();
		KABCSync::copy(*p,a,info,settings);
		PilotRecord *r = p->pack();
		if (r)
		{
			db.writeRecord(r);
			delete r;
		}
		delete p;
		++it;
		++count;
	}

	return 0;
}
