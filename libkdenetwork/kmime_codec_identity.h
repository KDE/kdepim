/*  -*- c++ -*-
    kmime_codec_identity.h

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

#ifndef __KMIME_CODEC_IDENTITY_H__
#define __KMIME_CODEC_IDENTITY_H__

#include "kmime_codecs.h"

namespace KMime {

class IdentityCodec : public Codec {
protected:
  friend class Codec;
  IdentityCodec() : Codec() {}

public:
  ~IdentityCodec() {}

  QByteArray encode( const QByteArray & src, bool withCRLF ) const;
  QCString encodeToQCString( const QByteArray & src, bool withCRLF ) const;
  QByteArray decode( const QByteArray & src, bool withCRLF ) const;

  int maxEncodedSizeFor( int insize, bool withCRLF ) const {
    if ( withCRLF )
      return 2 * insize;
    else
      return insize;
  }

  int maxDecodedSizeFor( int insize, bool withCRLF ) const {
    if ( withCRLF )
      return 2 * insize;
    else
      return insize;
  }

  Encoder * makeEncoder( bool withCRLF=false ) const;
  Decoder * makeDecoder( bool withCRLF=false ) const;
};

class SevenBitCodec : public IdentityCodec {
protected:
  friend class Codec;
  SevenBitCodec() : IdentityCodec() {}

public:
  ~SevenBitCodec() {}

  const char * name() const { return "7bit"; }
};

class EightBitCodec : public IdentityCodec {
protected:
  friend class Codec;
  EightBitCodec() : IdentityCodec() {}

public:
  ~EightBitCodec() {}

  const char * name() const { return "8bit"; }
};

class BinaryCodec : public IdentityCodec {
protected:
  friend class Codec;
  BinaryCodec() : IdentityCodec() {}

public:
  ~BinaryCodec() {}

  const char * name() const { return "binary"; }

  int maxEncodedSizeFor( int insize, bool ) {
    return insize;
  }
  int maxDecodedSizeFor( int insize, bool ) {
    return insize;
  }

  QCString encodeToQCString( const QByteArray &, bool ) const {
    return QCString();
  }

};

} // namespace KMime

#endif // __KMIME_CODEC_IDENTITY_H__
