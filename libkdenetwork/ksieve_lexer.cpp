/*  -*- c++ -*-
    ksieve_lexer.cpp

    This file is part of KSieve,
    the KDE internet mail/usenet news message filtering library.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

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

#include "ksieve_lexer.h"

#include "kmime_util.h"
using KMime::isOfSet;

#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <cassert>
#include <climits>
#include <cctype>
using std::isdigit;


namespace KSieve {

  bool Lexer::eatCRLF() {
    assert( *mCursor == '\n' || *mCursor == '\r' );

    if ( *mCursor == '\r' ) {
      ++mCursor;
      if ( atEnd() || *mCursor != '\n' ) {
	// CR w/o LF -> error
	makeError( Error::CRWithoutLF );
	return false;
      } else {
	// good CRLF
	newLine();
	return true;
      }
    } else /* *mCursor == '\n' */ {
      // good, LF only
      newLine();
      return true;
    }
  }
      

  bool Lexer::parseHashComment( QString & result, bool reallySave ) {
    // hash-comment := "#" *CHAR-NOT-CRLF CRLF

    // check that the caller plays by the rules:
    assert( *(mCursor-1) == '#' );

    const char * commentStart = mCursor;

    // find next CRLF:
    while ( !atEnd() ) {
      if ( *mCursor == '\n' || *mCursor == '\r' ) break;
      ++mCursor;
    }

    const char * commentEnd = mCursor - 1;

    if ( commentEnd == commentStart ) return true; // # was last char in script...

    if ( atEnd() || eatCRLF() ) {
      if ( reallySave ) {
	int commentLength = commentEnd - commentStart + 1;
	result += QString::fromUtf8( commentStart, commentLength );
      }
      return true;
    }

    return false;
  };

  bool Lexer::parseBracketComment( QString & result, bool reallySave ) {
    // bracket-comment := "/*" *(CHAR-NOT-STAR / ("*" CHAR-NOT-SLASH )) "*/"

    // check that caller plays by the rules:
    assert( *(mCursor-2) == '/' );
    assert( *(mCursor-1) == '*' );

    const char * commentStart = mCursor;
    int commentCol = column() - 2;
    int commentLine = mLine;

    // find next asterisk:
    do {
      if ( !skipTo( '*' ) ) {
	if ( !error() )
	  makeError( Error::UnfinishedBracketComment, commentLine, commentCol );
	return false;
      }
    } while ( !atEnd() && *++mCursor != '/' );

    if ( atEnd() ) {
      makeError( Error::UnfinishedBracketComment, commentLine, commentCol );
      return false;
    }

    assert( *mCursor == '/' );

    if ( reallySave ) {
      int commentLength = mCursor - commentStart - 1;
      if ( commentLength > 0 )
	result += QString::fromUtf8( commentStart, commentLength );
    }

    ++mCursor; // eat '/'
    return true;
  }

  bool Lexer::parseComment( QString & result, bool reallySave ) {
    // comment := hash-comment / bracket-comment

    switch( *mCursor ) {
    case '#':
      ++mCursor;
      return parseHashComment( result, reallySave );
    case '/':
      if ( charsLeft() < 2 || mCursor[1] != '*' ) {
	makeError( Error::IllegalCharacter );
	return false;
      } else {
	mCursor += 2; // eat "/*"
	return parseBracketComment( result, reallySave );
      }
    default:
      return false; // don't set an error here - there was no comment
    }
  }

  bool Lexer::eatCWS() {
    // white-space := 1*(SP / CRLF / HTAB / comment )

    while ( !atEnd() ) {
      switch( *mCursor ) {
      case ' ':
      case '\t': // SP / HTAB
	++mCursor;
	break;;
      case '\n':
      case '\r': // CRLF
	if ( !eatCRLF() )
	  return false;
	break;
      case '#':
      case '/': // comments
	{
	  QString dummy;
	  if ( !parseComment( dummy ) )
	    return false;
	}
	break;
      default:
	return true;
      }
    }
    return true;
  }

  // none except a-zA-Z0-9_
  static uchar iTextMap[16] = {
    0x00, 0x00, 0x00, 0x00, // CTLs:        none
    0x00, 0x00, 0xFF, 0xC0, // SP ... '?':  0-9
    0x7F, 0xFF, 0xFF, 0xE1, // '@' ... '_': A-Z_
    0x7F, 0xFF, 0xFF, 0xE0  // '`' ... DEL: a-z
  };

  static inline bool isIText( char ch ) {
    return ch <= 'z' && isOfSet( iTextMap, ch );
  }

  bool Lexer::parseIdentifier( QString & result ) {
    // identifier := (ALPHA / "_") *(ALPHA DIGIT "_")

    if ( !eatCWS() ) return false;
    if ( atEnd() ) return false;

    const char * identifierStart = mCursor;

    // first char:
    uchar ch = *mCursor;
    if ( ch < 'A' /* no digits for the first */ || !isIText( ch ) )
      return false;

    // rest of identifier chars ( now digits are allowed ):
    for ( ; !atEnd() && isIText( *mCursor ) ; ++mCursor );

    int identifierLength = identifierStart - mCursor;

    // Can use the fast fromLatin1 here, since identifiers are always
    // in the us-ascii subset:
    result += QString::fromLatin1( identifierStart, identifierLength );
    return true;
  }

  bool Lexer::parseTag( QString & result ) {
    // tag := ":" identifier

    // check that the caller plays by the rules:
    assert( *(mCursor-1) == ':' );

    return parseIdentifier( result );
  }

  bool Lexer::parseNumber( unsigned int & result, char & quantifier ) {
    // number     := 1*DIGIT [QUANTIFIER]
    // QUANTIFIER := "K" / "M" / "G"

    static const unsigned int maxIntByTen =
      (unsigned int)( double(UINT_MAX) / 10.0 );
    for ( result = 0 ; !atEnd() && isdigit( *mCursor ) && result <= maxIntByTen ; ++mCursor ) {
      result *= 10; result += int( *mCursor - '0' );
    }

    int factor = 1;
    switch ( *mCursor ) {
    case 'G':
    case 'g':
      factor *= 1024;
      // fall through
    case 'M':
    case 'm':
      factor *= 1024;
      // fall though
    case 'k':
    case 'K':
      factor *= 1024;
      quantifier = *mCursor;
      if ( result > double(UINT_MAX) / double(factor) ) {
	makeError( Error::NumberOutOfRange );
	return false;
      }
      result *= factor;
      ++mCursor;
      break;
    default:
      quantifier = '\0';
      break;
    };

    return true;
  };

  static inline QString removeDotStuffAndCRLF( const QString & s ) {
    bool dotstuffed = s.startsWith("..");
    bool CRLF = s.endsWith("\r\n");
    bool LF = !CRLF && s.endsWith("\n");

    int b = dotstuffed ? 1 : 0; // what to skip at the beginning
    int e = CRLF ? 2 : LF ? 1 : 0 ; // what to chop off at the end

    return s.mid( b, s.length() - b - e );
  };

  bool Lexer::parseMultiLine( QString & result ) {
    // multi-line          := "text:" *(SP / HTAB) (hash-comment / CRLF)
    //                        *(multi-line-literal / multi-line-dotstuff)
    //                        "." CRLF
    // multi-line-literal  := [CHAR-NOT-DOT *CHAR-NOT-CRLF] CRLF
    // multi-line-dotstuff := "." 1*CHAR-NOT-CRLF CRLF
    //         ;; A line containing only "." ends the multi-line.
    //         ;; Remove a leading '.' if followed by another '.'.

    if ( !eatCWS() ) return false;
    if ( atEnd() ) return false;

    if ( qstrnicmp( "text:", mCursor, charsLeft() ) )
      return false;
    mCursor += 5; // strlen("text:")


    // From here on, we are bound to return errors since we already
    // detected the multi-line start token "text:"

    while ( !atEnd() ) {
      switch ( *mCursor ) {
      case ' ':
      case '\t':
	++mCursor;
	break;
      case '#':
	{
	  ++mCursor;
	  QString dummy;
	  if ( !parseHashComment( dummy ) )
	    return false;
	  goto MultiLineStart; // break from switch _and_ while
	}
      case '\n':
      case '\r':
	if ( !eatCRLF() ) return false;
	goto MultiLineStart; // break from switch _and_ while
      default:
	makeError( Error::IllegalCharacter );
	return false;
      }
    }

  MultiLineStart:
    if ( atEnd() ) {
      makeError( Error::PrematureEndOfMultiLine );
      return false;
    }

    // Now, collect the single lines until one with only a single dot is found:
    QStringList lines;
    do {
      const char * oldBeginOfLine = mBeginOfLine;
      if ( !skipToCRLF() )
	return false;
      int lineLength = mCursor - oldBeginOfLine;
      QString line;
      if ( lineLength > 0 ) {
	line = QString::fromUtf8( oldBeginOfLine, lineLength );
	lines << removeDotStuffAndCRLF( line );
      } else {
	lines << QString::null;
      }
    } while ( !atEnd() && lines.last() != "." );

    if ( lines.last() != "." ) {
      makeError( Error::PrematureEndOfMultiLine );
      return false;
    }

    result = lines.join("\n");
    return true;
  };


  bool Lexer::parseQuotedString( QString & result ) {
    // quoted-string := DQUOTE *CHAR DQUOTE

    // check that caller plays by the rules:
    assert( *(mCursor-1) == '"' );

    const char * afterLastBackslash = mCursor;
    QTextCodec * codec = QTextCodec::codecForMib( 106 ); // UTF-8
    assert( codec );
    QTextDecoder * dec = codec->makeDecoder();
    assert( dec );
    for ( ; !atEnd() ; ++mCursor ) {
      if ( *mCursor == '\\' ) {
	// decode the chunk from the last backslash to the current one:
	int chunkLength = mCursor - afterLastBackslash;
	if ( chunkLength > 0 )
	  result += dec->toUnicode( afterLastBackslash, chunkLength );
	afterLastBackslash = ++mCursor; // skip \x (together with for loop)
	if ( atEnd() ) {
	  makeError( Error::PrematureEndOfQuotedString );
	  return false;
	}
      } else if ( *mCursor == '"' ) {
	int chunkLength = mCursor - afterLastBackslash;
	if ( chunkLength > 0 )
	  result += dec->toUnicode( afterLastBackslash, chunkLength );
	++mCursor;
	return true;
      }
    }
    makeError( Error::PrematureEndOfQuotedString );
    return false;
  }

}; // namespace KSieve
