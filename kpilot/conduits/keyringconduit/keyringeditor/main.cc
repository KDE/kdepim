/* main.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
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
#include <iostream>

#include <QCoreApplication>
#include <QDir>
#include <QDebug>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

#include "options.h"
#include "pilot.h"
#include "pilotRecord.h"
#include "pilotDatabase.h"
#include "pilotLocalDatabase.h"
#include "keyringhhdataproxy.h"
#include "record.h"

#include "keyringviewer.h"

using namespace std;

int main(int argc, char **argv)
{
	FUNCTIONSETUP;
	
	debug_level = 1;

	KAboutData aboutData("testdatebook", 0,ki18n("Test Date Book"),"0.1");
	KCmdLineArgs::init(argc,argv,&aboutData);


	KApplication app( true );
	
	Pilot::setupPilotCodec( CSL1( "ISO8859-15" ) );
	
	// If the database is not open the file did not exist. Let's create a new one.
	/*
	PilotLocalDatabase database( dbPath + '/' + dbName );
	KeyringHHDataProxy proxy( &database );
	proxy.openDatabase( pass );
	proxy.createDataStore();
	proxy.openDatabase( pass );
	*/
	KeyringViewer viewer( 0 );
	viewer.show();
	
	return app.exec();
}
