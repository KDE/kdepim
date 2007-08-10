#include <iostream>

#include <QCoreApplication>
#include <QDir>

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

	KCmdLineOptions options;
	options.add("+path", ki18n("Path of the db file"), "path");
	options.add("+name", ki18n("Database name"), "name");
	options.add("+pass", ki18n("Password for the keyring database"), "pass");
	options.add( "", ki18n("Example: ./testje ~/data KeyRing secret_pass") );
	KCmdLineArgs::addCmdLineOptions( options );

	KApplication app( true );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	
	QString dbPath = args->arg( 0 );
	QString dbName = args->arg( 1 );
	QString pass = args->arg( 2 );
	
	args->clear(); // Free up some memory.
	
	QDir dir( dbPath );
	if( !dir.exists() )
	{
		cout << "Database dir: " << dbPath << " does not exist." << endl;
		return 1;
	}
	
	QFile file( dbPath + "/" + dbName + ".pdb" );
	if( !file.exists() )
	{
		cout << "Database file: " << dbName << ".pdb does not exist." << endl;
		return 1;
	}
	
	if( dbPath.at( dbPath.size() - 1 ) != '/' )
	{
		dbPath.append( '/' );
	}

	DEBUGKPILOT << "Opening database: " << dbPath + dbName + ".pdb";
	
	Pilot::setupPilotCodec( CSL1( "ISO8859-15" ) );
	
	// If the database is not open the file did not exist. Let's create a new one.
	PilotLocalDatabase database( dbPath + "/" + dbName );
	if( database.isOpen() )
	{
		KeyringHHDataProxy proxy( &database );
		proxy.openDatabase( pass );
		
		KeyringViewer *viewer = new KeyringViewer( 0, &proxy );
		viewer->show();
	}
	else
	{
		qDebug() << "Database not open, trying to create one.";
		KeyringHHDataProxy proxy( &database );
		proxy.openDatabase( pass );
		proxy.createDataStore();
		
		KeyringViewer *viewer = new KeyringViewer( 0, &proxy );
		viewer->show();
	}
	
	//return 0;
	return app.exec();
}
