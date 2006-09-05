#include <boost/test/unit_test.hpp>
using namespace ::boost::unit_test;

#include <unistd.h>
#include "stringarray.h"

namespace stringarray_test {

const char* fname = "test.stringarray-delete-me";
void cleanup() {
	stringarray::remove( fname );
}

void test_size() {
	stringarray test( fname );
	test.add( "one" );
	test.add( "one" );
	test.add( "one" );
	test.add( "one" );
	//BOOST_CHECK_EQUAL( test.size(), 4 );
	cleanup();
}

void test_put_recover() {
	stringarray test( fname );
	BOOST_CHECK_EQUAL( test.add( "one" ), 0 );
	BOOST_CHECK_EQUAL( test.add( "two" ), 1 );
	BOOST_CHECK_EQUAL( test.add( "three" ), 2 );
	BOOST_CHECK_EQUAL( test.add( "four" ), 3 );

	BOOST_CHECK_EQUAL( test.get( 0 ), "one" );
	BOOST_CHECK_EQUAL( test.get( 1 ), "two" );
	BOOST_CHECK_EQUAL( test.get( 2 ), "three" );
	BOOST_CHECK_EQUAL( test.get( 3 ), "four" );

	cleanup();
}

void test_persistent() {
	{
		stringarray test( fname );
		BOOST_CHECK_EQUAL( test.add( "one" ), 0 );
		BOOST_CHECK_EQUAL( test.add( "two" ), 1 );
		BOOST_CHECK_EQUAL( test.add( "three" ), 2 );
		BOOST_CHECK_EQUAL( test.add( "four" ), 3 );
	}
	{
		stringarray test( fname );

		//BOOST_CHECK_EQUAL( test.size(), 4 );
		BOOST_CHECK_EQUAL( test.get( 0 ), "one" );
		BOOST_CHECK_EQUAL( test.get( 1 ), "two" );
		BOOST_CHECK_EQUAL( test.get( 2 ), "three" );
		BOOST_CHECK_EQUAL( test.get( 3 ), "four" );

	}
	cleanup();
}

void cstr() {
	stringarray test( fname );

	test.add( "one" );
	test.add( "two" );
	test.add( "three" );
	test.add( "four" );

	BOOST_CHECK( !strcmp( test.get_cstr( 0 ), "one" ) );
	BOOST_CHECK(  strcmp( test.get_cstr( 0 ), "not one" ) );
	BOOST_CHECK( !strcmp( test.get_cstr( 1 ), "two" ) );
	BOOST_CHECK( !strcmp( test.get_cstr( 2 ), "three" ) );
	BOOST_CHECK( !strcmp( test.get_cstr( 3 ), "four" ) );

	cleanup();
}

void erase() {
	stringarray test( fname );

	test.add( "one" );
	test.add( "two" );
	test.add( "three" );
	test.add( "four" );

	test.erase( 1 );
	BOOST_CHECK_EQUAL( test.get( 0 ), "one" );
	BOOST_CHECK_EQUAL( test.get( 1 ), "three" );
	BOOST_CHECK_EQUAL( test.size(), 3u );
	cleanup();
}


test_suite* get_suite() {
	test_suite* test = BOOST_TEST_SUITE( "Memvector tests" );
	test->add( BOOST_TEST_CASE( &test_size ) );
	test->add( BOOST_TEST_CASE( &test_put_recover ) );
	test->add( BOOST_TEST_CASE( &test_persistent ) );
	test->add( BOOST_TEST_CASE( &cstr ) );
	test->add( BOOST_TEST_CASE( &erase ) );
	return test;

}

} //namespace
