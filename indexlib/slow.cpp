
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

#include "slow.h"
#include "match.h"
#include "path.h"

using std::string;

slow::slow( string str ):
	strings_( path_concat( str, "strings" ) ),
	docs_( path_concat( str, "docs" ) )
{
}

void slow::remove( string name )
{
	stringarray::remove( path_concat( name, "strings" ) );
	stringarray::remove( path_concat( name, "docs" ) );
}

void slow::add( string str, string doc ) {
	docs_.add( doc );
	strings_.add( str );
}

std::vector<unsigned> slow::search( string str ) const {
	std::vector<unsigned> res;
	indexlib::Match m( str );

	for ( unsigned i = 0; i != strings_.size(); ++i ) {
		if ( m.process( strings_.get_cstr( i ) ) ) res.push_back( i );
	}
	return res;
}

