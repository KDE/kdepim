#include "format.h"
#include "mmap_manager.h"
#include <cstring>

#include <cstring>

template <typename T>
memvector<T>::memvector( std::string fname ):
	data_( new mmap_manager( fname ) )
{
	if ( !data_->size() ) {
		data_->resize( byte_io::byte_lenght<unsigned>() );
		byte_io::write<unsigned>( data_->rw_base( 0 ), 0 );
	}
}

template <typename T>
memvector<T>::~memvector() {
}

template <typename T>
void memvector<T>::print( std::ostream& out ) const {
	out << format( "size(): %s\n" ) % size();
	for ( const_iterator first = begin(), past = end(); first != past; ++first ) {
		out << *first << std::endl;
	}
}

template <typename T>
void memvector<T>::resize( size_type n_s ) {
	if ( size() >= n_s ) return;

	data_->resize( n_s * byte_io::byte_lenght<value_type>() + byte_io::byte_lenght<unsigned>() );
	iterator p_end = end();
	byte_io::write<unsigned>( data_->rw_base( 0 ), n_s );
	while ( operator !=<unsigned, unsigned>(p_end, end()) ) {
		*p_end = value_type();
		++p_end;
		
	}
}

template<typename T>
void memvector<T>::insert( const_iterator where, const value_type v ) {
	assert( !( where < begin() ) );
	assert( where <= end() );
	const unsigned to_idx = where.raw() - data_->ronly_base( 0 );
	data_->resize( ( size() + 1 ) * byte_io::byte_lenght<value_type>() + byte_io::byte_lenght<unsigned>() );
	unsigned char* to = data_->rw_base( to_idx );
	// make space:
	std::memmove( to + byte_io::byte_lenght<value_type>(), to, end().raw() - to );
	byte_io::write<value_type>( to, v );
	byte_io::write<unsigned>( data_->rw_base( 0 ), size() + 1 );
}

template <typename T>
void memvector<T>::erase( iterator where ) {

	assert( size() );
	assert( !( where < begin() ) );
	assert( where < end() );

	iterator next = where;
	++next;
	std::memmove( const_cast<unsigned char*>( where.raw() ), next.raw(), end().raw() - next.raw() );
	byte_io::write<uint32_t>( data_->rw_base( 0 ), size() - 1 );
}

template <typename T>
void memvector<T>::clear() {
	data_->resize( byte_io::byte_lenght<uint32_t>() );
	byte_io::write<uint32_t>( data_->rw_base( 0 ), 0 );
}

template<typename T>
void memvector<T>::remove( std::string fname ) {
	::unlink( fname.c_str() );
}


