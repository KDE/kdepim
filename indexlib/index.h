#ifndef LPC_INDEX_H1105549284_INCLUDE_GUARD_
#define LPC_INDEX_H1105549284_INCLUDE_GUARD_

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

#include <vector>
#include <string>
#include <memory>

namespace indexlib {
/**
 * Represents a result set
 *
 * The reason we do not return directly the answer set is twofold ( though connected )
 *
 * - We may decide to partially compute the set in the result list() method
 * - The possibility of implementing result_type::search to more efficiently handle repeated searches
 */
struct result {
	public:
		virtual ~result() { }
		/**
		 * The answer set
		 */
		virtual std::vector<unsigned> list() const = 0;
		/**
		 * A new search.
		 *
		 * The reason for this function is that if the user is typing and we do repeated searches for
		 * "s", "st", "str", "stri" ... "string"  we may (depending on the particular implementation)
		 * reuse the previous results for a speedup.
		 *
		 * @return null if the type does not support this or the particular search string makes it impossible to
		 * 		fulfill the search request.
		 */
		virtual std::auto_ptr<result> search( const char* ) = 0;
};

struct index {
	public:
		virtual ~index() { }
		/**
		 * Adds the document \param str under the name \param docname
		 */
		virtual void add( const char* str, const char* docname ) = 0;
		/**
		 * \see add( const char*, const char* );
		 */
		virtual void add( std::string s, std::string d ) { add( s.c_str(), d.c_str() ); }

		/**
		 * Removes the doc known as docname.
		 *
		 * In most implementations of index, this will probably be implemented by marking
		 * the document as deleted in some way and maintenance() will need to be called later.
		 */
		virtual void remove_doc( const char* doc ) = 0;
		void remove_doc( std::string doc ) { remove_doc( doc.c_str() ); }

		/**
		 * Perform any maintenance tasks necessary.
		 *
		 * Should be called every so often, but it can take some time to perform these operations,
		 * so call them when the app is idle.
		 */
		virtual void maintenance() { }

		/**
		 * Returns all documents matching \param pattern.
		 */
		virtual std::auto_ptr<result> search( const char* pattern ) const = 0;

		/**
		 * Returns the number of docs indexed.
		 */
		virtual unsigned ndocs() const = 0;
		/**
		 * Since docs are returned by index, there names need to be looked up.
		 */
		virtual std::string lookup_docname( unsigned ) const = 0;
	private:
};
}

#endif /* LPC_INDEX_H1105549284_INCLUDE_GUARD_ */

