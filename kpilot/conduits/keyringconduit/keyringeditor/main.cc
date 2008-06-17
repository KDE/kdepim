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
