#include <boost/test/unit_test.hpp>
#include "tokenizer.h"

using namespace ::boost::unit_test;
namespace indexlib { namespace tests { namespace tokenizer_test {

using indexlib::detail::tokenizer;
using indexlib::detail::get_tokenizer;

void simple() {
	std::auto_ptr<tokenizer> tokenizer = get_tokenizer( "default" );
	std::vector<std::string> tokens = tokenizer->string_to_words( "one     ,as, ''#`:ThReE,  בבבאחי" );
	std::vector<std::string> expected;
	expected.push_back( "ONE" );
	expected.push_back( "AS" );
	expected.push_back( "THREE" );
	expected.push_back( "AAACE" );
	std::sort( tokens.begin(), tokens.end() );
	std::sort( expected.begin(), expected.end() );
	BOOST_CHECK_EQUAL( expected.size(), tokens.size() );
	for ( int i = 0; i < expected.size() && i < tokens.size(); ++i ) {
		BOOST_CHECK_EQUAL( expected[ i ], tokens[ i ] );
	}
}

test_suite* get_suite() {
	test_suite* test = BOOST_TEST_SUITE( "Tokenizer tests" );
	test->add( BOOST_TEST_CASE( &simple ) );
	return test;
}

}}} //namespaces
