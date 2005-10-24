#ifndef LPC_MEMPOOL_H1103129409_INCLUDE_GUARD_
#define LPC_MEMPOOL_H1103129409_INCLUDE_GUARD_

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
#include "manager.h"
#include "memreference.h"
#include "thing.h"
#include <cassert>
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>

/**
 * @short implement memory management for things
 *
 * This class implements memory management for pools of things.
 * It uses a simple linked list memory management algorithm and
 * it depends on being supplied the right trait for the type held.
 */
template <typename Traits>
struct mempool /* : boost::noncopyable */ {
	public:
		typedef Traits traits_type;
		typedef typename traits_type::value_type data_type;
		typedef typename traits_type::pointer data_typeptr;
		explicit mempool( std::auto_ptr<memory_manager> source );

		/**
		 * Returns a memory block of size \param s.
		 * Analogous to malloc()
		 */
		data_typeptr allocate( unsigned s );
		/**
		 * Makes the memory pointed to by \param p of size \s or allocates a new block of size \s
		 * and moves the held data.
		 *
		 * Analogous to realloc()
		 * Unlike realloc(), does not support either reallocate( 0, s ) nor reallocate( p, 0 )
		 */
		data_typeptr reallocate( data_typeptr, unsigned );
		/** Releases the memory pointed to by \param p
		  * Analogous to free()
		  */
		void deallocate( data_typeptr p );

		/** Prints a half-readable description to \param out
		 * Mainly for debugging 
		 */
		void print( std::ostream& out ) const;

		/**
		 * How big (in bytes) is the memory managed?
		 */
		unsigned size() const { return manager_->size(); }
	private:
		/**
		 * Basically int( log_2( min( x + 1, min_size() ) ) )
		 */
		static unsigned order_of( unsigned x ) {
			assert( x > 0 );
			if ( x < traits_type::min_size() ) x = traits_type::min_size();
			--x;
			unsigned res = 0;
			while ( x ) {
				++res;
				x >>= 1;
			}
			return res;
		}
		/**
		 * @returns order^2
		 */
		static unsigned order_to_size( unsigned order ) {
			return 1 << order;
		}
		static unsigned size_of( data_typeptr data ) {
			return traits_type::size_of( data );
		}

		enum { min_order_for_free_node = 4 };
		
		friend struct list_node_manager;
		struct list_node_manager {
			protected:
				const mempool* parent_;
			public:
				explicit list_node_manager( const mempool* p = 0 ):parent_( p ) {}

				void* rw_base( unsigned idx ) const {
					return parent_->manager_->rw_base( idx );
				}
				const void* ronly_base( unsigned idx ) const {
					return parent_->manager_->ronly_base( idx );
				}
		};

		START_THING( list_node, thing<list_node_manager> )
			void set_parent( const mempool* p ) { this->parent_ = p; }
			MEMBER( uint16_t, order, 0 )
			MEMBER( uint32_t, next, 2 )
			MEMBER( uint32_t, prev, 6 )
		END_THING( list_node )

		list_nodeptr get_node( uint32_t p ) const;
		/**
		 * Get the free list header for a given order
		 */
		memory_reference<uint32_t> free_list( unsigned order );
		uint32_t free_list( unsigned order ) const {
			return const_cast<mempool*>( this )->free_list( order );
		}
		void insert_into_list( uint32_t where, unsigned order );
		void remove_from_list( uint32_t where, unsigned order );
		void break_up( uint32_t where );
		void init_memory();
		void fill_into_list( unsigned old_size );
		void fill_into_list( unsigned old_size, unsigned order );

		bool join( data_typeptr&, unsigned order );
		void deallocate( data_typeptr, unsigned order );

		std::auto_ptr<memory_manager> manager_;
		memory_reference<uint32_t> max_order_;
};

#include "mempool.tcc"

#endif /* LPC_MEMPOOL_H1103129409_INCLUDE_GUARD_ */
