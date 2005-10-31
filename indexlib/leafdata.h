#ifndef LPC_LEAFDATA_H1102530057_INCLUDE_GUARD_
#define LPC_LEAFDATA_H1102530057_INCLUDE_GUARD_

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


#include <iostream>
#include <iterator>
#include <inttypes.h>
#include "pointer.h"
#include "memvector.h"
#include "thing.h"

void  set_leafdata_manager( memory_manager* );
memory_manager* get_leafdata_manager();

struct leafdata_iterator : public std::iterator<STD_NAMESPACE_PREFIX input_iterator_tag,uint32_t> {
	public:
		explicit leafdata_iterator( const unsigned char* d ):
			data_( d ),
			value_( 0 ),
			valid_( true )
			{
			}

		value_type operator *() {
			assert( valid_ );
			valid_ = false;

			uint32_t delta = *data_++;
			if ( delta ) {
				value_ += delta;
			} else {
				value_ = byte_io::read<uint32_t>( data_ );
				data_ += byte_io::byte_lenght<uint32_t>();
			}
			return value_ - 1;
		}

		const unsigned char* raw() const { return data_; }

		leafdata_iterator& operator ++ () {
			valid_ = true;
			return *this;
		}
		bool operator == ( const leafdata_iterator& other ) const {
			return data_ == other.data_;
		}
		bool operator < ( const leafdata_iterator& other ) const {
			return data_ < other.data_;
		}
	private:
		const unsigned char* data_;
		uint32_t value_;
		bool valid_;
};

inline
bool operator != ( const leafdata_iterator& one, const leafdata_iterator& other ) {
	return !( one == other );
}

START_THING( leaf_data, thing< thing_manager<&get_leafdata_manager> > )
	private:
		MEMBER( uint16_t, capacity, 0 )
		MEMBER( uint16_t, usedbytes, 2 )
		friend class leaf_data_pool_traits;
		static const unsigned data_offset = 4;
	public:
		MY_BASE( data_offset )
	public:

	typedef leafdata_iterator iterator;
	leafdata_iterator begin() const { return iterator( const_cast<unsigned char*>( my_base() ) ); }
	leafdata_iterator end() const { return iterator( const_cast<unsigned char*>( my_base() ) + usedbytes() ); }

	uint32_t get_reference( unsigned idx ) const;
	void add_reference( uint32_t );
	void remove_reference( uint32_t );
	
	bool has_reference( uint32_t ) const;
	bool can_add( uint32_t ) const;

	unsigned nelems() const;
	unsigned next_byte_size() const;
	void grow();

	void print( std::ostream& out ) const;

	static void construct( void* );
	static void init( pointer<leaf_data> p ) { construct( p.raw_pointer() ); }
	static unsigned start_bytes() { return 16; }
END_THING( leaf_data )

DO_POINTER_SPECS( leaf_data )


typedef leaf_data leafdata;
typedef leaf_dataptr leafdataptr;

struct leaf_data_pool_traits {
	typedef leaf_data value_type;
	typedef leaf_dataptr pointer;
	
	static bool is_free( pointer p ) { return p->capacity() == 0; }
	static void mark_free( pointer p ) { memset( p.raw_pointer(), 0, p->capacity() ); }
	static unsigned size_of( pointer p ) { return p->capacity() + leafdata::data_offset; }

	static unsigned type_offset() { return 2; }

	static unsigned min_size() { return leaf_data::start_bytes(); }
	static unsigned max_size() { return 1024 * 64; }
	
	static void set_manager( memory_manager* m ) { return set_leafdata_manager( m ); }
	static void print( std::ostream& out, pointer p ) { out << '[' << p << "] leafdata:\n"; p->print( out ); }
};


#endif /* LPC_LEAFDATA_H1102530057_INCLUDE_GUARD_ */
