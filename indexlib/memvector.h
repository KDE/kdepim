#ifndef LPC_MEMVECTOR_H1105049836_INCLUDE_GUARD_
#define LPC_MEMVECTOR_H1105049836_INCLUDE_GUARD_

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

#include "memreference.h"
#include "bitio.h"
#include "compat.h"
#include "manager.h"
#include "boost-compat/static_assert.hpp"
#include "boost-compat/scoped_ptr.hpp"
#ifdef HAVE_BOOST
#include <boost/type_traits/is_convertible.hpp>
#endif
#include <iostream>
#include <iterator>
#include <string>
#include <cstring>
#include <assert.h>

template <typename> class memory_iterator;
template <typename> class memvector;

template <typename T>
struct memory_iterator : public std::iterator<STD_NAMESPACE_PREFIX random_access_iterator_tag,T> {
	private:
	public:
		template <typename U>
		memory_iterator( const memory_iterator<U>& other ):
			data_( const_cast<unsigned char*>( other.raw() ) )
			{
				BOOST_STATIC_ASSERT( (boost::is_convertible<U, T>::value ) );
			}
		explicit memory_iterator( unsigned char* d ):
			data_( d )
			{
			}
		T operator* () const {
			return byte_io::read<T>( data_ );
		}
		memory_reference<T> operator* () {
			return memory_reference<T>( data_ );
		}

		memory_iterator& operator ++() {
			data_ += byte_io::byte_lenght<T>();
			return *this;
		}

		memory_iterator& operator --() {
			data_ -= byte_io::byte_lenght<T>();
			return *this;
		}

		memory_iterator& operator += ( ptrdiff_t dif ) {
			data_ += dif * byte_io::byte_lenght<T>();
			return *this;
		}

		ptrdiff_t operator - ( const memory_iterator<T>& other ) const {
			assert( !( ( raw() - other.raw() )%byte_io::byte_lenght<T>() ) );
			return ( raw() - other.raw() )/byte_io::byte_lenght<T>();
		}

		bool operator < ( const memory_iterator<T>& other ) const {
			return ( this->raw() - other.raw() ) < 0;
		}
		const unsigned char* raw() const { return data_; }
	private:
		unsigned char* data_;
};

template <typename T, typename U>
inline
bool operator == ( const memory_iterator<T>& a, const memory_iterator<U>& b ) {
	return a.raw() == b.raw();
}

template <typename T, typename U>
inline
bool operator != ( const memory_iterator<T>& a, const memory_iterator<U>& b ) {
	return !( a == b );
}

template <typename T, typename U>
inline
bool operator <= ( const memory_iterator<T>& a, const memory_iterator<U>& b ) {
	return !( b < a );
}



template <typename T>
inline
memory_iterator<T> operator + ( memory_iterator<T> iter, typename memory_iterator<T>::difference_type dif ) {
	iter += dif;
	return iter;
}

template <typename T>
inline
memory_iterator<T>& operator -= ( memory_iterator<T>& iter, typename memory_iterator<T>::difference_type dif ) {
	iter += -dif;
	return iter;
}

template <typename T>
inline
memory_iterator<T> operator - ( memory_iterator<T> iter, typename memory_iterator<T>::difference_type  dif ) {
	iter -= dif;
	return iter;
}


template <typename T>
inline
memory_iterator<T> operator -- ( memory_iterator<T>& ref, int ) {
	memory_iterator<T> copy = ref;
	--ref;
	return copy;
}

template<typename T>
inline
memory_iterator<T> operator ++ ( memory_iterator<T>& ref, int ) {
	memory_iterator<T> copy = ref;
	++ref;
	return copy;
}

/**
 * A vector of T kept on disk.
 *
 * The interface is a subset of std::vector<T>'s interface.
 */
template <typename T>
struct memvector {
	public:
		memvector( std::string );
		~memvector();

		typedef T value_type;
		typedef unsigned size_type;
		typedef memory_iterator<T> iterator;
		typedef memory_iterator<const T> const_iterator;

		iterator begin() { return iterator( address_of( 0 ) ); }
		iterator end() { return iterator( address_of( size() ) ); }

		const_iterator begin() const { return const_iterator( address_of( 0 ) ); }
		const_iterator end() const { return const_iterator( address_of( size() ) ); }

		value_type operator[] ( unsigned idx ) const {
			assert( idx < size() );
			return byte_io::read<T>( address_of( idx ) );
		}

		memory_reference<T> operator[] ( unsigned idx ) {
			assert( idx < size() );
			return memory_reference<T>( address_of( idx ) );
		}

		/**
		 * For debugging, nothing else
		 */
		void print( std::ostream& ) const;
		size_type size() const { return byte_io::read<uint32_t>( data_->ronly_base( 0 ) ); }
		bool empty() const { return !size(); }
		void resize( size_type );

		void insert( const_iterator, const value_type );
		void erase( iterator );
		void clear();
		void push_back( value_type v ) { insert( end(), v ); }

		/**
		 * Removes from disk
		 */
		static void remove( std::string );

	private:
		boost::scoped_ptr<memory_manager> data_;
		unsigned char* address_of( unsigned i ) {
			return 	data_->rw_base(
				byte_io::byte_lenght<unsigned>() +
				i * byte_io::byte_lenght<T>() );
		}
		const unsigned char* address_of( unsigned i ) const {
			return const_cast<memvector*>( this )->address_of( i );
		}
};

#include "memvector.tcc"


#endif /* LPC_MEMVECTOR_H1105049836_INCLUDE_GUARD_ */
