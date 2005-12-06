
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

#include "stringset.h"
#include "path.h"
#include <cstring>
using std::strcmp;

stringset::stringset( std::string name ):
	strings_( path_concat( name, "strings-set" ) ),
	ordered_( path_concat( name, "ordered-set" ) ),
	trie_( path_concat( name, "trie" ) )
{
	if ( trie_.empty() ) {
		trie_.resize( 256 );
		if ( !ordered_.empty() ) {
			unsigned char last = 0;
			for ( unsigned i = 0; i != ordered_.size(); ++i ) {
				unsigned char cur = static_cast<unsigned char>( strings_.get_cstr( ordered_[ i ] )[ 0 ] );
				if ( cur != last ) {
					trie_[ cur ] = i;
					last = cur;
				}
			}
			if ( last < 255 ) trie_[ last + 1 ] = ordered_.size();
		}
	}
}

void stringset::remove( std::string name ) {
	stringarray::remove( path_concat( name, "strings-set" ) );
	memvector<stringarray::index_type>::remove( path_concat( name, "ordered-set" ) );
	memvector<stringarray::index_type>::remove( path_concat( name, "trie" ) );
}

std::pair<stringset::const_iterator, stringset::const_iterator> stringset::upper_lower( const char* str ) const {
	const_iterator first = lower_bound( str );
	const_iterator second = lower_bound( ( std::string( str ) + char( 254 ) ).c_str() );
	return std::make_pair( first, second );
}

stringset::const_iterator stringset::lower_bound( const char* str ) const {
	const_iterator top = begin() + trie_[ ( unsigned )str[ 0 ] ];
	const_iterator bottom = begin() + trie_[ ( unsigned )str[ 0 ] + 1 ];
	while ( top < bottom ) {
		const_iterator middle = top + ( bottom - top ) / 2;
		int c = strcmp( *middle, str );
		if ( c == 0 ) return middle;
		if ( c > 0 ) bottom = middle;
		else top = middle + 1;
	}
	return top;
}

unsigned stringset::order_of( const char* str ) const {
	const_iterator where = lower_bound( str );
	return where == end() || strcmp( *where, str ) ? unsigned( -1 ) : where.idx_;
}


stringarray::index_type stringset::add( const char* str ) {
	const_iterator where = lower_bound( str );
	if ( where != end() && !strcmp( *where, str ) ) return where.id();
	stringarray::index_type res = strings_.add( str );
	ordered_.insert( ordered_.begin() + where.order(), res );
	assert( ordered_.size() == strings_.size() );
	for ( unsigned next = ( unsigned )str[ 0 ] + 1; next != 256; ++next ) {
		++trie_[ next ];
	}
	return res;
}

void stringset::clear() {
	strings_.clear();
	ordered_.clear();
}


