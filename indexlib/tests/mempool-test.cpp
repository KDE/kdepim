#include <boost/test/unit_test.hpp>
#include "mempool.h"
#include "leafdata.h"

using namespace ::boost::unit_test;
namespace mempool_test {
const char* fname = "mempool-test-delete-me";
void cleanup() {
	::unlink( fname );
}

void deallocate() {
	cleanup();
	mempool<leaf_data_pool_traits> pool( std::auto_ptr<memory_manager>( new mmap_manager( fname ) ) );

	std::vector<leafdataptr> pointers;
	for ( int i = 0; i != 32; ++i ) {
		pointers.push_back( pool.allocate( 16 ) );
		leafdata::init( pointers.back() );
	}
	const unsigned size =  pool.size();

	for ( int i = 0; i != pointers.size(); ++i ) {
		pool.deallocate(pointers.at(i));
	}

	for ( int i = 0; i != 32; ++i ) {
		pointers.push_back( pool.allocate( 16 ) );
		leafdata::init( pointers.back() );
	}
	BOOST_CHECK_EQUAL( size, pool.size() );
}

void large() {
	cleanup();
	mempool<leaf_data_pool_traits> pool( std::auto_ptr<memory_manager>( new mmap_manager( fname ) ) );
	
	pool.allocate( 4095 );
	pool.allocate( 4097 );
	pool.allocate( 4096*2 );
	pool.allocate( 4096*4 );
	pool.allocate( 4096*8 );
}

test_suite* get_suite() {
	test_suite* test = BOOST_TEST_SUITE( "Mempool Tests" );
	test->add( BOOST_TEST_CASE( &deallocate ) );
	test->add( BOOST_TEST_CASE( &large ) );
	return test;
}

} // namespace

