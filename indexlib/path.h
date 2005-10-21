#ifndef LPC_PATH_H1118420718_INCLUDE_GUARD_
#define LPC_PATH_H1118420718_INCLUDE_GUARD_
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * Returns whether \param p is the name of a directory
 */
inline
bool isdir( const char* p ) {
	struct stat st;
	if ( stat( p, &st ) != 0 ) return false;
	return st.st_mode & S_IFDIR;
}

inline
bool isdir( std::string p ) { return isdir( p.c_str() ); }

namespace indexlib { namespace detail {
/**
 * Wrapper around mkdir which handles trailing slashes.
 */
inline 
bool mkdir_trailing( std::string p ) {
	while ( p.size() > 1 && p[ p.size() - 1 ] == '/' ) p.resize( p.size() - 1 );
	if ( p.empty() ) return false;
	return ::mkdir( p.c_str(), 0755 ) == 0;

}

}}
/**
 * If \param base is the basename of an index and \param ext the filename,
 * then this returns strcat( basename, ext ) but it's smart enough to
 * handle the case where basename is a directory differently.
 */
inline
std::string path_concat( std::string base, std::string ext ) {
	if ( isdir( base ) ) {
		base += "/index";
	}
	return base + '.' + ext;
}



#endif /* LPC_PATH_H1118420718_INCLUDE_GUARD_ */
