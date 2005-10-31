#ifndef LPC_POINTER_H1103643194_INCLUDE_GUARD_
#define LPC_POINTER_H1103643194_INCLUDE_GUARD_

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
#include <iostream>
#include "boost-compat/static_assert.hpp"
#ifdef HAVE_BOOST
#include <boost/type_traits.hpp>
#endif

/**
 * \class pointer
 *
 * Works with \ref thing in providing disk translucency (not transparency, but half-way there).
 *
 * This is a pointer like object which is really an index into a memory block which must be deferenced
 * every time it is accessed. The main advantage of this is that it allows the pointer to remain valid even
 * if the base memory block changes place.
 *
 * Actually, implementation-wise, thing does all this already and this is mainly an adapter.
 */
template <typename Thing >
struct pointer: private Thing {
	private:
		typedef Thing base_type;

		pointer( uint32_t i )
			:base_type( i )
			{
			}
	public:
		pointer()
			:base_type( 0 )
			{
			}
		typedef Thing value_type;
		template <typename D>
			pointer( const pointer<D>& other )
				:base_type( other.cast_to_uint32() )
				 {
					 typedef D derived_type;
					 BOOST_STATIC_ASSERT( (boost::is_convertible<derived_type, value_type>::value ) );
				 }
		uint32_t cast_to_uint32() const {
			return this->idx_;
		}
		static pointer cast_from_uint32( uint32_t d ) {
			return d;
		}
		value_type& operator* () const {
			return const_cast<value_type&>( static_cast<const value_type&>( *this ) );
		}
		value_type* operator -> () const {
			return const_cast<value_type*>( static_cast<const value_type*>( this ) );
		}
		bool operator!() const {
			return !this->idx_;
		}

		operator const volatile void*() const {
			return this->idx_ ? this : 0;
		}
		void* raw_pointer() { return base_type::base(); }
	private:
};



template <typename T>
std::ostream& operator << ( std::ostream& out, const pointer<T>& p ) {
	return out << p.cast_to_uint32();
}



#endif /* LPC_POINTER_H1103643194_INCLUDE_GUARD_ */
