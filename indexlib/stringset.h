#ifndef LPC_STRINGSET_H1106061353_INCLUDE_GUARD_
#define LPC_STRINGSET_H1106061353_INCLUDE_GUARD_

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


#include "memvector.h"
#include "stringarray.h"
#include <iterator>
#include <iostream>
#include <utility>


struct stringset {
	public:
		struct const_iterator : public std::iterator<STD_NAMESPACE_PREFIX random_access_iterator_tag,const char*> {
				const char* operator*() const {
					return mother_->strings_.get_cstr( mother_->ordered_[ idx_ ] );
				}
				unsigned id() const {
					return mother_->ordered_[ idx_ ];
				}
				unsigned order() const {
					return idx_;
				}
				const_iterator& operator ++() {
					++idx_;
					return *this;
				}
				const_iterator& operator --() {
					--idx_;
					return *this;
				}

				const_iterator& operator +=( ptrdiff_t d ) {
					idx_ += d;
					return *this;
				}
				const_iterator& operator -=( ptrdiff_t d ) {
					idx_ -= d;
					return *this;
				}
				const_iterator& operator = ( const const_iterator& other ) {
					mother_ = other.mother_;
					idx_ = other.idx_;
					return *this;
				}
				ptrdiff_t operator - ( const const_iterator& other ) const {
						assert( mother_ == other.mother_ );
						return idx_ - other.idx_;
				}
				bool operator < ( const const_iterator& other ) const {
						assert( mother_ == other.mother_ );
						return idx_ < other.idx_;
				}

				bool operator == ( const const_iterator& other ) const {
						return mother_ == other.mother_ && idx_ == other.idx_;
				}

				bool operator != ( const const_iterator& other ) const {
						return !( *this == other );
				}
				const_iterator():
					mother_( 0 ),
					idx_( 0 )
					{
					}
				const_iterator( const const_iterator& other ):
					mother_( other.mother_ ),
					idx_( other.idx_ )
					{
					}
			private:
				friend std::ostream& operator << ( std::ostream& out, const const_iterator& other );
				friend class stringset;
				const_iterator( const stringset* m, unsigned i ):
						mother_( m ),
						idx_( i )
						{
						}

				const stringset* mother_;
				unsigned idx_;
				
		};
		friend class const_iterator;
	public:
		stringset( std::string );
		bool count( const char* str ) const { return order_of( str ) != unsigned( -1 ); }
		unsigned id_of( const char* str ) const {
			unsigned order = order_of( str );
			return order == unsigned( -1 ) ?
			       unsigned( -1 ):
			       ordered_[ order ];
		}
		unsigned order_of( const char* ) const;
		unsigned add( std::string str ) { return add( str.c_str() ); }
		unsigned add( const char* );

		void clear();

		/**
		 * Returns std::make_pair( find( word ), find( word + 'Z' ) ) which makes it easy
		 * to implement word* searches
		 */
		std::pair<const_iterator, const_iterator> upper_lower( const char* ) const;

		const_iterator begin() const { return const_iterator( this, 0 ); }
		const_iterator end() const { return const_iterator( this, ordered_.size() ); }
		bool empty() const { return !ordered_.size(); }
		unsigned size() const { return ordered_.size(); }

		static void remove( std::string );
		const_iterator lower_bound( const char* ) const;
	private:
		stringarray strings_;
		memvector<stringarray::index_type> ordered_;
		memvector<stringarray::index_type> trie_;
};

inline
stringset::const_iterator operator + ( stringset::const_iterator a, stringset::const_iterator::difference_type d ) {
				return a += d;
}

inline
std::ostream& operator << ( std::ostream& out, const stringset::const_iterator& other ) {
				return out << "[ " << other.idx_ << " ]";
}


#endif /* LPC_STRINGSET_H1106061353_INCLUDE_GUARD_ */
