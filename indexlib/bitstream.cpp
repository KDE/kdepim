
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

#include "bitstream.h"
#include "logfile.h"
#include "format.h"

#include <assert.h>

bitstream::bitstream( unsigned char* data, unsigned size )
	:bytes_( data ),
	 writeable_( true ),
	 size_( size ),
	 cur_( 0 ) { }

bitstream::bitstream( const unsigned char* data, unsigned size )
	:bytes_( const_cast<unsigned char*>( data ) ),
	 writeable_( false ),
	 size_( size ),
	 cur_( 0 ) { }

bool bit( unsigned v, unsigned idx ) {
	return ( v >> idx ) & 1;
}

bitstream& bitstream::operator << ( uint16_t x ) {
	for ( unsigned i = 0; i != 16; ++i ) putbit( bit( x, i ) );
	return *this;
}

bitstream& bitstream::operator << ( uint32_t x ) {
	for ( unsigned i = 0; i != 32; ++i ) putbit( bit( x, i ) );
	return *this;
}

bitstream& bitstream::operator >> ( uint16_t& v ) {
	v = 0;
	for ( int i = 0; i != 16; ++i ) {
		v <<= 1;
		v |= getbit();
	}
	return *this;
}

bitstream& bitstream::operator >> ( uint32_t& v ) {
	v = 0;
	for ( int i = 0; i != 32; ++i ) {
		v <<= 1;
		v |= getbit();
	}
	return *this;
}

void bitstream::putback( uint16_t ) {
	assert( cur_ >= 16 );
	cur_ -= 16;
}

void bitstream::putback( uint32_t ) {
	assert( cur_ >= 32 );
	cur_ -= 32;
}

bool bitstream::getbit() {
	unsigned inbyte = cur_ % 8;
	unsigned byte = cur_ / 8;
	++cur_;
	return ( bytes_[ byte ] >> inbyte ) & 1;
}

void bitstream::putbit( bool value ) {
	unsigned inbyte = cur_ % 8;
	unsigned byte = cur_ / 8;
	assert( byte < size_ );
	if ( value ) bytes_[ byte ] |= ( 1 << inbyte );
	else bytes_[ byte ] &= ~( 1 << inbyte );
	++cur_;
}


