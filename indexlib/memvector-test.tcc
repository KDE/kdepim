#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>
#include <iostream>
using namespace ::boost::unit_test;

#include <unistd.h>
#include "memvector.h"

namespace memvector_test {

const char* fname = "test.vector-delete-me";
void cleanup() {
	memvector<uint32_t>::remove( fname );
}
void test_size() {
	cleanup();
	memvector<uint32_t> test( fname );
	test.push_back( 1 );
	test.push_back( 2 );
	test.push_back( 3 );
	test.push_back( 4 );
	BOOST_CHECK_EQUAL( test.size(), 4u );
}

template <typename T>
void test_put_recover() {
	cleanup();
	memvector<T> test( fname );
	for ( int i = 0; i != 20; ++i ) {
		test.push_back( T( i*13 + i*i*45 + 23 ) );
	}
	for ( int i = 0; i != 20; ++i ) {
		BOOST_CHECK_EQUAL( test[ i ], T( i*13 + i*i*45 + 23 ) );
	}
}

void resize() {
	cleanup();
	memvector<uint32_t> test( fname );
	test.push_back( 1 );
	test.resize( 50 );
	BOOST_CHECK_EQUAL( test.size(), 50u );
}


void test_persistent() {
	cleanup();
	{
		memvector<uint32_t> test( fname );
		test.push_back( 1 );
		test.push_back( 2 );
		test.push_back( 3 );
		test.push_back( 4 );
		test.push_back( 5 );
	}
	{
		memvector<uint32_t> test( fname );
		BOOST_CHECK_EQUAL( test.size(), 5u );
		for ( unsigned i = 0; i != test.size(); ++i ) 
			BOOST_CHECK_EQUAL( test[ i ], i + 1 );
	}
}

void test_insert() {
	cleanup();
	memvector<uint16_t> test( fname );
	test.push_back( 12 );
	test.push_back( 12 );
	test.push_back( 12 );
	test.push_back( 12 );

	test.insert( test.begin() + 2, 13 );

	BOOST_CHECK_EQUAL( test.size(), 5u );
	BOOST_CHECK_EQUAL( test[ 0 ], 12u );
	BOOST_CHECK_EQUAL( test[ 1 ], 12u );
	BOOST_CHECK_EQUAL( test[ 2 ], 13u );
	BOOST_CHECK_EQUAL( test[ 3 ], 12u );
	BOOST_CHECK_EQUAL( test[ 4 ], 12u );
}

void test_iterator() {
	cleanup();
	memvector<unsigned> test( fname );
	test.push_back( 1 );
	test.push_back( 2 );

	BOOST_CHECK_EQUAL( test[ 0 ], 1u );
	BOOST_CHECK_EQUAL( test[ 1 ], 2u );
	
	BOOST_CHECK_EQUAL( *test.begin(),  1u );
	BOOST_CHECK_EQUAL( *( test.begin() + 1 ),  2u );

	memvector<unsigned>::iterator iter = test.begin();

	BOOST_CHECK_EQUAL( *iter,  1u );

	BOOST_CHECK( test.begin() == iter );

	*iter= 3;

	BOOST_CHECK_EQUAL( test[ 0 ],  3u );
	BOOST_CHECK_EQUAL( *iter,  3u );
	BOOST_CHECK_EQUAL( *test.begin(),  3u );

	++iter;
	
	BOOST_CHECK_EQUAL( *iter,  2u );
	
	*iter = 5;
	
	BOOST_CHECK_EQUAL( *iter,  5u );
	BOOST_CHECK_EQUAL( test[ 1 ],  5u );

	BOOST_CHECK_EQUAL( std::distance( test.begin(), test.end() ) , test.size() );
	test.push_back( 5 );
	BOOST_CHECK_EQUAL( std::distance( test.begin(), test.end() ) , test.size() );
	test.push_back( 5 );
	BOOST_CHECK_EQUAL( std::distance( test.begin(), test.end() ) , test.size() );
}

void test_iteration() {
	cleanup();
	memvector<unsigned> test( fname );

	test.push_back( 1 );
	test.push_back( 2 );
	test.push_back( 5 );
	test.push_back( 3 );

	memvector<unsigned>::const_iterator iter = test.begin();

	BOOST_CHECK( iter == test.begin() );
	BOOST_CHECK( iter != test.end() );

	BOOST_CHECK_EQUAL( *iter, 1u );
	++iter;
	BOOST_CHECK_EQUAL( *iter, 2u );
	iter += 2;
	BOOST_CHECK_EQUAL( *iter, 3u );
	*iter = 7;
	BOOST_CHECK_EQUAL( *iter, 7u );
	--iter;
	BOOST_CHECK_EQUAL( *iter, 5u );
	BOOST_CHECK( iter != test.end() );
	iter += 2;
	BOOST_CHECK( iter == test.end() );
}

void test_sort() {
	cleanup();
	memvector<unsigned> test( fname );
	test.push_back( 10 );
	test.push_back(  0 );
	test.push_back( 14 );
	test.push_back(  8 );
	test.push_back( 12 );
	test.push_back(  5 );
	test.push_back(  4 );
	test.push_back(  3 );

	
	BOOST_CHECK_EQUAL( *std::min_element( test.begin(), test.end() ),  0 );
	BOOST_CHECK( std::min_element( test.begin(), test.end() ) == test.begin() + 1 );
	BOOST_CHECK_EQUAL( *std::max_element( test.begin(), test.end() ), 14 );
	BOOST_CHECK( std::max_element( test.begin(), test.end() ) == test.begin() + 2 );

	std::sort( test.begin(), test.end() );
	BOOST_CHECK_EQUAL( test[ 0 ],  0 );
	BOOST_CHECK_EQUAL( test[ 1 ],  3 );
	BOOST_CHECK_EQUAL( test[ 2 ],  4 );
	BOOST_CHECK_EQUAL( test[ 3 ],  5 );
	BOOST_CHECK_EQUAL( test[ 4 ],  8 );
	BOOST_CHECK_EQUAL( test[ 5 ], 10 );
	BOOST_CHECK_EQUAL( test[ 6 ], 12 );
	BOOST_CHECK_EQUAL( test[ 7 ], 14 );
}

void remove() {
	{
		cleanup();
		memvector<unsigned> test( fname );
		test.push_back( 1 );
		BOOST_CHECK_EQUAL( test.size(), 1 );
	}
	memvector<unsigned>::remove( fname );
	memvector<unsigned> test( fname );
	BOOST_CHECK_EQUAL( test.size(), 0 );
}

void assign() {
	cleanup();
	memvector<uint32_t> test( fname );
	test.push_back( 2 );
	test[ 0 ] = 3;
	BOOST_CHECK_EQUAL( test[ 0 ], 3u );
}

void erase() {
	cleanup();
	memvector<uint32_t> test( fname );
	test.push_back(  2 );
	test.push_back(  4 );
	test.push_back(  8 );
	test.push_back( 16 );
	test.push_back( 32 );

	BOOST_CHECK_EQUAL( test.size(), 5u );
	test.erase( test.begin() + 1 );

	BOOST_CHECK_EQUAL( test[ 0 ],  2u );
	BOOST_CHECK_EQUAL( test[ 1 ],  8u );
	BOOST_CHECK_EQUAL( test[ 2 ], 16u );
	BOOST_CHECK_EQUAL( test[ 3 ], 32u );
	BOOST_CHECK_EQUAL( test.size(), 4u );

	test.erase( test.begin() + 3 );
	BOOST_CHECK_EQUAL( test[ 0 ],  2u );
	BOOST_CHECK_EQUAL( test[ 1 ],  8u );
	BOOST_CHECK_EQUAL( test[ 2 ], 16u );
	BOOST_CHECK_EQUAL( test.size(), 3u );

}

void clear() {
	cleanup();
	memvector<uint32_t> test( fname );
	test.push_back(  2 );
	test.push_back(  4 );
	test.push_back(  8 );
	test.push_back( 16 );
	test.push_back( 32 );

	test.clear();


	BOOST_CHECK_EQUAL( test.size(), 0u );
}

test_suite* get_suite() {
	test_suite* test = BOOST_TEST_SUITE( "Memvector tests" );
	test->add( BOOST_TEST_CASE( &test_size ) );
	test->add( BOOST_TEST_CASE( &test_put_recover<uint32_t> ) );
	test->add( BOOST_TEST_CASE( &test_put_recover<uint16_t> ) );
	test->add( BOOST_TEST_CASE( &test_put_recover<uint8_t> ) );
	test->add( BOOST_TEST_CASE( &resize ) );
	test->add( BOOST_TEST_CASE( &test_persistent ) );
	test->add( BOOST_TEST_CASE( &remove ) );
	test->add( BOOST_TEST_CASE( &assign ) );
	test->add( BOOST_TEST_CASE( &erase ) );
	test->add( BOOST_TEST_CASE( &clear ) );
	return test;

}

} // namespace


