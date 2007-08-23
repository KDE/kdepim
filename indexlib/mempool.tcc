#include "format.h"
#include <iostream>
#include <cstring>
#include "logfile.h"
#include "compat.h"

/* BASIC ALGORITHM AND STRUCTURE
 *
 * This is a memory pool manager which works by dividing its memory into
 * blocks (all blocks have a size which is a power-of-two). Each block is either
 * in use or in its corresponding free list.
 *
 * The free lists are doubly linked and there are head-pointers in
 * the first page of the pool. 
 *
 * POOL ORGANIZATION:
 *
 * FIRST PAGE
 * max_order_: 32 bits
 * [pseudo-order 0]: 32 bits
 * [pseudo-order 1]: 32 bits
 * [pseudo-order 2]: 32 bits
 * [list order 3]: 32 bits
 * [list order 4]: 32 bits
 * [list order 5]: 32 bits
 * [list order 5]: 32 bits
 * ...
 * [list order max_order_]: 32 bits
 *
 * SUBSEQUENT PAGES:
 * nodes*
 *
 */

template <typename Traits>
mempool<Traits>::mempool( std::auto_ptr<memory_manager> source ):
	manager_( source ),
	max_order_( 0 )
{
	if ( !manager_->size() ) init_memory();
	max_order_.assign( memory_reference<uint32_t>( manager_->rw_base( 0 ) ) );
	if ( !max_order_ ) {
		max_order_ = order_of( traits_type::max_size() );
	}
	traits_type::set_manager( manager_.get() );
}

template <typename Traits>
typename mempool<Traits>::data_typeptr mempool<Traits>::allocate( unsigned size ) {
	if ( size < traits_type::min_size() ) size = traits_type::min_size();
	max_order_ = kMax<uint32_t>( order_of( size ), max_order_ );
	const unsigned order = kMax<unsigned>( order_of( size ), min_order_for_free_node );
	if ( uint32_t res = free_list( order ) ) {
		free_list( order ) = get_node( res )->next();
		if ( free_list( order ) ) get_node( free_list( order ) )->set_prev( 0 );
		logfile() << format( "%s( %s ): (order %s) Returning %s\n" ) % __PRETTY_FUNCTION__ % size % order % res;
		return data_typeptr::cast_from_uint32( res );
	} else {
		logfile() << format( "For size %s going up to %s\n") % size % max_order_;
		for ( unsigned bigger = order + 1; bigger <= max_order_; ++bigger ) {
			if ( uint32_t res = free_list( bigger ) ) {
				while ( bigger > order ) {
					break_up( res );
					--bigger;
				}
				logfile() << format( "%s( %s ): recursing\n" ) % __PRETTY_FUNCTION__ % size;
				return allocate( size );
			}
		}
		const unsigned old_size = manager_->size();
		manager_->resize( manager_->size() + order_to_size( order ) );
		max_order_.assign( memory_reference<uint32_t>( manager_->rw_base( 0 ) ) );
		fill_into_list( old_size, order );
		return allocate( size );
	}
	
}

template <typename Traits>
void mempool<Traits>::fill_into_list( unsigned next_block, unsigned order ) {
	logfile() << format( "%s( %s, %s )\n" ) % __PRETTY_FUNCTION__ % next_block % order;
	const unsigned size = manager_->size();
	const unsigned min_order =
		kMax<unsigned>( min_order_for_free_node, order_of( traits_type::min_size() ) );
	while ( next_block < size && order >= min_order ) {
		const unsigned block_size = order_to_size( order );
		while ( ( size - next_block ) >= block_size ) {
			insert_into_list( next_block, order );
			next_block += block_size;
		}
		--order;
	}
}

template <typename Traits>
void mempool<Traits>::fill_into_list( unsigned next_block ) {
	fill_into_list( next_block, max_order_ );
}

template <typename Traits>
void mempool<Traits>::init_memory() {
	manager_->resize( 4096 );
}

template <typename Traits>
void mempool<Traits>::print( std::ostream& out ) const {
	uint32_t iterator = 0, end = manager_->size();

	out << "free lists:\n";
	for ( unsigned i = 0; i != max_order_ + 1; ++i ) {
		out << "\t" << i << ": " << free_list( i ) << '\n';
	}
	out << '\n';

	iterator = order_to_size( max_order_ );

	while ( iterator < end ) {
		data_typeptr p = data_typeptr::cast_from_uint32( iterator );
		if ( traits_type::is_free( p ) ) {
			out << '[' << iterator << "] free_node:\n";
			list_nodeptr node = get_node( iterator );
			out << "order:\t" << node->order() << '\n';
			out << "prev:\t" << node->prev() << '\n';
			out << "next:\t" << node->next() << '\n';
			out << '\n';
			iterator += order_to_size( node->order() );
		} else {
			out << format( "size_of(): %s\n" ) % traits_type::size_of( p );
			traits_type::print( out, p );
			iterator += traits_type::size_of( p );
		}
	}
}

