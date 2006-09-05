#include <boost/test/unit_test.hpp>
#include "ifile.h"
#include <string>
#include <stdarg.h>

using namespace ::boost::unit_test;
namespace ifile_test {
//using indexlib::detail::ifile;
const char* fname = "ifile-test-delete-me";
void cleanup() {
	ifile::remove( fname );
}

inline
bool check_results( const ifile& ifi, const char* str, ... ) {
	const char* s;
	va_list args;
	va_start( args, str );
	std::vector<unsigned> res = ifi.search( str )->list();
	unsigned i = 0;

	while ( s = va_arg( args, const char* ) ) {
		if ( i == res.size() ) return false;
		if ( std::string( s ) != ifi.lookup_docname( res[ i++ ] ) ) return false;
	}
	va_end( args );
	return i == res.size();
}


inline
unsigned count_results( const ifile& ifi, const char* str ) {
	return ifi.search( str )->list().size();
}

void simple() {
	cleanup();
	ifile ifi( fname );
	ifi.add( "this", "doc" );
	BOOST_CHECK_EQUAL( ifi.search( "this" )->list().size(), 1u );
	BOOST_CHECK_EQUAL( ifi.search( "this" )->list()[ 0 ], 0 );
	BOOST_CHECK_EQUAL( ifi.lookup_docname( ifi.search( "this" )->list()[ 0 ] ), "doc" );
	ifi.add( "that", "doc2" );
	BOOST_CHECK_EQUAL( ifi.search( "this" )->list().size(), 1u );
	BOOST_CHECK_EQUAL( ifi.search( "this" )->list()[ 0 ], 0 );
	BOOST_CHECK_EQUAL( ifi.lookup_docname( ifi.search( "this" )->list()[ 0 ] ), "doc" );
	
	BOOST_CHECK_EQUAL( ifi.search( "that" )->list().size(), 1u );
	BOOST_CHECK_EQUAL( ifi.search( "that" )->list()[ 0 ], 1 );
	BOOST_CHECK_EQUAL( ifi.lookup_docname( ifi.search( "that" )->list()[ 0 ] ), "doc2" );
}

void ndocs() {
	cleanup();
	ifile ifi( fname );
	ifi.add( "one", "one" );
	ifi.add( "one", "two" );
	BOOST_CHECK_EQUAL( ifi.ndocs(), 2 );

	ifi.add( "one", "three" );
	ifi.add( "one", "four" );

	BOOST_CHECK_EQUAL( ifi.ndocs(), 4 );
	BOOST_CHECK_EQUAL( ifi.lookup_docname( 0 ), std::string( "one" ) );
	BOOST_CHECK_EQUAL( ifi.lookup_docname( 1 ), std::string( "two" ) );
	BOOST_CHECK_EQUAL( ifi.lookup_docname( 2 ), std::string( "three" ) );
	BOOST_CHECK_EQUAL( ifi.lookup_docname( 3 ), std::string( "four" ) );
}

void space() {
	cleanup();
	ifile ifi( fname );

	ifi.add( "one two three", "doc" );
	BOOST_CHECK_EQUAL( ifi.search( "two" )->list().size(), 1 );
}

void numbers() {
	cleanup();
	ifile ifi( fname );

	ifi.add( "one 123 123456789 four444 five", "doc" );
	BOOST_CHECK_EQUAL( ifi.search( "123" )->list().size(), 1 );
	BOOST_CHECK_EQUAL( ifi.search( "123456789" )->list().size(), 1 );
	BOOST_CHECK_EQUAL( ifi.search( "four444" )->list().size(), 1 );
	BOOST_CHECK_EQUAL( ifi.search( "five" )->list().size(), 1 );
}

void partial() {
	cleanup();
	ifile ifi( fname );
	ifi.add( "longword", "doc_0" );

	BOOST_CHECK_EQUAL( ifi.search( "l" )->list().size(), 1u );
	BOOST_CHECK_EQUAL( ifi.search( "long" )->list().size(), 1u );
	BOOST_CHECK_EQUAL( ifi.search( "longword" )->list().size(), 1u );

	BOOST_CHECK_EQUAL( ifi.search( "longword" )->list().size(), 1u );

	ifi.add( "longnord", "doc_1" );
	BOOST_CHECK_EQUAL( ifi.search( "l" )->list().size(), 2u );
	BOOST_CHECK_EQUAL( ifi.search( "long" )->list().size(), 2u );
	BOOST_CHECK_EQUAL( ifi.search( "longw" )->list().size(), 1u );
	BOOST_CHECK_EQUAL( ifi.search( "longn" )->list().size(), 1u );
}

void several() {
	cleanup();
	ifile ifi( fname );
	ifi.add( "one two three four", "0" );
	ifi.add( "two three four", "1" );
	ifi.add( "something else", "2" );
	ifi.add( "something two", "3" );
	ifi.add( "two something four", "4" );
	ifi.add( "else something", "5" );
	ifi.add( "else four", "6" );

	BOOST_CHECK_EQUAL( count_results( ifi, "one" ), 1u );
	BOOST_CHECK_EQUAL( count_results( ifi, "one two three four" ), 1u );
	BOOST_CHECK_EQUAL( count_results( ifi, "two three four" ), 2u );

	BOOST_CHECK_EQUAL( count_results( ifi, "one two" ), 1u );
	BOOST_CHECK_EQUAL( count_results( ifi, "one" ), 1u );

	BOOST_CHECK_EQUAL( count_results( ifi, "something else" ), 2u );
	BOOST_CHECK_EQUAL( count_results( ifi, "something two" ), 2u );
}

void remove_doc() {
	cleanup();
	ifile ifi( fname );
	ifi.add( "one two three four", "0" );
	ifi.add( "two three four", "1" );
	ifi.add( "three four five", "2" );
	ifi.remove_doc( "1" );

	BOOST_CHECK( check_results( ifi, "one", "0", NULL ) );
	BOOST_CHECK( check_results( ifi, "two", "0", NULL ) );
	BOOST_CHECK( check_results( ifi, "three", "0", "2", NULL ) );
	BOOST_CHECK_EQUAL( count_results( ifi, "four" ), 0u );
}

test_suite* get_suite() {
	test_suite* test = BOOST_TEST_SUITE( "Ifile tests" );
	test->add( BOOST_TEST_CASE( &simple ) );
	test->add( BOOST_TEST_CASE( &ndocs ) );
	test->add( BOOST_TEST_CASE( &space ) );
	//test->add( BOOST_TEST_CASE( &numbers ) );
	test->add( BOOST_TEST_CASE( &partial ) );
	test->add( BOOST_TEST_CASE( &several ) );
	test->add( BOOST_TEST_CASE( &remove) );
	return test;
}

} // namespace

