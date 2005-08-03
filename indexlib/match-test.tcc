#include <boost/test/unit_test.hpp>
#include "match.h"

using namespace ::boost::unit_test;
namespace match_test {

void cleanup() {
}

void simple() {
	cleanup();
	Match m( "pat" );
	BOOST_CHECK_EQUAL( m.process( "not here" ), false );
	BOOST_CHECK_EQUAL( m.process( "p a t" ), false );
	BOOST_CHECK_EQUAL( m.process( "pa t" ), false );

	
	BOOST_CHECK_EQUAL( m.process( "pat" ), true );
	BOOST_CHECK_EQUAL( m.process( "pattern" ), true );
	BOOST_CHECK_EQUAL( m.process( " pat " ), true );
	BOOST_CHECK_EQUAL( m.process( "zpat patx ipato " ), true );
}

void empty() {
	cleanup();
	{
		Match m( "pat" );
		BOOST_CHECK( !m.process( "" ) );
	}
	{
		Match m( "" );
		BOOST_CHECK( m.process( "" ) );
		BOOST_CHECK( m.process( "string" ) );
	}
}
	

void string() {
	cleanup();
	Match m( std::string( "pat" ) );

	BOOST_CHECK_EQUAL( m.process( std::string( "not here" ) ), false );
	BOOST_CHECK_EQUAL( m.process( std::string( "here pattern" ) ), true );
}

void casesensitive() {
	cleanup();
	Match m( std::string( "pat" ), ~Match::caseinsensitive );

	BOOST_CHECK_EQUAL( m.process( std::string( "PAT" ) ), false );
	BOOST_CHECK_EQUAL( m.process( std::string( "aPATa" ) ), false );
	BOOST_CHECK_EQUAL( m.process( std::string( "pAt" ) ), false );
	BOOST_CHECK_EQUAL( m.process( std::string( "pattern" ) ), true );
}

void caseinsensitive() {
	cleanup();
	Match m( std::string( "pat" ), Match::caseinsensitive );

	BOOST_CHECK_EQUAL( m.process( std::string( "PAT" ) ), true );
	BOOST_CHECK_EQUAL( m.process( std::string( "aPATa" ) ), true );
	BOOST_CHECK_EQUAL( m.process( std::string( "pAt" ) ), true );
	BOOST_CHECK_EQUAL( m.process( std::string( "pattern" ) ), true );
}


test_suite* get_suite() {
	test_suite* test = BOOST_TEST_SUITE( "Match tests" );
	test->add( BOOST_TEST_CASE( &simple ) );
	test->add( BOOST_TEST_CASE( &empty ) );
	test->add( BOOST_TEST_CASE( &string ) );
	test->add( BOOST_TEST_CASE( &casesensitive ) );
	test->add( BOOST_TEST_CASE( &caseinsensitive ) );
	return test;
}

} // namespace

