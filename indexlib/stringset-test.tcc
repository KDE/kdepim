#include <boost/test/unit_test.hpp>
#include "stringset.h"

using namespace ::boost::unit_test;
namespace stringset_test {

const char* fname = "stringset-test-delete-me";
void cleanup() {
	stringset::remove( fname );
}

void simple() {
	cleanup();
	stringset set( fname );
	set.add( "string1" );
	set.add( "string2" );

	BOOST_CHECK( set.count( "string1" ) );
	BOOST_CHECK( set.count( "string2" ) );

	BOOST_CHECK( !set.count( "string3" ) );
	BOOST_CHECK( !set.count( "other" ) );
}

void empty() {
	cleanup();
	stringset set( fname );
	BOOST_CHECK( set.empty() );
}


void persistent() {
	cleanup();
	{
		stringset set( fname );
		set.add( "string" );
		set.add( "victor" );
		set.add( "file" );

		BOOST_CHECK( set.count( "string" ) );
		BOOST_CHECK( set.count( "victor" ) );
		BOOST_CHECK( set.count( "file" ) );
	}
	{
		stringset set( fname );
		BOOST_CHECK( set.count( "string" ) );
		BOOST_CHECK( set.count( "victor" ) );
		BOOST_CHECK( set.count( "file" ) );
	}
}

void iterator() {
	cleanup();
	stringset set( fname );
	set.add( "string" );

	stringset::const_iterator iter = set.begin();

	BOOST_CHECK_EQUAL( std::string( "string" ), *iter );
	BOOST_CHECK_EQUAL( set.begin(), iter );
	BOOST_CHECK( !( set.end() == iter ) );
	++iter;
	BOOST_CHECK_EQUAL( set.end(), iter );
}

void order() {
	cleanup();
	stringset set( fname );

	set.add( "two" );
	set.add( "wlast" );
	set.add( "one" );

	stringset::const_iterator iter = set.begin();

	BOOST_CHECK_EQUAL( *iter, std::string( "one" ) );
	++iter;
	BOOST_CHECK_EQUAL( *iter, std::string( "two" ) );
	++iter;
	BOOST_CHECK_EQUAL( *iter, std::string( "wlast" ) );
	++iter;
	BOOST_CHECK_EQUAL( iter, set.end() );
}

void order_of() {
	cleanup();
	stringset set( fname );
	set.add( "one" );
	BOOST_CHECK_EQUAL( set.order_of( "one" ), 0 );
	BOOST_CHECK_EQUAL( set.order_of( "two" ), unsigned( -1 ) );
	set.add( "two" );
	BOOST_CHECK_EQUAL( set.order_of( "two" ), 1 );
	set.add( "before" );
	BOOST_CHECK_EQUAL( set.order_of( "two" ), 2 );
	BOOST_CHECK_EQUAL( set.order_of( "one" ), 1 );
	BOOST_CHECK_EQUAL( set.order_of( "before" ), 0 );
}

void id_of() {
	cleanup();
	stringset set( fname );
	set.add( "one" );
	BOOST_CHECK_EQUAL( set.id_of( "one" ), 0 );
	BOOST_CHECK_EQUAL( set.id_of( "two" ), unsigned( -1 ) );
	set.add( "two" );
	BOOST_CHECK_EQUAL( set.id_of( "two" ), 1 );
	set.add( "before" );
	BOOST_CHECK_EQUAL( set.id_of( "two" ), 1 );
	BOOST_CHECK_EQUAL( set.id_of( "one" ), 0 );
	BOOST_CHECK_EQUAL( set.id_of( "before" ), 2 );
}

void add_return() {
	cleanup();
	stringset set( fname );
	BOOST_CHECK_EQUAL( set.add( "one" ), 0 );
	BOOST_CHECK_EQUAL( set.add( "two" ), 1 );
	BOOST_CHECK_EQUAL( set.add( "before" ), 2 );
}

void lower() {
	cleanup();
	stringset set( fname );
	set.add( "aab" );
	set.add( "aac" );
	set.add( "aba" );
	set.add( "abc" );
	set.add( "acc" );

	BOOST_CHECK_EQUAL( std::string( *set.lower_bound( "ab" ) ), "aba" );
	BOOST_CHECK_EQUAL( std::string( *set.lower_bound( "abz" ) ), "acc" );
}

void lower_upper() {
	cleanup();
	stringset set( fname );
	set.add( "aab" );
	set.add( "aac" );
	set.add( "aba" );
	set.add( "abc" );
	set.add( "acc" );

	std::pair<stringset::const_iterator,stringset::const_iterator> limits;
	stringset::const_iterator& upper = limits.first;
	stringset::const_iterator& lower = limits.second;


	limits = set.upper_lower( "ab" );
	BOOST_CHECK_EQUAL( std::distance( upper, lower ), 2u );
	BOOST_CHECK_EQUAL( std::string( *upper ), "aba" );
	++upper;
	BOOST_CHECK_EQUAL( std::string( *upper ), "abc" );
	++upper;
	BOOST_CHECK( upper == lower );

	limits = set.upper_lower( "abc" );
	BOOST_CHECK_EQUAL( std::distance( upper, lower ), 1u );
	BOOST_CHECK_EQUAL( std::string( *upper ), "abc" );

	limits = set.upper_lower( "abz" );
	BOOST_CHECK_EQUAL( std::distance( upper, lower ), 0u );
}

void clear() {
	cleanup();
	stringset set( fname );
	set.add( "string1" );
	set.add( "string2" );
	set.add( "one" );
	set.add( "two" );
	set.add( "three" );

	set.clear();
	BOOST_CHECK_EQUAL( set.size(), 0u );
}

test_suite* get_suite() {
	test_suite* test = BOOST_TEST_SUITE( "Stringset tests" );
	test->add( BOOST_TEST_CASE( &simple ) );
	test->add( BOOST_TEST_CASE( &empty ) );
	test->add( BOOST_TEST_CASE( &persistent ) );
	test->add( BOOST_TEST_CASE( &iterator ) );
	test->add( BOOST_TEST_CASE( &order ) );
	test->add( BOOST_TEST_CASE( &order_of ) );
	test->add( BOOST_TEST_CASE( &id_of ) );
	test->add( BOOST_TEST_CASE( &add_return ) );
	test->add( BOOST_TEST_CASE( &lower ) );
	test->add( BOOST_TEST_CASE( &lower_upper ) );
	test->add( BOOST_TEST_CASE( &clear ) );
	return test;
}

} // namespace

