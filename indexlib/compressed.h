#ifndef LPC_COMPRESSED_H1108569807_INCLUDE_GUARD_
#define LPC_COMPRESSED_H1108569807_INCLUDE_GUARD_

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

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "memvector.h"
#include "manager.h"
#include "mempool.h"
#include "thing.h"
#include "pointer.h"
#include "format.h"

void set_compression_manager( memory_manager* );
memory_manager* get_comp_p();

START_THING( compressed_page, thing<thing_manager<get_comp_p> > ) 
	public: // private:
		MEMBER( uint8_t, capacity, 0 )
		enum { header_size = 1 };
		MY_BASE( header_size )
	public:
		static void init( pointer<compressed_page> p ) {
			p->set_capacity( 4 );
		}
		void * data() { return my_base(); }
		const void * data() const { return my_base(); }
		void grow_to_size( const unsigned size ) {
			unsigned cur = ( 1 << capacity() ) - header_size;
			if ( size <= cur ) return;
			set_capacity( capacity() + 1 );
			grow_to_size( size );
		}
		void print( std::ostream& out ) {
			out << format( "\tsized %s\n" ) % ( 1 << capacity() );
		}
END_THING( compressed_page )
DO_POINTER_SPECS( compressed_page )

struct compressed_page_traits {
	typedef compressed_page value_type;
	typedef compressed_pageptr pointer;

	static bool is_free( pointer p ) { return p->capacity() == 0; }
	static void mark_free( pointer p ) { p->set_capacity( 0 ); }
	static unsigned size_of( pointer p ) { return 1 << p->capacity(); }

	static unsigned type_offset() { return compressed_page::header_size; }

	static unsigned min_size() { return 16; } // zlib needs minimum 12 bytes
	static unsigned max_size() { return 1024 * 4; }
	
	static void set_manager( memory_manager* m ) { set_compression_manager( m ); }
	static void print( std::ostream& out, pointer p ) { out << '[' << p << "] compressed_page:\n"; p->print( out ); }
};

class compressed_file : public memory_manager {
		struct page;
		friend struct page;
	public:
		compressed_file( std::string );
		~compressed_file();
		const unsigned char* ronly_base( unsigned idx ) const;
		unsigned char* rw_base( unsigned idx ) const;
		unsigned size() const;
		void resize( unsigned );
		void print( std::ostream& ) const;
		static void remove( std::string base );
	private:

		void write_back();
		void zlibchecktrue( bool ) const;
		void zlibcheck( int err, int expected = 0 ) const;

		enum { page_bits = 12, page_bytes = 1 << page_bits, compress_buffer_size = page_bytes + 12 + page_bytes/50 };
		static unsigned pageidx( unsigned idx ) { return idx >> page_bits; }
		static unsigned inpageidx( unsigned idx ) { return idx & ( ( 1 << page_bits ) - 1 ); }
		struct page {
			explicit page( bool d = false ):
				dirty_( d ) {
				}
			compressed_pageptr origin_;
			bool dirty_;
			unsigned char data_[ compressed_file::page_bytes ];
		};
		mutable std::vector<page*> pages_;
		mutable memvector<uint32_t> auxdata_;
		memory_reference<uint32_t> compressed_data_for( unsigned idx ) const {
			return auxdata_[ idx + 1 ];
		}
		mempool<compressed_page_traits> data_;
};



#endif /* LPC_COMPRESSED_H1108569807_INCLUDE_GUARD_ */
