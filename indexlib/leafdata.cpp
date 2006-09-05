
/* This file is part of indexlib.
 * Copyright (C) 2005 Luís Pedro Coelho <luis@luispedro.org>
 *
 * Indexlib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation and available as file
 * GPL_V2 which is distributed along with indexlib.
 * 
 * Indexlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA
 * 
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of this program with any edition of
 * the Qt library by Trolltech AS, Norway (or with modified versions
 * of Qt that use the same license as Qt), and distribute linked
 * combinations including the two.  You must obey the GNU General
 * Public License in all respects for all of the code used other than
 * Qt.  If you modify this file, you may extend this exception to
 * your version of the file, but you are not obligated to do so.  If
 * you do not wish to do so, delete this exception statement from
 * your version.
 */

#include "leafdata.h"
#include "logfile.h"
#include <iostream>
#include <algorithm>
#include "boost-compat/next_prior.hpp"
#include "format.h"

namespace {
	memory_manager* manager = 0;
}

memory_manager* get_leafdata_manager() { return manager ; }
void set_leafdata_manager( memory_manager* m ) { manager = m; }

uint32_t leaf_data::get_reference( unsigned idx ) const {
	leafdata_iterator iter = begin();
	while ( idx-- ) { *iter; ++iter; }
	return *iter;
}

bool leaf_data::can_add( uint32_t ref ) const {
	if ( ( capacity() - usedbytes() ) > ( 1 + byte_io::byte_lenght<uint32_t>() ) ) return true;
	if ( capacity() == usedbytes() ) return false;
	uint32_t last = 0;
	for ( iterator first = begin(), past = end(); first != past; ++first ) {
		assert( first < past );
		last = *first;
		if ( last == ref ) return true;
	}
	return ( ref > last && ( ref - last ) < 256 );
}

bool leaf_data::has_reference( uint32_t ref ) const {
	for ( iterator first = begin(), past = end(); first != past; ++first ) {
		uint32_t here = *first;
		//logfile() << format( "leaf_data[%s]::has_reference( %s ): looking at %s\n" ) % idx_ % ref % here;

		if ( here == ref ) {
			//logfile() << format( "leaf_data[%s]::has_reference( %s ): true\n" ) % idx_ % ref;
			return true;
		}
	}
	//logfile() << format( "leaf_data[%s]::has_reference( %s ): false\n" ) % idx_ % ref;
	return false;
}

void leaf_data::add_reference( uint32_t ref ) {
	//logfile() << format( "leaf_data[%s]::add_reference( %s )\n" ) % idx_ % ref;
	assert( can_add( ref ) );
	if ( has_reference( ref ) ) return;
	iterator first = begin();
	const iterator past = end();
	unsigned value = 0;
	while ( first != past ) {
		value = *first;
		++first;
	}
	++ref;
	if ( usedbytes() ) ++value;
	unsigned char* target = const_cast<unsigned char*>( first.raw() );
	assert( target == my_base() + usedbytes() );
	if ( ref > value && ( ref - value ) < 256 ) {
		assert( ref != value );
		*target = ref - value;
		set_usedbytes( usedbytes() + 1 );
	} else {
		*target++ = 0;
		byte_io::write<uint32_t>( target, ref );
		set_usedbytes( usedbytes() + 1 + byte_io::byte_lenght<uint32_t>() );
	}
	assert( usedbytes() <= capacity() );
}

void leaf_data::remove_reference( uint32_t ref ) {
	//logfile() << format( "leaf_data[%s]::remove_reference( %s )\n" ) % idx_ % ref;
	unsigned idx = 0;
	iterator first = begin();
	const iterator past = end();
	for ( ; first != past; ++first ) {
		if ( *first == ref ) break;
		++idx;
	}
	if ( first != past ) {
		//assert( get_reference( idx ) == ref );
		iterator next = boost::next( first );
		unsigned nbytes = end().raw() - first.raw();
		std::memmove( const_cast<unsigned char*>( first.raw() ), next.raw(), nbytes );
		set_usedbytes( usedbytes() - nbytes );
		unsigned char* iter = const_cast<unsigned char*>( first.raw() );
		for ( ; iter < end().raw(); ++iter) {
			if (*iter) --*iter;
			else {
				++iter;
				byte_io::write<uint32_t>(iter,byte_io::read<uint32_t>(iter)-1);
				iter += byte_io::byte_lenght<uint32_t>();
			}
		}
	}
}

unsigned leaf_data::nelems() const {
	unsigned res = 0;
	for ( iterator first = begin(), past =end(); first != past; ++first ) {
		++res;
		*first;
	}
	return res;
}


unsigned leaf_data::next_byte_size() const {
	return 2 * ( capacity() + data_offset );
}

void leaf_data::grow() {
	set_capacity( ( next_byte_size() - data_offset ) );
	memset( my_base() + usedbytes(), 0, capacity() - usedbytes() );
}

void leaf_data::construct( void* m ) {
	unsigned s = leaf_data::start_bytes();
	memset( m, 0, s );
	byte_io::write<uint16_t>( reinterpret_cast<unsigned char*>( m ), ( s - data_offset ) );
}

void leaf_data::print( std::ostream& out ) const {
	//out << format( "\tsize: %8s\n" ) % used();
	out << format( "\tcapacity: %8s\n" ) % capacity();
	//out << format( "\tnext: %8s\n" ) % next();
	int i = 0;
	for ( iterator first = begin(), past = end(); first != past; ++first ) {
		out << format( "\tref[ %1% ] = %2%\n" ) % i++ % *first;
	}
}

