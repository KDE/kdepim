/* dataprovider.cpp
   Copyright (C) 2004 Klar�vdalens Datakonsult AB

   This file is part of QGPGME.

   QGPGME is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   QGPGME is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with QGPGME; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA. */

// -*- c++ -*-

#include <config.h>

#include <qgpgme/dataprovider.h>

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

static bool resizeAndInit( QByteArray & ba, size_t newSize ) {
  const size_t oldSize = ba.size();
  ba.resize( newSize );
  const bool ok = ( newSize == static_cast<size_t>( ba.size() ) );
  if ( ok )
    memset( ba.data() + oldSize, 0, newSize - oldSize );
  return ok;
}

QGpgME::QByteArrayDataProvider::QByteArrayDataProvider()
  : GpgME::DataProvider(), mOff( 0 ) {}

QGpgME::QByteArrayDataProvider::QByteArrayDataProvider( const QByteArray & initialData )
  : GpgME::DataProvider(), mArray( initialData ), mOff( 0 ) {}

QGpgME::QByteArrayDataProvider::~QByteArrayDataProvider() {}

ssize_t QGpgME::QByteArrayDataProvider::read( void * buffer, size_t bufSize ) {
#ifndef NDEBUG
  //qDebug( "QGpgME::QByteArrayDataProvider::read( %p, %d )", buffer, bufSize );
#endif
  if ( bufSize == 0 )
    return 0;
  if ( mOff >= mArray.size() )
    return 0; // EOF
  size_t amount = qMin( bufSize, static_cast<size_t>( mArray.size() - mOff ) );
  assert( amount > 0 );
  memcpy( buffer, mArray.data() + mOff, amount );
  mOff += amount;
  return amount;
}

ssize_t QGpgME::QByteArrayDataProvider::write( const void * buffer, size_t bufSize ) {
#ifndef NDEBUG
  qDebug( "QGpgME::QByteArrayDataProvider::write( %p, %d )", buffer, bufSize );
#endif
  if ( bufSize == 0 )
    return 0;
  if ( mOff >= mArray.size() )
    resizeAndInit( mArray, mOff + bufSize );
  if ( mOff >= mArray.size() ) {
    errno = EIO;
    return -1;
  }
  assert( bufSize <= mArray.size() - mOff );
  memcpy( mArray.data() + mOff, buffer, bufSize );
  mOff += bufSize;
  return bufSize;
}

off_t QGpgME::QByteArrayDataProvider::seek( off_t offset, int whence ) {
#ifndef NDEBUG
  qDebug( "QGpgME::QByteArrayDataProvider::seek( %d, %d )", int(offset), whence );
#endif
  int newOffset = mOff;
  switch ( whence ) {
  case SEEK_SET:
    newOffset = offset;
    break;
  case SEEK_CUR:
    newOffset += offset;
    break;
  case SEEK_END:
    newOffset = mArray.size() + offset;
    break;
  default:
    errno = EINVAL;
    return (off_t)-1;
  }
  return mOff = newOffset;
}

void QGpgME::QByteArrayDataProvider::release() {
#ifndef NDEBUG
  qDebug( "QGpgME::QByteArrayDataProvider::release()" );
#endif
  mArray = QByteArray();
}
