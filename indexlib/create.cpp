
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

#include "create.h"
#include "quotes.h"
#include "path.h"
#include "version.h"
#include <fstream>
#include <unistd.h>

namespace {

indexlib::index_type::type type_of( const char* basename ) {
	std::ifstream info( path_concat( basename, "info" ).c_str() );
	if ( !info ) return indexlib::index_type::none;
	std::string type;
	std::string marker;
	std::string ver;
	int major, minor;
	char sep;
	std::getline( info, marker );
	info >> ver >> major >> sep >> minor;
	info >> type;
	if ( !info ) return indexlib::index_type::none;
	if ( type == "quotes" ) return indexlib::index_type::quotes;
	if ( type == "ifile" ) return indexlib::index_type::ifile;
	return indexlib::index_type::none;
}
}

std::auto_ptr<indexlib::index> indexlib::create( const char* basename, indexlib::index_type::type flags ) {
	using namespace indexlib::version;
	if ( type_of( basename ) !=  indexlib::index_type::none ) return std::auto_ptr<indexlib::index>( 0 );
	try {
		if ( basename[ strlen( basename ) - 1 ] == '/' && !isdir( basename ) ) {
			if ( !indexlib::detail::mkdir_trailing( basename ) ) return std::auto_ptr<indexlib::index>( 0 );
		}
		std::ofstream info( path_concat( basename, "info" ).c_str() );
		info << marker << std::endl;
		info << "version " << major << '.' << minor << "\n";
		if ( flags == index_type::quotes ) {
			info << "quotes" << std::endl;
			return std::auto_ptr<indexlib::index>( new quotes( basename ) );
		}
		if ( flags == index_type::ifile ) {
			info << "ifile" << std::endl;
			return std::auto_ptr<indexlib::index>( new ifile( basename ) );
		}
	} catch ( const std::exception& e ) {
		std::cerr << "index creation failed: " << e.what() << std::endl;
	}
	return std::auto_ptr<indexlib::index>( 0 );
}

std::auto_ptr<indexlib::index> indexlib::open( const char* basename, unsigned flags ) {
	using namespace indexlib;
	switch ( type_of( basename ) ) {
		case index_type::ifile: return std::auto_ptr<indexlib::index>( new ifile( basename ) );
		case index_type::quotes: return std::auto_ptr<indexlib::index>( new quotes( basename ) );
		case index_type::none:
		if ( flags == open_flags::fail_if_nonexistant ) return std::auto_ptr<indexlib::index>();
		return create( basename, index_type::type( flags ) );
	}
	logfile() << format( "%s:%s: Unexpected code reached!\n" ) % __FILE__ % __LINE__;
	return std::auto_ptr<indexlib::index>( 0 );
}

bool indexlib::exists( const char* basename ) {
	return basename && ( type_of( basename ) == indexlib::index_type::none );
}

void indexlib::remove( const char* basename ) {
	assert( basename );
	if ( !basename ) return;

	using namespace indexlib;
	switch ( type_of( basename ) ) {
		case index_type::ifile: ifile::remove( basename );
					break;
		case index_type::quotes: quotes::remove( basename );
					 break;
		case index_type::none: /* do nothing */;
	}
	::unlink( path_concat( basename, "info" ).c_str() );
}



