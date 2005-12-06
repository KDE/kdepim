
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

#include "stringarray.h"
#include "mmap_manager.h"
#include "bitio.h"
#include "logfile.h"
#include "path.h"
#include "format.h"

stringarray::stringarray( std::string filebase ):
	data_( new mmap_manager( path_concat( filebase, "string-data" ) ) ),
	indeces_( path_concat( filebase, "string-indeces" ) )
{
	if ( !data_->size() ) init_structure();
}

void stringarray::remove( std::string filebase ) {
	unlink( path_concat( filebase, "string-data" ).c_str() );
	unlink( path_concat( filebase, "string-indeces" ).c_str() );
}

stringarray::~stringarray() {
}

stringarray::index_type stringarray::add( std::string str ) {
	const unsigned count = indeces_.size();
	const index_type res = count;
	const index_type previous = count ? indeces_[ count - 1 ] : 0;
	const unsigned starti = count ? ( previous + get( count - 1 ).size() + 1 ) : 0;

	logfile() << format( "add( %s ) at starti = %d, with count = %d\n" ) % str % starti % count;

	if ( ( starti + str.size() + 1 ) > data_->size() ) {
		data_->resize( data_->size() + str.size() + 1 );
	}
	
	strcpy( reinterpret_cast<char*>( data_->rw_base( starti ) ), str.c_str() );
	indeces_.push_back( starti );
	return res;
}

void stringarray::erase( index_type idx ) {
	assert( idx < size() );
	char* target = const_cast<char*>( get_cstr( idx ) );
	if ( idx != size() - 1 ) {
		const char* next = get_cstr( idx + 1 );
		unsigned delta = strlen( target ) + 1;
		std::memmove( target, next, data_->size() - indeces_[ idx + 1 ] );
		// Hack: Don't compare the iterators directly, it ices gcc-2.95
		for ( memvector<uint32_t>::iterator first = indeces_.begin() + idx, past = indeces_.end(); first.raw() != past.raw(); ++first ) {
			*first -= delta;
		}
	}
	indeces_.erase( indeces_.begin() + idx );
}

void stringarray::clear() {
	data_->resize( 0 );
	indeces_.clear();

}

const char* stringarray::get_cstr( index_type idx ) const {
	uint32_t didx = indeces_[ idx ];
	//logfile() << format( "stringarray::get( %s ): %s\n" ) % idx
	//	% std::string( reinterpret_cast<char*>( data_->base() ) + didx );
	return reinterpret_cast<const char*>( data_->ronly_base( didx ) );
}


void stringarray::init_structure() {
}

void stringarray::print( std::ostream& out ) const {
	for ( unsigned i = 0; i != indeces_.size(); ++i ) {
		out << format( "string[ %s ] = %s\n" ) % i % get_cstr( i );
	}
}

void stringarray::verify() const {
	for ( unsigned i = 1; i < indeces_.size(); ++i ) {
		assert( !*( data_->ronly_base( indeces_[ i ] - 1 ) ) );
	}
}

