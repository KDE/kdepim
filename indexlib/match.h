#ifndef LPC_MATCH_H1105564052_INCLUDE_GUARD_
#define LPC_MATCH_H1105564052_INCLUDE_GUARD_

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
#include <map>

namespace indexlib {

/**
 * \brief A class for string matching
 *
 * This class represents a pattern in a pre-processed form so that searching for it in larger textes is
 * very fast. This allows for a very fast way to ask "does this pattern appear in this text?"
 *
 * The implementation uses the shift-or algorithm which is very fast, but especially designed for
 * patterns which are shorter than the number of bits in a word ( 32 bits on most architechtures ). For
 * larger patterns, the first 32 characters are matched using shift-or and the next characters are strcmp()ed.
 * Even for patterns larger than 32 characters, this should be a fast strategy.
 */
class Match {
	public:
		enum flags { caseinsensitive = 1 };
		/** Construct an object to match string \param pattern
		 */
		Match( std::string pattern, unsigned flags = 0 );
		~Match();

		/**
		 * Returns true if the pattern appears in \param string
		 * It has no memory
		 */
		bool process( const char* string ) const;
		bool process( std::string str ) const { return process( str.c_str() ); }
	private:
		typedef std::vector<unsigned> masks_type;
		masks_type masks_;
		unsigned hot_bit_;
		bool caseinsensitive_;
		std::string pattern_rest_;
};
}


#endif /* LPC_MATCH_H1105564052_INCLUDE_GUARD_ */
