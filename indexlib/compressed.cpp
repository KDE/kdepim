
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

#include "compressed.h"
#include "logfile.h"
#include "bitio.h"
#include "path.h"
#include "format.h"
#include <zlib.h>
#include <cassert>
#include <cstring>
#include <iostream>

namespace {
	memory_manager* cmanager = 0;
}

void set_compression_manager( memory_manager* m ) {
	cmanager = m;
}
memory_manager* get_comp_p() {
	return cmanager;
}


compressed_file::compressed_file( std::string base ):
	auxdata_( path_concat( base, "table" ) ),
	data_( std::auto_ptr<memory_manager>( new mmap_manager( path_concat( base, "data" ) ) ) )
{
	if ( auxdata_.empty() ) auxdata_.push_back( 0 );
}

compressed_file::~compressed_file() {
	write_back();
}

const unsigned char* compressed_file::ronly_base( const unsigned idx ) const {
	logfile() << format( "%s ( addr: %s ) page: %s \n" ) % __PRETTY_FUNCTION__ % idx % pageidx( idx );
	assert( idx < size() );
	if ( pages_.size() <= pageidx( idx ) ) pages_.resize( pageidx( idx ) + 1 );
	if ( !pages_[ pageidx( idx ) ] ) {
		logfile() << format( "%s allocating a page %s\n" ) % __PRETTY_FUNCTION__ % pageidx( idx );
		page* p =  new page( true );
		pages_[ pageidx( idx ) ] = p;
		z_stream stream;
		stream.zalloc = 0;
		stream.zfree = 0;
		stream.opaque = 0;
		zlibcheck( inflateInit( &stream ) );
		p->origin_ = compressed_pageptr::cast_from_uint32( compressed_data_for( pageidx( idx ) ) );
		unsigned char* source = reinterpret_cast<unsigned char*>( p->origin_->data() );
		stream.next_in = source;
		stream.avail_in = ( 1 << p->origin_->capacity() ) - compressed_page::header_size;
		stream.next_out = p->data_;
		stream.avail_out = page_bytes + 1;
		zlibcheck( inflate( &stream, Z_FINISH ), Z_STREAM_END );
		//assert( !stream.avail_out );
		zlibcheck( inflateEnd( &stream ) );
	}
	return & ( pages_[ pageidx( idx ) ]->data_[ inpageidx( idx ) ] );
}

void compressed_file::print( std::ostream& out ) const {
	data_.print( out );
}

void compressed_file::remove( std::string base ) {
	memvector<uint32_t>::remove( path_concat( base, "table" ) );
	mmap_manager::remove( path_concat( base, "data" ) );
}


unsigned compressed_file::size() const {
	return auxdata_[ 0 ];
}

void compressed_file::resize( const unsigned n_s ) {
	logfile() << format( "%s ( %s )\n" ) % __PRETTY_FUNCTION__ % n_s;
	if ( n_s <= size() ) return;
	unsigned curpages = size() >> page_bits;
	assert( !( size() % page_bytes ) );
	const unsigned targetpages = ( n_s >> page_bits ) + bool( n_s % page_bytes );
	auxdata_[ 0 ] = targetpages << page_bits;
	logfile() << format( "Size set to %s (pages = %s, n_s = %s ) \n" ) % size() % targetpages % n_s;
	auxdata_.resize( targetpages + 1 ); // 1 for the auxdata_[0]

	static unsigned char empty[ page_bytes ] = { 0 };
	unsigned char empty_compress[ compress_buffer_size ];
	uLongf size = compress_buffer_size;
	zlibcheck( compress( empty_compress, &size, empty, page_bytes ) );
	while ( curpages < targetpages ) {
		compressed_pageptr p = data_.allocate( size + compressed_page::header_size );
		compressed_page::init( p );
		p->grow_to_size( size );
		std::memcpy( p->data(), empty_compress, size );
		compressed_data_for ( curpages ) = p.cast_to_uint32();
		++curpages;
	}
}

unsigned char* compressed_file::rw_base( unsigned idx ) const {
	const unsigned char* res = ronly_base( idx );
	pages_[ pageidx( idx ) ]->dirty_ = true;
	logfile() << format( "rw_base( %s ), pageidx=%s returning %s\n" ) % idx % pageidx( idx ) % ( void* )res;
	return const_cast<unsigned char*>( res );
}

void compressed_file::write_back() {
	logfile() <<format( "%s\n" ) % __PRETTY_FUNCTION__;
	for ( unsigned pagei = 0, size = pages_.size(); pagei != size; ++ pagei ) {
		page* p = pages_[ pagei ];
		if ( p && p->dirty_ ) {
			unsigned char buffer[ compress_buffer_size ];
			uLongf size = compress_buffer_size;
			zlibcheck( compress( buffer, &size, p->data_, page_bytes ) );
			{

				unsigned char buffer2[ 4096 ] = {0};
				z_stream stream;
				stream.zalloc = 0;
				stream.zfree = 0;
				stream.opaque = 0;
				zlibcheck( inflateInit( &stream ) );
				stream.next_in = buffer;
				stream.avail_in = size;
				stream.next_out = buffer2;
				stream.avail_out = page_bytes + 1;
				zlibcheck( inflate( &stream, Z_FINISH ), Z_STREAM_END );
				//assert( !stream.avail_out );
				zlibcheck( inflateEnd( &stream ) );

				assert( !memcmp( buffer2, p->data_, 4096 ) );
			}

			logfile() << format( "Compressed page %s to size %s\n" ) % pagei % size;
			unsigned original = ( 1 << p->origin_->capacity() ) - compressed_page::header_size;
			if ( size > original ) {
				p->origin_ = data_.reallocate( p->origin_, size + compressed_page::header_size );
				p->origin_->grow_to_size( size );
			}
			std::memcpy( p->origin_->data(), buffer, size );
			compressed_data_for( pagei ) = p->origin_.cast_to_uint32();
		} else {
			logfile() << format( "write_back() not saving %s\n" ) % pagei;
		}
	}
}


void compressed_file::zlibcheck( int err, int expected ) const {
	if ( err != expected ) {
		std::cerr << "zlib reports an error: " << err << std::endl;
		// throw compressed_file::zlib_exception( err );
		abort();
	}
}

