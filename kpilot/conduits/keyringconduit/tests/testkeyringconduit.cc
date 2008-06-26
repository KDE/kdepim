#include "testkeyringconduit.h"

#include "options.h"
#include "idmapping.h"
#include "testkeyringproxy.h"

TestKeyringConduit::TestKeyringConduit( const QVariantList& args )
	: KeyringConduit( 0, args )
	, hhDataProxy( 0L )
	, backupDataProxy( 0L )
	, pcDataProxy( 0L )
{
}

bool TestKeyringConduit::initDataProxies()
{
	fMapping = new IDMapping( "Test User", fConduitName );

	hhDataProxy = new TestKeyringProxy( "hhproxy.pdb" );
	hhDataProxy->openDatabase( "test" );
	hhDataProxy->createDataStore();
	
	backupDataProxy = new TestKeyringProxy( "backupproxy.pdb" );
	backupDataProxy->openDatabase( "test" );
	backupDataProxy->createDataStore();
	
	pcDataProxy = new TestKeyringProxy( "pcproxy.pdb" );
	pcDataProxy->openDatabase( "test" );
	pcDataProxy->createDataStore();
	
	fHHDataProxy = hhDataProxy;
	fPCDataProxy = pcDataProxy;
	fBackupDataProxy = backupDataProxy;
	
	return true;
}

void TestKeyringConduit::hotSync()
{
	hotOrFullSync();
	
	// Make sure that the synced records get stored so that we can examine the
	// datastores.
	hhDataProxy->commit();
	backupDataProxy->commit();
	pcDataProxy->commit();
	
	// And make sure we don't sync the current records next time.
	hhDataProxy->syncFinished();
	backupDataProxy->syncFinished();
	pcDataProxy->syncFinished();
}

IDMapping* TestKeyringConduit::mapping() const
{
	return fMapping;
}

TestKeyringProxy* TestKeyringConduit::hhProxy() const
{
	return hhDataProxy;
}

TestKeyringProxy* TestKeyringConduit::backupProxy() const
{
	return backupDataProxy;
}

TestKeyringProxy* TestKeyringConduit::pcProxy() const
{
	return pcDataProxy;
}

