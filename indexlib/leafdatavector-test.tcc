#include <boost/test/unit_test.hpp>
#include <map>
#include "leafdatavector.h"

using namespace ::boost::unit_test;
namespace leafdatavector_test {

const char* fname = "leafdatavector-test-delete-me";
void cleanup() {
	leafdatavector::remove( fname );
}

void simple() {
	cleanup();
	leafdatavector f( fname );
	f.add( 0, 1 );
	BOOST_CHECK_EQUAL( f.get( 0 ).size(), 1u );
	BOOST_CHECK_EQUAL( f.get( 0 )[ 0 ], 1u );
	f.add( 0, 2 );
	BOOST_CHECK_EQUAL( f.get( 0 ).size(), 2u );
}

void persistent() {
	cleanup();
	{
		leafdatavector f( fname );
		f.add( 0, 1 );
	}
	{
		leafdatavector f( fname );
		BOOST_CHECK_EQUAL( f.get( 0 )[ 0 ], 1u );
	}
}

void complicated() {
	cleanup();
	leafdatavector f( fname );

	f.add( 0, 1 );
	f.add( 0, 3 );
	f.add( 1, 3 );
	f.add( 0, 2 );
	f.add( 0, 4 );
	f.add( 1, 8 );
	f.add( 2, 4 );
	f.add( 1, 5 );
	f.add( 2, 5 );
	f.add( 0, 5 );
	f.add( 0, 9 );

	BOOST_CHECK_EQUAL( f.get( 0 ).size(), 6u );
	BOOST_CHECK_EQUAL( f.get( 1 ).size(), 3u );
	BOOST_CHECK_EQUAL( f.get( 2 ).size(), 2u );
	std::vector<unsigned> one = f.get( 1 );
	std::sort( one.begin(), one.end() );
	BOOST_CHECK_EQUAL( one.size(), 3u );
	BOOST_CHECK_EQUAL( one[ 0 ], 3u );
	BOOST_CHECK_EQUAL( one[ 1 ], 5u );
	BOOST_CHECK_EQUAL( one[ 2 ], 8u );
}

void unique() {
	cleanup();
	leafdatavector f( fname );

	f.add( 0, 1 );
	f.add( 0, 1 );
	f.add( 0, 1 );

	BOOST_CHECK_EQUAL( f.get( 0 ).size(), 1u );

	f.add( 0, 4 );
	BOOST_CHECK_EQUAL( f.get( 0 ).size(), 2u );

	f.add( 0, 1 );
	f.add( 0, 4 );
	
	BOOST_CHECK_EQUAL( f.get( 0 ).size(), 2u );

}

void large() {
	cleanup();
	leafdatavector f( fname );
	std::map<uint, uint> counts;

	for ( uint i = 0; i != 32; ++i ) {
		for ( uint j = 0; j != 256 + 3; ++j ) {
			uint ref = i * ( j + 51 ) / 13 + i % 75 + j + 3;
			f.add( j, ref );
			++counts[ j ];
		}
	}
	for ( std::map<uint,uint>::const_iterator first = counts.begin(), past = counts.end();
			first != past; ++first ) {
		BOOST_CHECK_EQUAL( first->second, f.get( first->first ).size() );
	}

}

void one_zero() {
	cleanup();
	leafdatavector f( fname );

	f.add( 0, 0 );
	f.add( 0, 1 );
	f.add( 0, 3 );
	
	BOOST_CHECK_EQUAL( f.get( 0 ).size(), 3u );
	BOOST_CHECK_EQUAL( f.get( 0 )[ 0 ], 0u );
	BOOST_CHECK_EQUAL( f.get( 0 )[ 1 ], 1u );
	BOOST_CHECK_EQUAL( f.get( 0 )[ 2 ], 3u );

}
		

test_suite* get_suite() {
	test_suite* test = BOOST_TEST_SUITE( "leafdatavector tests" );
	test->add( BOOST_TEST_CASE( &simple ) );
	test->add( BOOST_TEST_CASE( &persistent ) );
	test->add( BOOST_TEST_CASE( &complicated ) );
	test->add( BOOST_TEST_CASE( &unique ) );
	test->add( BOOST_TEST_CASE( &large ) );
	test->add( BOOST_TEST_CASE( &one_zero ) );
	return test;
}

} // namespace

