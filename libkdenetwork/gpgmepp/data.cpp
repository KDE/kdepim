/* data.cpp - wraps a gpgme data object
   Copyright (C) 2003 Klarälvdalens Datakonsult AB

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gpgmepp/context.h> // Error
#include <gpgmepp/interfaces/dataprovider.h>
#include "data_p.h"

#include <gpgme.h>

#ifndef NDEBUG
#include <iostream>
#endif

GpgME::Data::Private::~Private()  {
  if ( data )
    gpgme_data_release( data );
}

GpgME::Data GpgME::Data::null( (gpgme_data_t)0 );

GpgME::Data::Data() {
  gpgme_data_t data;
  const gpgme_error_t e = gpgme_data_new( &data );
  d = new Private( e ? 0 : data );
  d->ref();
}

GpgME::Data::Data( gpgme_data_t data ) {
  d = new Private( data );
  d->ref();
}

GpgME::Data::Data( const Data & other )
  : d( other.d )
{
  d->ref();
}

GpgME::Data::~Data() {
  d->unref(); d = 0;
}


const GpgME::Data & GpgME::Data::operator=( const Data & other ) {
  if ( this->d == other.d ) return *this;

  if ( other.d )
    other.d->ref();
  if ( this->d )
    this->d->unref();
  this->d = other.d;

  return *this;
}

GpgME::Data::Data( const char * buffer, size_t size, bool copy ) {
  gpgme_data_t data;
  const gpgme_error_t e = gpgme_data_new_from_mem( &data, buffer, size, int( copy ) );
  d = new Private( e ? 0 : data );
  d->ref();
}

GpgME::Data::Data( const char * filename ) {
  gpgme_data_t data;
  const gpgme_error_t e = gpgme_data_new_from_file( &data, filename, 1 );
  d = new Private( e ? 0 : data );
  d->ref();
}

GpgME::Data::Data( const char * filename, off_t offset, size_t length ) {
  gpgme_data_t data;
  const gpgme_error_t e = gpgme_data_new_from_filepart( &data, filename, 0, offset, length );
  d = new Private( e ? 0 : data );
  d->ref();
}

GpgME::Data::Data( FILE * fp ) {
  gpgme_data_t data;
  const gpgme_error_t e = gpgme_data_new_from_stream( &data, fp );
  d = new Private( e ? 0 : data );
  d->ref();
}

GpgME::Data::Data( FILE * fp, off_t offset, size_t length ) {
  gpgme_data_t data;
  const gpgme_error_t e = gpgme_data_new_from_filepart( &data, 0, fp, offset, length );
  d = new Private( e ? 0 : data );
  d->ref();
}

GpgME::Data::Data( int fd ) {
  gpgme_data_t data;
  const gpgme_error_t e = gpgme_data_new_from_fd( &data, fd );
  d = new Private( e ? 0 : data );
  d->ref();
}

GpgME::Data::Data( DataProvider * dp ) {
  d = new Private();
  d->ref();
  if ( !dp )
    return;
  if ( !dp->isSupported( DataProvider::Read ) )
    d->cbs.read = 0;
  if ( !dp->isSupported( DataProvider::Write ) )
    d->cbs.write = 0;
  if ( !dp->isSupported( DataProvider::Seek ) )
    d->cbs.seek = 0;
  if ( !dp->isSupported( DataProvider::Release ) )
    d->cbs.release = 0;
  const gpgme_error_t e = gpgme_data_new_from_cbs( &d->data, &d->cbs, dp );
  if ( e )
    d->data = 0;
#ifndef NDEBUG
  std::cerr << "GpgME::Data(): DataProvider supports: "
	    << ( d->cbs.read ? "read" : "no read" ) << ", "
	    << ( d->cbs.write ? "write" : "no write" ) << ", "
	    << ( d->cbs.seek ? "seek" : "no seek" ) << ", "
	    << ( d->cbs.release ? "release" : "no release" ) << std::endl;
#endif
}



bool GpgME::Data::isNull() const {
  return !d || !d->data;
}

ssize_t GpgME::Data::read( void * buffer, size_t length ) {
  return gpgme_data_read( d->data, buffer, length );
}

ssize_t GpgME::Data::write( const void * buffer, size_t length ) {
  return gpgme_data_write( d->data, buffer, length );
}

off_t GpgME::Data::seek( off_t offset, int whence ) {
  return gpgme_data_seek( d->data, offset, whence );
}
