/*  -*- c++ -*-
    utf8validator.cpp

    This file is part of KSieve,
    the KDE internet mail/usenet news message filtering library.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    KSieve is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KSieve is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <ksieve/utf8validator.h>

#include <qglobal.h>
#include <qcstring.h>

namespace KSieve {

  static inline bool is8Bit( signed char ch ) {
    return ch < 0;
  }

  static inline bool isUtf8TupelIndicator( unsigned char ch ) {
    return (ch & 0xE0) == 0xC0; // 110x xxxx
  }
  static inline bool isUtf8OverlongTupel( unsigned char ch ) {
    return (ch & 0xFE) == 0xC0;
  }

  static inline bool isUtf8TripleIndicator( unsigned char ch ) {
    return (ch & 0xF0) == 0xE0; // 1110 xxxx
  }
  static inline bool isUtf8OverlongTriple( unsigned char ch1, unsigned char ch2 ) {
    return (ch1 & 0xFF) == 0xE0  &&  (ch2 & 0xE0) == 0x80 ;
  }

  static inline bool isUtf8QuartetIndicator( unsigned char ch ) {
    return (ch & 0xF8) == 0xF0; // 1111 0xxx
  }
  static inline bool isUtf8OverlongQuartet( unsigned char ch1, unsigned char ch2 ) {
    return (ch1 & 0xFF) == 0xF0  &&  (ch2 & 0xF0) == 0x80 ;
  }

  static inline bool isUtf8QuintetIndicator( unsigned char ch ) {
    return (ch & 0xFC) == 0xF8; // 1111 10xx
  }
  static inline bool isUtf8OverlongQuintet( unsigned char ch1, unsigned char ch2 ) {
    return (ch1 & 0xFF) == 0xF8  &&  (ch2 & 0xF8) == 0x80 ;
  }

  static inline bool isUtf8SextetIndicator( unsigned char ch ) {
    return (ch & 0xFE) == 0xFC; // 1111 110x
  }
  static inline bool isUtf8OverlongSextet( unsigned char ch1, unsigned char ch2 ) {
    return (ch1 & 0xFF) == 0xFC  &&  (ch2 & 0xFC) == 0x80 ;
  }

  static inline bool isUtf8Continuation( unsigned char ch ) {
    return (ch & 0xC0) == 0x80;
  }

  bool KSieve::isValidUtf8( const char * s, unsigned int len ) {
    for ( unsigned int i = 0 ; i < len ; ++i ) {
      const unsigned char ch = s[i];
      if ( !is8Bit( ch ) )
	continue;
      if ( isUtf8TupelIndicator( ch ) ) {
	if ( len - i < 1 ) // too short
	  return false;
	if ( isUtf8OverlongTupel( ch ) ) // not minimally encoded
	  return false;
	if ( !isUtf8Continuation( s[i+1] ) ) // not followed by 10xx xxxx
	  return false;
	i += 1;
      } else if ( isUtf8TripleIndicator( ch ) ) {
	if ( len - i < 2 ) // too short
	  return false;
	if ( isUtf8OverlongTriple( ch, s[i+1] ) ) // not minimally encoded
	  return false;
	if ( !isUtf8Continuation( s[i+2] ) ) // not followed by 10xx xxxx
	  return false;
	i += 2;
      } else if ( isUtf8QuartetIndicator( ch ) ) {
	if ( len - i < 3 ) // too short
	  return false;
	if ( isUtf8OverlongQuartet( ch, s[i+1] ) ) // not minimally encoded
	  return false;
	if ( !isUtf8Continuation( s[i+2] ) ||
	     !isUtf8Continuation( s[i+3] ) ) // not followed by 2x 10xx xxxx
	  return false;
	i += 3;
      } else if ( isUtf8QuintetIndicator( ch ) ) {
	if ( len - i < 4 ) // too short
	  return false;
	if ( isUtf8OverlongQuintet( ch, s[i+1] ) ) // not minimally encoded
	  return false;
	if ( !isUtf8Continuation( s[i+2] ) ||
	     !isUtf8Continuation( s[i+3] ) ||
	     !isUtf8Continuation( s[i+4] ) ) // not followed by 3x 10xx xxxx
	  return false;
	i += 4;
      } else if ( isUtf8SextetIndicator( ch ) ) {
	if ( len - i < 5 ) // too short
	  return false;
	if ( isUtf8OverlongSextet( ch, s[i+1] ) ) // not minimally encoded
	  return false;
	if ( !isUtf8Continuation( s[i+2] ) ||
	     !isUtf8Continuation( s[i+3] ) ||
	     !isUtf8Continuation( s[i+4] ) ||
	     !isUtf8Continuation( s[i+5] ) ) // not followed by 4x 10xx xxxx
	  return false;
	i += 5;
      } else
	return false;
    }
    return true;
  }

} // namespace KSieve

