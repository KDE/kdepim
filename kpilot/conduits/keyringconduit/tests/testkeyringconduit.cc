#include "testkeyringconduit.h"

#include "options.h"
#include "idmapping.h"
#include "keyringhhdataproxy.h"

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

	hhDataProxy = new KeyringHHDataProxy( "hhproxy.pdb" );
	hhDataProxy->openDatabase( "test" );
	hhDataProxy->createDataStore();
	
	backupDataProxy = new KeyringHHDataProxy( "backupproxy.pdb" );
	backupDataProxy->openDatabase( "test" );
	backupDataProxy->createDataStore();
	
	pcDataProxy = new KeyringHHDataProxy( "pcproxy.pdb" );
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
}

IDMapping* TestKeyringConduit::mapping() const
{
	return fMapping;
}

KeyringHHDataProxy* TestKeyringConduit::hhProxy() const
{
	return hhDataProxy;
}

KeyringHHDataProxy* TestKeyringConduit::backupProxy() const
{
	return backupDataProxy;
}

KeyringHHDataProxy* TestKeyringConduit::pcProxy() const
{
	return pcDataProxy;
}