template <typename Traits>
memory_reference<uint32_t> mempool<Traits>::free_list( unsigned order ) {
	assert( order );
	return memory_reference<uint32_t>( manager_->rw_base( order * byte_io::byte_lenght<uint32_t>() ) );
}

template <typename Traits>
typename mempool<Traits>::list_nodeptr mempool<Traits>::get_node( uint32_t p ) const {
	assert( p );
	list_nodeptr res = list_nodeptr::cast_from_uint32( p + Traits::type_offset() );
	res->set_parent( this );
	return res;
}

template <typename Traits>
void mempool<Traits>::remove_from_list( uint32_t where, unsigned order ) {
	logfile() << format( "%s( %s, %s )\n" ) % __PRETTY_FUNCTION__ % where % order;
	list_nodeptr node = get_node( where );
	if ( node->next() ) get_node( node->next() )->set_prev( node->prev() );
	if ( node->prev() ) get_node( node->prev() )->set_next( node->next() );
	if ( free_list( order ) == where ) free_list( order ) = node->next();
}

template <typename Traits>
void mempool<Traits>::insert_into_list( uint32_t where, unsigned order ) {
	logfile() << format( "%s( %s, %s )\n" ) % __PRETTY_FUNCTION__ % where % order;
	traits_type::mark_free( data_typeptr::cast_from_uint32( where ) );
	list_nodeptr new_node = get_node( where );
	new_node->set_order( order );
	new_node->set_next( free_list( order ) );
	new_node->set_prev( 0 );
	if ( free_list( order ) ) {
		get_node( free_list( order ) )->set_prev( where );
	}
	free_list( order ) = where;
}

template <typename Traits>
void mempool<Traits>::break_up( uint32_t where ) {
	logfile() << "break_up( " << where << " )\n";
	assert( traits_type::is_free( data_typeptr::cast_from_uint32( where ) ) );
	const unsigned old_order = get_node( where )->order();
	assert( old_order );
	const unsigned new_order = old_order - 1;
	remove_from_list( where, old_order );
	insert_into_list( where + order_to_size( new_order ), new_order );
	insert_into_list( where, new_order );
}

template <typename Traits>
bool mempool<Traits>::join( data_typeptr& node, unsigned order ) {
	logfile() << format( "%s( %s, %s )\n" ) % __PRETTY_FUNCTION__ % node.cast_to_uint32() % order;
	const uint32_t byte_idx = node.cast_to_uint32();
	const unsigned block_size = order_to_size( order );
	const unsigned block_idx = byte_idx / block_size;
	uint32_t partner;
	if ( block_idx % 2 ) {
		partner = byte_idx - block_idx;
	} else {
		partner = byte_idx + block_idx;
	}
	if ( partner >= manager_->size() ) return false;
	bool res = traits_type::is_free( data_typeptr::cast_from_uint32( partner ) )
		&& get_node( partner )->order() == order;
	if ( res ) {
		node = ( block_idx % 2 ) ? data_typeptr::cast_from_uint32( partner ) : node;
		remove_from_list( byte_idx, order );
		remove_from_list( partner, order );
		insert_into_list( node.cast_to_uint32(), order + 1 );
	}
	return res;
}

template <typename Traits>
void mempool<Traits>::deallocate( data_typeptr data ) {
	logfile() << "deallocate( " << data << " )\n";
	unsigned order = order_of( size_of( data ) );
	while ( ( order < max_order_ ) && join( data, order ) ) ++order;
	deallocate( data, order );
}

template <typename Traits>
void mempool<Traits>::deallocate( data_typeptr data, unsigned order ) {
	logfile() << format( "%s( %s, %s )\n" ) % __PRETTY_FUNCTION__ % data.cast_to_uint32() % order;
	assert( data );
	traits_type::mark_free( data );
	insert_into_list( data.cast_to_uint32(), order );
}

template <typename Traits>
typename mempool<Traits>::data_typeptr mempool<Traits>::reallocate( data_typeptr data, unsigned size ) {
	logfile() << format( "%s( %s, %s)\n" ) % __PRETTY_FUNCTION__ % data % size;
	max_order_ = kMax<uint32_t>( max_order_, order_of( max_order_ ) );
	const unsigned original_size = size_of( data );
	unsigned char* temporary = static_cast<unsigned char*>( operator new( original_size ) );
	std::memmove( temporary, data.raw_pointer(), original_size );

	unsigned current = order_of( original_size );
	unsigned desired = order_of( size );
	while ( desired < current && join( data, current ) ) ++current;
	if ( desired != current ) deallocate( data, current );
	data = allocate( size );
	std::memcpy( data.raw_pointer(), temporary, original_size );
	operator delete( temporary );
	return data;
}

