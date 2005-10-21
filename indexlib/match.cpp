
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

#include "match.h"
#include "format.h"
#include "compat.h"
#include <iostream>
#include <assert.h>

namespace {
	inline
	void setbit( unsigned& u, unsigned idx ) {
		u |= ( 1 << idx );
	}
	inline
	bool getbit( unsigned u, unsigned idx ) {
		return u & ( 1 << idx );
	}
}

indexlib::Match::Match( std::string str, unsigned flags ):
	masks_( 256 ),
	caseinsensitive_( flags & caseinsensitive ),
	pattern_rest_( str, kMin( str.size(), sizeof( unsigned ) * 8 - 1 ) )
{
	hot_bit_ = kMin( str.size(), sizeof( unsigned ) * 8 - 1 );
	for ( unsigned i = 0; i != hot_bit_; ++i ) {
		if ( caseinsensitive_ ) {
			setbit( masks_[ ( unsigned char )std::toupper( str[ i ] ) ], i );
			setbit( masks_[ ( unsigned char )std::tolower( str[ i ] ) ], i );
		} else {
			setbit( masks_[ ( unsigned char )str[ i ] ], i );
		}
	}
}

indexlib::Match::~Match() {
}

bool indexlib::Match::process( const char* string ) const {
	unsigned state = 0;
	while ( *string ) {
		state |= 1;
		state &= masks_[ ( unsigned char )*string ];
		state <<= 1;
		++string;
		if ( getbit( state, hot_bit_ ) && ( pattern_rest_ == std::string( string, pattern_rest_.size() ) ) ) return true;
	}
	return !hot_bit_;
}

