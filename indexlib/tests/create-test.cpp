#include <boost/test/unit_test.hpp>
#include "create.h"
#include "index.h"

using namespace ::boost::unit_test;

namespace create_test {

const char* fname = "create-test-delete-me/////";
	
void cleanup() {
	indexlib::remove( fname );
}

void simple() {
	cleanup();
	std::auto_ptr<indexlib::index> ptr = indexlib::create( fname );
	BOOST_CHECK( ptr.get() );
}

test_suite* get_suite() {
	test_suite* test = BOOST_TEST_SUITE( "Create tests" );
	test->add( BOOST_TEST_CASE( &simple ) );
	return test;
}

}

