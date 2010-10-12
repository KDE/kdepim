#ifndef EXAMPLETEST_H
#define EXAMPLETEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <qstring.h>

#include "kpilotlocallink.h"
#include "syncAction.h"

class VCalConduitTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( VCalConduitTest );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST_SUITE_END();

private:
	QString device;
	KPilotLocalLink *link;
	SyncAction::SyncMode::Mode syncMode;

public:
  void setUp();
  void tearDown();

  void testConstructor();
};

#endif
