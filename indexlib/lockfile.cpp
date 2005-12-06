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

#include "lockfile.h"
#include "format.h"
#include "logfile.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

using indexlib::detail::lockfile;

lockfile::lockfile( std::string filename ):
	filename_( filename ),
	locked_( false ) {
	}

lockfile::~lockfile() {
       if ( locked() ) unlock();
}

bool lockfile::locked() const {
	return locked_;
}

bool lockfile::trylock() {
	int fd = ::open( filename_.c_str(), O_RDWR | O_EXCL | O_CREAT, 0600 );
	locked_ = false;
	if ( fd >= 0 ) {
		locked_ = true;
		close( fd );
	}
	logfile() << format( "trylock(%s) returning %s (fd:%s) (error:%s)\n" ) % filename_ % locked_ %fd % strerror( errno );
	return locked_;
}

void lockfile::unlock() {
	if ( locked() ) {
		unlink( filename_.c_str() );
		locked_ = false;
	}
}

void lockfile::force_unlock() {
	unlink( filename_.c_str() );
	locked_ = false;
}

