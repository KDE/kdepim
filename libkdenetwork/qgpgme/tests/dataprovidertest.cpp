/* tests/dataprovidertest.cpp
   Copyright (C) 2004 Klarälvdalens Datakonsult AB

   This file is part of QGPGME's regression test suite.
 
   QGPGME is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   QGPGME is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with QGPGME; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307 USA.  */

// -*- c++ -*-

#ifdef NDEBUG
#undef NDEBUG
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qgpgme/dataprovider.h>
#include <gpgmepp/data.h>
#include <gpgmepp/data_p.h>
#include <gpgme.h>

#include <iostream>

#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>

using namespace GpgME;

static const char input[] = "foo bar baz\0foo bar baz";
static const size_t inputSize = sizeof (input) / sizeof (*input) - 1;
static const char nulls[256] = { '\0' };

#define assertEqual( expr, expected ) \
do { \
  long long foo = (expr); \
  if ( foo != (long long)expected ) { \
    std::cerr << #expr << ": expected " << expected << "; got " << foo \
              << ";errno: " << errno << "(" << strerror( errno ) << ")" << std::endl; \
    exit( 1 ); \
  } \
} while( 0 )

int main( int, char** ) {

  {
    //
    // QByteArrayDataProvider
    //

    // writing:
    QGpgME::QByteArrayDataProvider qba_dp;
    Data data( &qba_dp );

    assertEqual( data.write( input, inputSize ), inputSize );

    const QByteArray ba1 = qba_dp.data();
    assertEqual( ba1.size(), inputSize );
    assertEqual( memcmp( ba1.data(), input, inputSize ), 0 );

    // seeking and reading:
    assertEqual( data.seek( 0, SEEK_CUR ), inputSize );
    assertEqual( data.seek( 4, SEEK_SET ), 4 );
    char ch = '\0';
    assertEqual( data.read( &ch, 1 ), 1 );
    assertEqual( ch, input[4] );
    assertEqual( data.read( &ch, 1 ), 1 );
    assertEqual( ch, input[5] );

    char buf[ inputSize + 10 ] = { '\0' };
    assertEqual( data.seek( 0, SEEK_SET ), 0 );
    assertEqual( data.read( buf, sizeof buf ), inputSize );
    assertEqual( memcmp( buf, input, inputSize ), 0 );

    // writing single char at end:
    assertEqual( data.seek( 0, SEEK_END ), inputSize );
    assertEqual( data.write( &ch, 1 ) , 1 );
    const QByteArray ba2 = qba_dp.data();
    assertEqual( ba2.size(), inputSize + 1 );
    assertEqual( memcmp( ba2.data(), input, inputSize ), 0 );
    assertEqual( ba2[inputSize], ch );

    // writing past end of buffer:
    assertEqual( data.seek( 10, SEEK_END ), inputSize + 11 );
    assertEqual( data.write( &ch, 1 ), 1 );
    const QByteArray ba3 = qba_dp.data();
    assertEqual( ba3.size(), inputSize + 12 );
    assertEqual( memcmp( ba3.data(), input, inputSize ), 0 );
    assertEqual( ba3[inputSize], ch );
    assertEqual( ba3[inputSize+11], ch );
    assertEqual( memcmp( ba3.data() + inputSize + 1, nulls, 10 ), 0 );
  }

  {
    //
    // QByteArrayDataProvider with initial data:
    //
    QByteArray ba;
    ba.duplicate( input, inputSize );
    QGpgME::QByteArrayDataProvider qba_dp( ba );
    Data data( &qba_dp );

    assertEqual( data.seek( 0, SEEK_END ), inputSize );
    assertEqual( data.seek( 0, SEEK_SET ), 0 );
    const QByteArray ba1 = qba_dp.data();
    assertEqual( ba1.size(), inputSize );
    assertEqual( memcmp( ba1.data(), input, inputSize ), 0 );
  }
  return 0;
}
