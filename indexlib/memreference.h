#ifndef LPC_MEMREFERENCE_H1108569807_INCLUDE_GUARD_
#define LPC_MEMREFERENCE_H1108569807_INCLUDE_GUARD_

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


#include "bitio.h"

/**
 * Works like a reference to a memory location
 * which is written and read in our byte-format.
 */
template <typename T>
struct memory_reference {
	public:
		explicit memory_reference( unsigned char* d ):
			data_( d )
			{
			}
		~memory_reference() { }
		memory_reference& operator = ( const memory_reference& other ) {
			operator=( T( other ) );
			return *this;
		}
		memory_reference& operator = ( const T& value ) {
			byte_io::write<T>( data_, value );
			return *this;
		}
		operator T () const {
			return byte_io::read<T>( data_ );
		}
		memory_reference( const memory_reference& other ):
			data_( other.data_ )
			{
			}
		/**
		 * This is a sneaky method to change pointers
		 */
		void assign( const memory_reference& other ) {
			data_ = other.data_;
		}
	private:
		unsigned char* data_;
};


// A mixed and fairly random collection of helper functions

template <typename T, typename U>
memory_reference<T> operator += ( memory_reference<T> ref, U v ) {
	return ref = ref + v;
}

template <typename T>
memory_reference<T> operator ++( memory_reference<T> ref ) {
	return ref = ref + 1;
}

template <typename T>
memory_reference<T> operator --( memory_reference<T> ref ) {
	return ref = ref - 1;
}

template <typename T>
T operator ++( memory_reference<T> ref, int ) {
	T v = ref;
	ref = ref + 1;
	return v;
}

template <typename T>
T operator --( memory_reference<T> ref, int ) {
	T v = ref;
	ref = ref - 1;
	return v;
}

template <typename T, typename U>
memory_reference<T> operator -= ( memory_reference<T> ref, U v ) {
	return ref = ref - v;
}

template <typename T, typename U>
memory_reference<T> operator *= ( memory_reference<T> ref, U v ) {
	return ref = ref * v;
}



#endif /* LPC_MEMREFERENCE_H1108569807_INCLUDE_GUARD_ */
