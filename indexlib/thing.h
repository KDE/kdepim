#ifndef LPC_THING_H1103643194_INCLUDE_GUARD_
#define LPC_THING_H1103643194_INCLUDE_GUARD_

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


#include <inttypes.h>
#include "bitio.h"
#include "pointer.h"

/**
 * \class thing
 *
 * This is perhaps one of the most important classes in the system.
 *
 * Ideally one would like to have something like:
 *
 *     struct_on_disk X {
 *                int32_t a;
 *                int32_t b;
 *     };
 *
 *     X var;
 *
 * And var would be accessed in our exact format. Since I want to control the exact format
 * to be able to use the same index even in different architechtures, it's not really possible.
 *
 * So we would do:
 *
 *     START_THING( X, simple_accessor )
 *		MEMBER( int32_t, a, 0 )
 *		MEMBER( int32_t, b, 4 )
 *     END_THING( X )
 *
 * This base class provides the machinery for this.
 */
template <typename accessor>
struct thing : protected accessor { // this allow the emtpy base optimization
	protected:
		thing( uint32_t idx, const accessor& access = accessor() ):
			accessor( access ),
			idx_( idx )
			{
			}

		unsigned char* base() {
			return reinterpret_cast<unsigned char*>( accessor::rw_base( idx_ ) );
		}
		const unsigned char* base() const {
			return reinterpret_cast<const unsigned char*>( accessor::ronly_base( idx_ ) );
		}
	public:
		~thing() { }
		thing( const thing& other ):
			accessor( static_cast<const accessor&>( other ) ),
			idx_( other.idx_ )
			{
			}

		thing& operator = ( const thing& other ) {
			accessor::operator=( other );
			idx_ = other.idx_;
			return *this;
		}
	protected:
		uint32_t idx_;
};

template <void * ( *get_base )()>
struct simple_accessor {
	public:
		void* rw_base( unsigned idx ) const {
			return reinterpret_cast<unsigned char*>( get_base() ) + idx;
		}
		const void* ronly_base( unsigned idx ) const {
			return reinterpret_cast<const unsigned char*>( get_base() ) + idx;
		}
};


#define START_THING( name, base ) \
	class name  : public base { \
		friend class pointer<name>; \
		protected: \
			name ( const base& b ) \
				:base( b ) \
				{ \
				} \
			\
			name ( uint32_t i ) \
				:base( i ) \
				{ \
				} \
		public:

#define MEMBER( type, name, idx ) \
		type name() const { \
			const unsigned char* data = this->base() + idx; \
			return byte_io::read<type>( data ); \
		} \
		\
		void set_ ## name ( const type & n_ ## name ) { \
			unsigned char* data = this->base() + idx; \
			byte_io::write<type>( data, n_ ## name ); \
		}

#define MY_BASE( idx ) \
		private: \
		unsigned char* my_base() { return base() + idx; } \
		const unsigned char* my_base() const { return base() + idx; } \


#define END_THING( name ) \
	}; \
	\
	typedef ::pointer< name > name ## ptr;

#define DO_POINTER_SPECS( name ) \
	namespace byte_io { \
	template<> \
	inline \
	pointer<name> read< pointer<name> >( const unsigned char* in ) \
	{ \
		return pointer< name >::cast_from_uint32( read<uint32_t>( in ) ); \
	}\
	template<> \
	inline \
	void write< pointer<name> >( unsigned char* out, pointer<name> p ) { \
		write<uint32_t>( out, p.cast_to_uint32() ); \
	} \
	template<> \
	struct byte_lenght_struct< pointer <name> > { \
		static const unsigned value = byte_lenght_struct<uint32_t>::value; \
	}; \
	} // namespace




#endif /* LPC_THING_H1103643194_INCLUDE_GUARD_ */
