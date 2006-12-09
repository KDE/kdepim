#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

int main( int argc, char **argv)
{
	CppUnit::TestFactoryRegistry &registry =
		CppUnit::TestFactoryRegistry::getRegistry();

	CppUnit::TextUi::TestRunner runner;
	runner.addTest( registry.makeTest() );

	// Run the tests.
	bool wasSucessful = runner.run();

	// Return error code 1 if the one of test failed.
	return wasSucessful ? 0 : 1;
}
