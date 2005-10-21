
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

#include "quotes.h"
#include "match.h"
#include "path.h"
#include "result.h"
#include "format.h"

quotes::quotes( std::string name ):
	impl_( path_concat( name, "base" ) ),
	docs_( path_concat( name, "docs" ) )
{
}

void quotes::remove( std::string name ) {
	ifile::remove( path_concat( name, "base" ) );
	stringarray::remove( path_concat( name, "docs" ) );
}

void quotes::add( const char * str, const char* doc ) {
	assert( str );
	assert( doc );
	try {
		impl_.add( str, doc );
		docs_.add( str );
	} catch ( const std::exception& e ) {
		std::cerr << "error in quotes::add: " << e.what() << "\n";
	}
}

void quotes::remove_doc( const char* doc ) {
	logfile() << format( "%s( %s )\n" ) % __PRETTY_FUNCTION__ % doc;
	for ( unsigned idx = 0; idx != docs_.size(); ++idx ) {
		if ( !strcmp( docs_.get_cstr( idx ), doc ) ) {
			docs_.erase( idx );
			break;
		}
	}
	impl_.remove_doc( doc );
}

std::auto_ptr<indexlib::result> quotes::search( const char* cstr ) const {
	std::string str = cstr;
	if ( str[ 0 ] != '\"' ) return impl_.search( cstr );
	str = cstr + 1; // cut "
	if ( str.size() && str[ str.size() - 1 ] == '\"' ) str.erase( str.size() - 1 );
	std::auto_ptr<indexlib::result> prev = impl_.search( str.c_str() );
	if ( str.find( ' ' ) != std::string::npos ) {
		indexlib::Match m( str );
		std::vector<unsigned> candidates = prev->list();
		std::vector<unsigned> res;
		res.reserve( candidates.size() );
		for ( std::vector<unsigned>::const_iterator first = candidates.begin(), past = candidates.end();
				first != past;
				++first ) {
			if ( m.process( docs_.get_cstr( *first ) ) ) {
				res.push_back( *first );
			}
		}
		return std::auto_ptr<indexlib::result>( new indexlib::detail::simple_result( res ) );
	} else { return prev; }
}

