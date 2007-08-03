#include <iostream>

#include <QCoreApplication>
#include <QDir>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

#include "options.h"
#include "pilotDatabase.h"
#include "pilotLocalDatabase.h"
#include "keyringhhdataproxy.h"
#include "record.h"

using namespace std;

int main(int argc, char **argv)
{
	FUNCTIONSETUP;

	KAboutData aboutData("testdatebook", 0,ki18n("Test Date Book"),"0.1");
	KCmdLineArgs::init(argc,argv,&aboutData);

	KCmdLineOptions options;
	options.add("+path", ki18n("Path of the db file"), "path");
	options.add("+name", ki18n("Database name"), "name");
	options.add("+pass", ki18n("Password for the keyring database"), "pass");
	options.add( "", ki18n("Example: ./testje ~/data KeyRing secret_pass") );
	KCmdLineArgs::addCmdLineOptions( options );

	KApplication app( false );

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
	
	cout << "- Opening database: " << dbPath + dbName + ".pdb" << endl;
	
	// Create the database.
	PilotLocalDatabase database( dbPath + "/" + dbName );
	
	KeyringHHDataProxy proxy( &database );
	
	DEBUGKPILOT <<"Recordcount:" << proxy.recordCount();
	
	proxy.setIterateMode( DataProxy::All );
	proxy.resetIterator();
	
	while( proxy.hasNext() )
	{
		Record *rec = proxy.next();
		DEBUGKPILOT <<"Record:" << rec->toString();
	}
	
	return 0;
}
