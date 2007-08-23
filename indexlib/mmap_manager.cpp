
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

#include "mmap_manager.h"
#include "logfile.h"
#include "exception.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include "format.h"

using indexlib::detail::errno_error;

mmap_manager::mmap_manager( std::string filename )
	:filename_( filename ),
	 pagesize_( ( size_t )sysconf( _SC_PAGESIZE ) ),
	 base_( 0 ),
	 size_( 0 )
{
	fd_ = open( filename.c_str(), O_RDWR | O_CREAT, 0644 );
	logfile() << format( "open( %s, O_RDWR) returned %s\n" ) % filename % fd_;
	if ( fd_ > 0 ) {
		struct stat st;
		if ( fstat( fd_, &st ) == -1 ) {
			throw errno_error( "stat()" );
		}
		if ( st.st_size ) map( st.st_size );
	} else {
		fd_ = open( filename.c_str(), O_RDWR );
		if ( !fd_ ) throw errno_error( "open()" );
	}

}

mmap_manager::~mmap_manager()
{
	unmap();
	close( fd_ );
}

void mmap_manager::resize( unsigned ns ) {
	if ( size() >= ns ) return;
	unsigned old_size = size();
	unmap();
	ns = ( ns / pagesize_ + bool( ns % pagesize_ ) ) * pagesize_;
	ftruncate( fd_, ns );
	map( ns );
	logfile() << format( "Going to bzero from %s to %s)\n" ) % old_size % size();
	memset( rw_base( old_size ), 0, size() - old_size );
}

void mmap_manager::unmap() {
	if ( !base_ ) return;
	if ( munmap( base_, size_ ) == -1 ) {
		throw errno_error( "munmap()" ); // This should be BUG
	}
	base_ = 0;
	size_ = 0;
}

void mmap_manager::map( unsigned size ) {
	base_ = mmap( 0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_, 0 );
	if ( base_ == reinterpret_cast<void*>( -1 ) ) {
		throw errno_error( "mmap()" );
	}
	size_ = size;
}

