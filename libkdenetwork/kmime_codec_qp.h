/*  -*- c++ -*-
    kmime_codec_qp.h

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

#ifndef __KMIME_CODEC_QP__
#define __KMIME_CODEC_QP__

#include "kmime_codecs.h"

namespace KMime {


class QuotedPrintableCodec : public Codec {
protected:
  friend class Codec;
  QuotedPrintableCodec() : Codec() {}

public:
  virtual ~QuotedPrintableCodec() {}

  const char * name() const {
    return "quoted-printable";
  }

  int maxEncodedSizeFor( int insize, bool withCRLF=false ) const {
    // all chars encoded:
    int result = 3*insize;
    // then after 25 hexchars comes a soft linebreak: =(\r)\n
    result += (withCRLF ? 3 : 2) * (insize/25);
    
    return result;
  }

  int maxDecodedSizeFor( int insize, bool withCRLF=false ) const;

  Encoder * makeEncoder( bool withCRLF=false ) const;
  Decoder * makeDecoder( bool withCRLF=false ) const;
};


class Rfc2047QEncodingCodec : public Codec {
protected:
  friend class Codec;
  Rfc2047QEncodingCodec() : Codec() {}

public:
  virtual ~Rfc2047QEncodingCodec() {}

  const char * name() const {
    return "q";
  }

  int maxEncodedSizeFor( int insize, bool withCRLF=false ) const {
    (void)withCRLF; // keep compiler happy
    // this one is simple: We don't do linebreaking, so all that can
    // happen is that every char needs encoding, so:
    return 3*insize;
  }

  int maxDecodedSizeFor( int insize, bool withCRLF=false ) const;

  Encoder * makeEncoder( bool withCRLF=false ) const;
  Decoder * makeDecoder( bool withCRLF=false ) const;
};


class Rfc2231EncodingCodec : public Codec {
protected:
  friend class Codec;
  Rfc2231EncodingCodec() : Codec() {}

public:
  virtual ~Rfc2231EncodingCodec() {}

  const char * name() const {
    return "x-kmime-rfc2231";
  }

  int maxEncodedSizeFor( int insize, bool withCRLF=false ) const {
    (void)withCRLF; // keep compiler happy
    // same as for "q" encoding:
    return 3*insize;
  }

  int maxDecodedSizeFor( int insize, bool withCRLF=false ) const;

  Encoder * makeEncoder( bool withCRLF=false ) const;
  Decoder * makeDecoder( bool withCRLF=false ) const;
};


} // namespace KMime

#endif // __KMIME_CODEC_QP__
