/*  -*- c++ -*-

    This file is part of KMime, the KDE internet mail/usenet news message library.

    Copyright (c) 2001-2002 Marc Mutz <mutz@kde.org>

    KMime is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMime is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this library with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "kmime_codecs.h"
#include "kmime_util.h"

#include "kmime_codec_base64.h"
#include "kmime_codec_qp.h"
#include "kmime_codec_uuencode.h"
#include "kmime_codec_identity.h"

#include "kautodeletehash.h"

#include <kasciistringtools.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#include <cassert>
#include <cstring>

using namespace KMime;
using namespace KPIM;

namespace KMime {

// global list of KMime::Codec's
KAutoDeleteHash<QByteArray, Codec> * Codec::all = 0;
static KStaticDeleter< KAutoDeleteHash<QByteArray, Codec> > sdAll;
#if defined(QT_THREAD_SUPPORT)
QMutex* Codec::dictLock = 0;
static KStaticDeleter<QMutex> sdDictLock;
#endif

void Codec::fillDictionary() {
  //all->insert( "7bit", new SevenBitCodec() );
  //all->insert( "8bit", new EightBitCodec() );
  all->insert( "base64", new Base64Codec() );
  all->insert( "quoted-printable", new QuotedPrintableCodec() );
  all->insert( "b", new Rfc2047BEncodingCodec() );
  all->insert( "q", new Rfc2047QEncodingCodec() );
  all->insert( "x-kmime-rfc2231", new Rfc2231EncodingCodec() );
  all->insert( "x-uuencode", new UUCodec() );
  //all->insert( "binary", new BinaryCodec() );

}

Codec * Codec::codecForName( const char * name ) {
  const QByteArray ba( name );
  return codecForName( ba );
}

Codec * Codec::codecForName( const QByteArray & name ) {
#if defined(QT_THREAD_SUPPORT)
  if ( !dictLock )
    sdDictLock.setObject( dictLock, new QMutex );
  dictLock->lock(); // protect "all"
#endif
  if ( !all ) {
    sdAll.setObject( all, new KAutoDeleteHash<QByteArray, Codec>() );
    fillDictionary();
  }
  QByteArray lowerName = name;
  KPIM::kAsciiToLower( lowerName.data() );
  Codec * codec = (*all)[ lowerName ];
#if defined(QT_THREAD_SUPPORT)
  dictLock->unlock();
#endif

  if ( !codec )
    kdDebug() << "Unknown codec \"" << name << "\" requested!" << endl;

  return codec;
}

bool Codec::encode( const char* & scursor, const char * const send,
                    char* & dcursor, const char * const dend,
                    bool withCRLF ) const
{
  // get an encoder:
  Encoder * enc = makeEncoder( withCRLF );
  assert( enc );

  // encode and check for output buffer overflow:
  while ( !enc->encode( scursor, send, dcursor, dend ) )
    if ( dcursor == dend ) {
      delete enc;
      return false; // not enough space in output buffer
    }

  // finish and check for output buffer overflow:
  while ( !enc->finish( dcursor, dend ) )
    if ( dcursor == dend ) {
      delete enc;
      return false; // not enough space in output buffer
    }

  // cleanup and return:
  delete enc;
  return true; // successfully encoded.
}

QByteArray Codec::encode( const QByteArray & src, bool withCRLF ) const
{
  // allocate buffer for the worst case:
  QByteArray result( maxEncodedSizeFor( src.size(), withCRLF ) );

  // set up iterators:
  QByteArray::ConstIterator iit = src.begin();
  QByteArray::ConstIterator iend = src.end();
  QByteArray::Iterator oit = result.begin();
  QByteArray::ConstIterator oend = result.end();

  // encode
  if ( !encode( iit, iend, oit, oend, withCRLF ) )
    kdFatal() << name() << " codec lies about it's mEncodedSizeFor()"
              << endl;

  // shrink result to actual size:
  result.truncate( oit - result.begin() );

  return result;
}

QByteArray Codec::decode( const QByteArray & src, bool withCRLF ) const
{
  // allocate buffer for the worst case:
  QByteArray result( maxDecodedSizeFor( src.size(), withCRLF ) );

  // set up iterators:
  QByteArray::ConstIterator iit = src.begin();
  QByteArray::ConstIterator iend = src.end();
  QByteArray::Iterator oit = result.begin();
  QByteArray::ConstIterator oend = result.end();

  // decode
  if ( !decode( iit, iend, oit, oend, withCRLF ) )
    kdFatal() << name() << " codec lies about it's maxDecodedSizeFor()"
              << endl;

  // shrink result to actual size:
  result.truncate( oit - result.begin() );

  return result;
}

bool Codec::decode( const char* & scursor, const char * const send,
                    char* & dcursor, const char * const dend,
                    bool withCRLF ) const
{
  // get a decoder:
  Decoder * dec = makeDecoder( withCRLF );
  assert( dec );

  // decode and check for output buffer overflow:
  while ( !dec->decode( scursor, send, dcursor, dend ) )
    if ( dcursor == dend ) {
      delete dec;
      return false; // not enough space in output buffer
    }

  // finish and check for output buffer overflow:
  while ( !dec->finish( dcursor, dend ) )
    if ( dcursor == dend ) {
      delete dec;
      return false; // not enough space in output buffer
    }

  // cleanup and return:
  delete dec;
  return true; // successfully encoded.
}

// write as much as possible off the output buffer. Return true if
// flushing was complete, false if some chars could not be flushed.
bool Encoder::flushOutputBuffer( char* & dcursor, const char * const dend ) {
  int i;
  // copy output buffer to output stream:
  for ( i = 0 ; dcursor != dend && i < mOutputBufferCursor ; ++i )
    *dcursor++ = mOutputBuffer[i];

  // calculate the number of missing chars:
  int numCharsLeft = mOutputBufferCursor - i;
  // push the remaining chars to the begin of the buffer:
  if ( numCharsLeft )
    qmemmove( mOutputBuffer, mOutputBuffer + i, numCharsLeft );
  // adjust cursor:
  mOutputBufferCursor = numCharsLeft;

  return !numCharsLeft;
}


} // namespace KMime
