/*  -*- c++ -*-
    kmime_codec_identity.cpp

    This file is part of KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2004 Marc Mutz <mutz@kde.org>

    KMime is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMime is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#include "kmime_codec_identity.h"

#include <kdebug.h>
#include <kglobal.h>

#include <cassert>
#include <cstring>

using namespace KMime;

namespace KMime {


class IdentityEnDecoder : public Encoder, public Decoder {
protected:
  friend class IdentityCodec;
  IdentityEnDecoder( bool withCRLF )
    : Encoder( false )
  {
    kdWarning( withCRLF, 5100 ) << "IdentityEnDecoder: withCRLF isn't yet supported!" << endl;
  }

public:
  ~IdentityEnDecoder() {}

  bool encode( const char* & scursor, const char * const send,
	       char* & dcursor, const char * const dend ) {
    return decode( scursor, send, dcursor, dend );
  }
  bool decode( const char* & scursor, const char * const send,
	       char* & dcursor, const char * const dend );
  bool finish( char* & /*dcursor*/, const char * const /*dend*/ ) { return true; }
};


Encoder * IdentityCodec::makeEncoder( bool withCRLF ) const {
  return new IdentityEnDecoder( withCRLF );
}

Decoder * IdentityCodec::makeDecoder( bool withCRLF ) const {
  return new IdentityEnDecoder( withCRLF );
}


  /********************************************************/
  /********************************************************/
  /********************************************************/



bool IdentityEnDecoder::decode( const char* & scursor, const char * const send,
				char* & dcursor, const char * const dend )
{
  const int size = kMin( send - scursor, dcursor - dend );
  if ( size > 0 ) {
    std::memmove( dcursor, scursor, size );
    dcursor += size;
    scursor += size;
  }
  return scursor == send;
}

QByteArray IdentityCodec::encode( const QByteArray & src, bool withCRLF ) const {
  kdWarning( withCRLF, 5100 ) << "IdentityCodec::encode(): withCRLF not yet supported!" << endl;
  return src;
}

QByteArray IdentityCodec::decode( const QByteArray & src, bool withCRLF ) const {
  kdWarning( withCRLF, 5100 ) << "IdentityCodec::decode(): withCRLF not yet supported!" << endl;
  return src;
}

QCString IdentityCodec::encodeToQCString( const QByteArray & src, bool withCRLF ) const {
  kdWarning( withCRLF, 5100 ) << "IdentityCodec::encodeToQCString(): withCRLF not yet supported!" << endl;
  return QCString( src.data(), src.size() + 1 );
}

} // namespace KMime
