/*  -*- c++ -*-
    kmime_codec_uuencode.h

    This file is part of KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

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

#ifndef __KMIME_CODEC_UUENCODE_H__
#define __KMIME_CODEC_UUENCODE_H__

#include "kmime_codecs.h"

namespace KMime {

class UUCodec : public Codec {
protected:
  friend class Codec;
  UUCodec() : Codec() {}

public:
  virtual ~UUCodec() {}

  const char * name() const {
    return "x-uuencode";
  }

  int maxEncodedSizeFor( int insize, bool withCRLF=false ) const {
    (void)withCRLF;
    return insize; // we have no encoder!
  }

  int maxDecodedSizeFor( int insize, bool withCRLF=false ) const {
    // assuming all characters are part of the uuencode stream (which
    // does almost never hold due to required linebreaking; but
    // additional non-uu chars don't affect the output size), each
    // 4-tupel of them becomes a 3-tupel in the decoded octet
    // stream. So:
    int result = ( ( insize + 3 ) / 4 ) * 3;
    // but all of them may be \n, so
    if ( withCRLF )
      result *= 2; // :-o

    return result;
  }

  Encoder * makeEncoder( bool withCRLF=false ) const;
  Decoder * makeDecoder( bool withCRLF=false ) const;
};

} // namespace KMime

#endif // __KMIME_CODEC_UUENCODE_H__
