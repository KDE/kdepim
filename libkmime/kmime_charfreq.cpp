/*
    kmime_charfreq.cpp

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001-2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include "kmime_charfreq.h"

namespace KMime {

CharFreq::CharFreq( const QByteArray & buf )
  : NUL(0),
    CTL(0),
    CR(0), LF(0),
    CRLF(0),
    printable(0),
    eightBit(0),
    total(0),
    lineMin(0xffffffff),
    lineMax(0),
    mTrailingWS(false),
    mLeadingFrom(false)
{
  if ( !buf.isEmpty() )
    count( buf.data(), buf.size() );
}

CharFreq::CharFreq( const char * buf, size_t len )
  : NUL(0),
    CTL(0),
    CR(0), LF(0),
    CRLF(0),
    printable(0),
    eightBit(0),
    total(0),
    lineMin(0xffffffff),
    lineMax(0),
    mTrailingWS(false),
    mLeadingFrom(false)
{
  if ( buf && len > 0 )
    count( buf, len );
}

static inline bool isWS( char ch ) { return ( ch == '\t' || ch == ' ' ); }

void CharFreq::count( const char * it, size_t len ) {

  const char * end = it + len;
  uint currentLineLength = 0;
  // initialize the prevChar with LF so that From_ detection works w/o
  // special-casing:
  char prevChar = '\n';
  char prevPrevChar = 0;

  for ( ; it != end ; ++it ) {
    ++currentLineLength;
    switch ( *it ) {
    case '\0': ++NUL; break;
    case '\r': ++CR;  break;
    case '\n': ++LF;
      if ( prevChar == '\r' ) { --currentLineLength; ++CRLF; }
      if ( currentLineLength >= lineMax ) lineMax = currentLineLength-1;
      if ( currentLineLength <= lineMin ) lineMin = currentLineLength-1;
      if ( !mTrailingWS )
	if ( isWS( prevChar ) || ( prevChar == '\r' && isWS( prevPrevChar ) ) )
	  mTrailingWS = true;
      currentLineLength = 0;
      break;
    case 'F': // check for lines starting with From_ if not found already:
      if ( !mLeadingFrom )
	if ( prevChar == '\n' && end - it >= 5 && !qstrncmp( "From ", it, 5 ) )
	  mLeadingFrom = true;
      ++printable;
      break;
    default:
      {
	uchar c = *it;
	if ( c == '\t' || c >= ' ' && c <= '~' )
	  ++printable;
	else if ( c == 127 || c < ' ' )
	  ++CTL;
	else
	  ++eightBit;
      }
    }
    prevPrevChar = prevChar;
    prevChar = *it;
  }

  // consider the length of the last line
  if ( currentLineLength >= lineMax ) lineMax = currentLineLength;
  if ( currentLineLength <= lineMin ) lineMin = currentLineLength;

  // check whether the last character is tab or space
  if ( isWS( prevChar ) )
    mTrailingWS = true;

  total = len;
}

bool CharFreq::isEightBitData() const {
  return type() == EightBitData;
}

bool CharFreq::isEightBitText() const {
  return type() == EightBitText;
}

bool CharFreq::isSevenBitData() const {
  return type() == SevenBitData;
}

bool CharFreq::isSevenBitText() const {
  return type() == SevenBitText;
}

bool CharFreq::hasTrailingWhitespace() const {
  return mTrailingWS;
}

bool CharFreq::hasLeadingFrom() const {
  return mLeadingFrom;
}

CharFreq::Type CharFreq::type() const {
#if 0
  qDebug( "Total: %d; NUL: %d; CTL: %d;\n"
	  "CR: %d; LF: %d; CRLF: %d;\n"
	  "lineMin: %d; lineMax: %d;\n"
	  "printable: %d; eightBit: %d;\n"
          "trailing whitespace: %s;\n"
          "leading 'From ': %s;\n",
	  total, NUL, CTL, CR, LF, CRLF, lineMin, lineMax,
	  printable, eightBit,
	  mTrailingWS ? "yes" : "no" , mLeadingFrom ? "yes" : "no" );
#endif
  if ( NUL ) // must be binary
    return Binary;

  // doesn't contain NUL's:
  if ( eightBit ) {
    if ( lineMax > 988 ) return EightBitData; // not allowed in 8bit
    if ( CR != CRLF || controlCodesRatio() > 0.2 ) return EightBitData;
    return EightBitText;
  }

  // doesn't contain NUL's, nor 8bit chars:
  if ( lineMax > 988 ) return SevenBitData;
  if ( CR != CRLF || controlCodesRatio() > 0.2 ) return SevenBitData;

  // no NUL, no 8bit chars, no excessive CTLs and no lines > 998 chars:
  return SevenBitText;
}

float CharFreq::printableRatio() const {
  if ( total ) return float(printable) / float(total);
  else         return 0;
}

float CharFreq::controlCodesRatio() const {
  if ( total ) return float(CTL) / float(total);
  else         return 0;
}

} // namespace KMime


