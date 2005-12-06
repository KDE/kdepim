
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

#include "leafdatavector.h"
#include "mmap_manager.h"
#include "compressed.h"
#include "logfile.h"
#include "path.h"

#include "format.h"
#include <unistd.h>

#ifdef USE_ZLIB_COMPRESSION
typedef compressed_file leafdatavector_manager;
#else
typedef mmap_manager leafdatavector_manager;
#endif

leafdatavector::leafdatavector( std::string name ):
	leafs_( std::auto_ptr<memory_manager>( new leafdatavector_manager( path_concat( name, "leafs" ) ) ) ),
	table_( path_concat( name, "table" ) )
{
}

void leafdatavector::remove( std::string name ) {
	leafdatavector_manager::remove( path_concat( name, "leafs" ) );
	memvector<leaf_dataptr>::remove( path_concat( name, "table" ) );
}

void leafdatavector::add( unsigned idx, unsigned what ) {
	//logfile() << format( "leafdatavector::add( %s, %s )\n" ) % idx % what;
	table_.resize( idx + 1 );
	int32_t now = table_[ idx ];
	if ( !now ) {
		++what;
		table_[ idx ] = -int( what );
	} else if ( now < 0 ) {
		leafdataptr just = leafs_.allocate( leaf_data_pool_traits::min_size() );
		leafdata::construct( just.raw_pointer() );
		table_[ idx ] = just.cast_to_uint32();
		just->add_reference( -now - 1 );
		assert( just->can_add( what ) );
		just->add_reference( what );
	} else {
		leafdataptr just = leafdataptr::cast_from_uint32( now );
		if ( !just->can_add( what ) ) {
			just = leafs_.reallocate( just, just->next_byte_size() );
			just->grow();
			table_[ idx ] = just.cast_to_uint32();
		}
		just->add_reference( what );
	}
}

void leafdatavector::remove_references_to( unsigned ref ) {
	//logfile() << format( "%s( %s )\n" ) % __PRETTY_FUNCTION__ % ref;
	for ( unsigned idx = 0; idx != table_.size(); ++idx ) {
		int32_t now = table_[ idx ];
		if ( now == -int( ref ) ) table_[ idx ] = 0;
		else if ( now > 0 ) leafdataptr::cast_from_uint32( now )->remove_reference( ref );
	}
}

std::vector<unsigned> leafdatavector::get( unsigned idx ) const {
	if ( idx >= table_.size() ) return std::vector<unsigned>();
	int32_t now = table_[ idx ];
	if ( now < 0 ) {
		std::vector<unsigned> res;
		res.push_back( -now - 1 );
		return res;
	} else if ( now > 0 ) {
		logfile() << format( "%s( %s ) in %s\n" ) % __PRETTY_FUNCTION__ % idx % now;
		leafdataptr just = leafdataptr::cast_from_uint32( now );
		return std::vector<unsigned>( just->begin(), just->end() );
	} else {
		return std::vector<unsigned>();
	}
}


