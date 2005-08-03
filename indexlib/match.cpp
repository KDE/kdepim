
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
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston,
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
#include <iostream>
#include <assert.h>
#include "format.h"

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

Match::Match( std::string str, unsigned flags ):
	masks_( 256 ),
	caseinsensitive_( flags & caseinsensitive ){
	assert( str.size() <= sizeof( unsigned ) * 8 );
	pat_len_ = str.size();
	for ( unsigned i = 0; i != pat_len_; ++i ) {
		if ( caseinsensitive_ ) {
			setbit( masks_[ ( unsigned char )std::toupper( str[ i ] ) ], i );
			setbit( masks_[ ( unsigned char )std::tolower( str[ i ] ) ], i );
		} else {
			setbit( masks_[ ( unsigned char )str[ i ] ], i );
		}
	}
}

Match::~Match() {
}

bool Match::process( const char* string ) const {
	unsigned state = 0;
	while ( *string ) {
		state |= 1;
		state &= masks_[ ( unsigned char )*string ];
		state <<= 1;
		if ( getbit( state, pat_len_ ) ) return true;
		++string;
	}
	return !pat_len_;
}

