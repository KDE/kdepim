#include "exampletest.h"

#include <config-kpilot.h>
#include "options.h"

CPPUNIT_TEST_SUITE_REGISTRATION( VCalConduitTest );


void VCalConduitTest::setUp()
{
	device = QString("testdevice");
	link = new KPilotLocalLink(0, "localLink");
	syncMode = SyncAction::SyncMode::eHotSync;
}


void VCalConduitTest::tearDown()
{
	delete link;
}


void VCalConduitTest::testConstructor()
{
	CPPUNIT_ASSERT( true == true );
}
