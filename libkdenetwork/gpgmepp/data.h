/* data.h - wraps a gpgme data object
   Copyright (C) 2003,2004 Klarälvdalens Datakonsult AB

   This file is part of GPGME++.
 
   GPGME++ is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   GPGME++ is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GPGME++; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307 USA.
*/

#ifndef __GPGMEPP_DATA_H__
#define __GPGMEPP_DATA_H__

#include <gpgmepp/gpgmefw.h>

#include <sys/types.h> // for size_t, off_t
#include <cstdio> // FILE
#include <kdepimmacros.h>

namespace GpgME {

  class DataProvider;
  class Error;

  class KDE_EXPORT Data {
  public:
    Data();
    Data( gpgme_data_t data );
    Data( const Data & other );

    // Memory-Based Data Buffers:
    Data( const char * buffer, size_t size, bool copy=true );
    Data( const char * filename );
    Data( const char * filename, off_t offset, size_t length );
    Data( FILE * fp, off_t offset, size_t length );
    // File-Based Data Buffers:
    Data( FILE * fp );
    Data( int fd );
    // Callback-Based Data Buffers:
    Data( DataProvider * provider );

    virtual ~Data();

    static Data null;

    const Data & operator=( const Data & other );

    bool isNull() const;

    ssize_t read( void * buffer, size_t length );
    ssize_t write( const void * buffer, size_t length );
    off_t seek( off_t offset, int whence );

    class Private;
    Private * impl() const { return d; }
  private:
    Private * d;
  };

}

#endif // __GPGMEPP_DATA_H__
