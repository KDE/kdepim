/*  -*- c++ -*-
    parser/lexer.cpp

    This file is part of KSieve,
    the KDE internet mail/usenet news message filtering library.
    Copyright (c) 2002-2003 Marc Mutz <mutz@kde.org>

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

#include <config.h>

#include <ksieve/lexer.h>
#include <impl/lexer.h>

#include <impl/utf8validator.h>
#include <ksieve/error.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <memory> // std::auto_ptr

#include <assert.h>
#include <ctype.h> // isdigit

#ifdef STR_DIM
# undef STR_DIM
#endif
#define STR_DIM(x) (sizeof(x) - 1)

namespace KSieve {

  //
  //
  // Lexer Bridge implementation
  //
  //

  Lexer::Lexer( const char * scursor, const char * send, int options )
    : i( 0 )
  {
    i = new Impl( scursor, send, options );
  }

  Lexer::~Lexer() {
    delete i; i = 0;
  }

  bool Lexer::ignoreComments() const {
    assert( i );
    return i->ignoreComments();
  }

  const Error & Lexer::error() const {
    assert( i );
    return i->error();
  }

  bool Lexer::atEnd() const {
    assert( i );
    return i->atEnd();
  }

  int Lexer::column() const {
    assert( i );
    return i->column();
  }

  int Lexer::line() const {
    assert( i );
    return i->line();
  }

  void Lexer::save() {
    assert( i );
    i->save();
  }

  void Lexer::restore() {
    assert( i );
    i->restore();
  }

  Lexer::Token Lexer::nextToken( QString & result ) {
    assert( i );
    return i->nextToken( result );
  }

} // namespace KSieve


// none except a-zA-Z0-9_
static const unsigned char iTextMap[16] = {
    0x00, 0x00, 0x00, 0x00, // CTLs:        none
    0x00, 0x00, 0xFF, 0xC0, // SP ... '?':  0-9
    0x7F, 0xFF, 0xFF, 0xE1, // '@' ... '_': A-Z_
    0x7F, 0xFF, 0xFF, 0xE0  // '`' ... DEL: a-z
};

// SP, HT, CR, LF, {}[]();,#/
// ### exclude '['? Why would one want to write identifier["foo"]?
static const unsigned char delimMap[16] = {
    0x00, 0x64, 0x00, 0x00, // CTLs:        CR, HT, LF
    0x90, 0xC9, 0x00, 0x10, // SP ... '?':  SP, #(),;
    0x00, 0x00, 0x00, 0x16, // '@' ... '_': []
    0x00, 0x00, 0x00, 0x16  // '`' ... DEL: {}
};

// All except iText, delim, "*:
static const unsigned char illegalMap[16] = {
    0xFF, 0x9B, 0xFF, 0xFF,
    0x4F, 0x16, 0x00, 0x0F,
    0x80, 0x00, 0x00, 0x0A,
    0x80, 0x00, 0x00, 0x0A
};

static inline bool isOfSet( const unsigned char map[16], unsigned char ch ) {
    assert( ch < 128 );
    return ( map[ ch/8 ] & 0x80 >> ch%8 );
}

static inline bool isIText( unsigned char ch ) {
    return ch <= 'z' && isOfSet( iTextMap, ch );
}

static inline bool isDelim( unsigned char ch ) {
    return ch <= '}' && isOfSet( delimMap, ch );
}

static inline bool isIllegal( unsigned char ch ) {
    return ch >= '~' || isOfSet( illegalMap, ch );
}

static inline bool is8Bit( signed char ch ) {
    return ch < 0;
}

static QString removeDotStuffAndCRLF( const QString & s ) {
    const bool dotstuffed = s.startsWith("..");
    const bool CRLF = s.endsWith("\r\n");
    const bool LF = !CRLF && s.endsWith("\n");

    const int b = dotstuffed ? 1 : 0; // what to skip at the beginning
    const int e = CRLF ? 2 : LF ? 1 : 0 ; // what to chop off at the end

    return s.mid( b, s.length() - b - e );
}

namespace KSieve {

  //
  //
  // Lexer Implementation
  //
  //

  Lexer::Impl::Impl( const char * scursor, const char * send, int options )
    : mState( scursor ? scursor : send ),
      mEnd( send ? send : scursor ),
      mIgnoreComments( options & IgnoreComments ),
      mIgnoreLF( options & IgnoreLineFeeds )
  {
    if ( !scursor || !send )
      assert( atEnd() );
  }

  Lexer::Token Lexer::Impl::nextToken( QString & result ) {
    assert( !atEnd() );
    result = QString::null;
    //clearErrors();

    const int oldLine = line();

    const bool eatingWSSucceeded = ignoreComments() ? eatCWS() : eatWS() ;

    if ( !ignoreLineFeeds() && oldLine != line() ) {
      result.setNum( line() - oldLine ); // return number of linefeeds encountered
      return LineFeeds;
    }

    if ( !eatingWSSucceeded )
      return None;

    if ( atEnd() )
      return None;

    switch ( *mState.cursor ) {
    case '#': // HashComment
      assert( !ignoreComments() );
      ++mState.cursor;
      if ( !atEnd() )
	parseHashComment( result, true );
      return HashComment;
    case '/': // BracketComment
      assert( !ignoreComments() );
      ++mState.cursor; // eat slash
      if ( atEnd() || *mState.cursor != '*' ) {
	makeError( Error::SlashWithoutAsterisk );
	return BracketComment;
      }
      ++mState.cursor; // eat asterisk
      if ( atEnd() ) {
	makeError( Error::UnfinishedBracketComment );
	return BracketComment;
      }
      parseBracketComment( result, true );
      return BracketComment;
    case ':': // Tag
      ++mState.cursor;
      if ( atEnd() ) {
	makeError( Error::UnexpectedCharacter, line(), column() - 1 );
	return Tag;
      }
      if ( !isIText( *mState.cursor ) ) {
	makeIllegalCharError( *mState.cursor );
	return Tag;
      }
      parseTag( result );
      return Tag;
    case '"': // QuotedString
      ++mState.cursor;
      parseQuotedString( result );
      return QuotedString;
    case '{':
    case '}':
    case '[':
    case ']':
    case '(':
    case ')':
    case ';':
    case ',': // Special
      result = *mState.cursor++;
      return Special;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': // Number
      parseNumber( result );
      return Number;
    case 't': // maybe MultiLineString, else Identifier
      if ( _strnicmp( mState.cursor, "text:", STR_DIM("text:") ) == 0 ) {
	// MultiLineString
	mState.cursor += STR_DIM("text:");
	parseMultiLine( result );
	// ### FIXME: There can be a hash-comment between "text:"
	// and CRLF! That should be preserved somehow...
	return MultiLineString;
      }
      // else fall through:
    default: // Identifier (first must not be 0-9, and can't (caught by Number above))
      if ( !isIText( *mState.cursor ) ) {
	makeError( Error::IllegalCharacter );
	return None;
      }
      parseIdentifier( result );
      return Identifier;
    }
  }

  bool Lexer::Impl::eatWS() {
    while ( !atEnd() )
      switch ( *mState.cursor ) {
      case '\r':
      case '\n':
	if ( !eatCRLF() )
	  return false;
	break;
      case ' ':
      case '\t':
	++mState.cursor;
	break;
      default:
	return true;
      }

    // at end:
    return true;
  }

  bool Lexer::Impl::eatCRLF() {
    assert( !atEnd() );
    assert( *mState.cursor == '\n' || *mState.cursor == '\r' );

    if ( *mState.cursor == '\r' ) {
      ++mState.cursor;
      if ( atEnd() || *mState.cursor != '\n' ) {
	// CR w/o LF -> error
	makeError( Error::CRWithoutLF );
	return false;
      } else {
	// good CRLF
	newLine();
	return true;
      }
    } else /* *mState.cursor == '\n' */ {
      // good, LF only
      newLine();
      return true;
    }
  }
      

  bool Lexer::Impl::parseHashComment( QString & result, bool reallySave ) {
    // hash-comment := "#" *CHAR-NOT-CRLF CRLF

    // check that the caller plays by the rules:
    assert( *(mState.cursor-1) == '#' );

    const char * const commentStart = mState.cursor;

    // find next CRLF:
    while ( !atEnd() ) {
      if ( *mState.cursor == '\n' || *mState.cursor == '\r' ) break;
      ++mState.cursor;
    }

    const char * const commentEnd = mState.cursor - 1;

    if ( commentEnd == commentStart ) return true; // # was last char in script...

    if ( atEnd() || eatCRLF() ) {
      const int commentLength = commentEnd - commentStart + 1;
      if ( commentLength > 0 ) {
	if ( !isValidUtf8( commentStart, commentLength ) ) {
	  makeError( Error::InvalidUTF8 );
	  return false;
	}
	if ( reallySave )
	  result += QString::fromUtf8( commentStart, commentLength );
      }
      return true;
    }

    return false;
  }

  bool Lexer::Impl::parseBracketComment( QString & result, bool reallySave ) {
    // bracket-comment := "/*" *(CHAR-NOT-STAR / ("*" CHAR-NOT-SLASH )) "*/"

    // check that caller plays by the rules:
    assert( *(mState.cursor-2) == '/' );
    assert( *(mState.cursor-1) == '*' );

    const char * const commentStart = mState.cursor;
    const int commentCol = column() - 2;
    const int commentLine = line();

    // find next asterisk:
    do {
      if ( !skipTo( '*' ) ) {
	if ( !error() )
	  makeError( Error::UnfinishedBracketComment, commentLine, commentCol );
	return false;
      }
    } while ( !atEnd() && *++mState.cursor != '/' );

    if ( atEnd() ) {
      makeError( Error::UnfinishedBracketComment, commentLine, commentCol );
      return false;
    }

    assert( *mState.cursor == '/' );

    const int commentLength = mState.cursor - commentStart - 1;
    if ( commentLength > 0 ) {
      if ( !isValidUtf8( commentStart, commentLength ) ) {
	makeError( Error::InvalidUTF8 );
	return false;
      }
      if ( reallySave ) {
	QString tmp = QString::fromUtf8( commentStart, commentLength );
	result += tmp.remove( '\r' ); // get rid of CR in CRLF pairs
      }
    }

    ++mState.cursor; // eat '/'
    return true;
  }

  bool Lexer::Impl::parseComment( QString & result, bool reallySave ) {
    // comment := hash-comment / bracket-comment

    switch( *mState.cursor ) {
    case '#':
      ++mState.cursor;
      return parseHashComment( result, reallySave );
    case '/':
      if ( charsLeft() < 2 || mState.cursor[1] != '*' ) {
	makeError( Error::IllegalCharacter );
	return false;
      } else {
	mState.cursor += 2; // eat "/*"
	return parseBracketComment( result, reallySave );
      }
    default:
      return false; // don't set an error here - there was no comment
    }
  }

  bool Lexer::Impl::eatCWS() {
    // white-space := 1*(SP / CRLF / HTAB / comment )

    while ( !atEnd() ) {
      switch( *mState.cursor ) {
      case ' ':
      case '\t': // SP / HTAB
	++mState.cursor;
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

  bool Lexer::Impl::parseIdentifier( QString & result ) {
    // identifier := (ALPHA / "_") *(ALPHA DIGIT "_")

    assert( isIText( *mState.cursor ) );

    const char * const identifierStart = mState.cursor;

    // first char:
    if ( isdigit( *mState.cursor ) ) { // no digits for the first
      makeError( Error::NoLeadingDigits );
      return false;
    }

    // rest of identifier chars ( now digits are allowed ):
    for ( ++mState.cursor ; !atEnd() && isIText( *mState.cursor ) ; ++mState.cursor );

    const int identifierLength = mState.cursor - identifierStart;

    // Can use the fast fromLatin1 here, since identifiers are always
    // in the us-ascii subset:
    result += QString::fromLatin1( identifierStart, identifierLength );

    if ( atEnd() || isDelim( *mState.cursor ) )
      return true;

    makeIllegalCharError( *mState.cursor );
    return false;
  }

  bool Lexer::Impl::parseTag( QString & result ) {
    // tag := ":" identifier

    // check that the caller plays by the rules:
    assert( *(mState.cursor-1) == ':' );
    assert( !atEnd() );
    assert( isIText( *mState.cursor ) );

    return parseIdentifier( result );
  }

  bool Lexer::Impl::parseNumber( QString & result ) {
    // number     := 1*DIGIT [QUANTIFIER]
    // QUANTIFIER := "K" / "M" / "G"

    assert( isdigit( *mState.cursor ) );

    while ( !atEnd() && isdigit( *mState.cursor ) )
      result += *mState.cursor++;

    if ( atEnd() || isDelim( *mState.cursor ) )
      return true;

    switch ( *mState.cursor ) {
    case 'G':
    case 'g':
    case 'M':
    case 'm':
    case 'K':
    case 'k':
      result += *mState.cursor++;
      break;
    default:
      makeIllegalCharError();
      return false;
    }

    // quantifier found. Check for delimiter:
    if ( atEnd() || isDelim( *mState.cursor ) )
      return true;
    makeIllegalCharError();
    return false;
  }

  bool Lexer::Impl::parseMultiLine( QString & result ) {
    // multi-line          := "text:" *(SP / HTAB) (hash-comment / CRLF)
    //                        *(multi-line-literal / multi-line-dotstuff)
    //                        "." CRLF
    // multi-line-literal  := [CHAR-NOT-DOT *CHAR-NOT-CRLF] CRLF
    // multi-line-dotstuff := "." 1*CHAR-NOT-CRLF CRLF
    //         ;; A line containing only "." ends the multi-line.
    //         ;; Remove a leading '.' if followed by another '.'.

    assert( _strnicmp( mState.cursor - 5, "text:", STR_DIM("text:") ) == 0 );

    const int mlBeginLine = line();
    const int mlBeginCol = column() - 5;

    while ( !atEnd() ) {
      switch ( *mState.cursor ) {
      case ' ':
      case '\t':
	++mState.cursor;
	break;
      case '#':
	{
	  ++mState.cursor;
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
	makeError( Error::NonCWSAfterTextColon );
	return false;
      }
    }

  MultiLineStart:
    if ( atEnd() ) {
      makeError( Error::PrematureEndOfMultiLine, mlBeginLine, mlBeginCol );
      return false;
    }

    // Now, collect the single lines until one with only a single dot is found:
    QStringList lines;
    do {
      const char * const oldBeginOfLine = beginOfLine();
      if ( !skipToCRLF() )
	return false;
      const int lineLength = mState.cursor - oldBeginOfLine;
      if ( lineLength > 0 ) {
	if ( !isValidUtf8( oldBeginOfLine, lineLength ) ) {
	  makeError( Error::InvalidUTF8 );
	  return false;
	}
	const QString line = QString::fromUtf8( oldBeginOfLine, lineLength );
	lines.push_back( removeDotStuffAndCRLF( line ) );
      } else {
	lines.push_back( QString::null );
      }
    } while ( !atEnd() && lines.back() != "." );

    if ( lines.back() != "." ) {
      makeError( Error::PrematureEndOfMultiLine, mlBeginLine, mlBeginCol );
      return false;
    }

    assert( !lines.empty() );
    lines.erase( --lines.end() ); // don't include the lone dot.
    result = lines.join("\n");
    return true;
  }

  bool Lexer::Impl::parseQuotedString( QString & result ) {
    // quoted-string := DQUOTE *CHAR DQUOTE

    // check that caller plays by the rules:
    assert( *(mState.cursor-1) == '"' );

    const int qsBeginCol = column() - 1;
    const int qsBeginLine = line();

    const QTextCodec * const codec = QTextCodec::codecForMib( 106 ); // UTF-8
    assert( codec );
    const std::auto_ptr<QTextDecoder> dec( codec->makeDecoder() );
    assert( dec.get() );

    while ( !atEnd() )
      switch ( *mState.cursor ) {
      case '"':
	++mState.cursor;
	return true;
      case '\r':
      case '\n':
	if ( !eatCRLF() )
	  return false;
	result += '\n';
	break;
      case '\\':
	++mState.cursor;
	if ( atEnd() )
	  break;
	// else fall through:
      default:
	if ( !is8Bit( *mState.cursor ) )
	  result += *mState.cursor++;
	else { // probably UTF-8
	  const char * const eightBitBegin = mState.cursor;
	  skipTo8BitEnd();
	  const int eightBitLen = mState.cursor - eightBitBegin;
	  assert( eightBitLen > 0 );
	  if ( isValidUtf8( eightBitBegin, eightBitLen ) )
	    result += dec->toUnicode( eightBitBegin, eightBitLen );
	  else {
	    assert( column() >= eightBitLen );
	    makeError( Error::InvalidUTF8, line(), column() - eightBitLen );
	    return false;
	  }
	}
      }

    makeError( Error::PrematureEndOfQuotedString, qsBeginLine, qsBeginCol );
    return false;
  }

  void Lexer::Impl::makeIllegalCharError( char ch ) {
    makeError( isIllegal( ch ) ? Error::IllegalCharacter : Error::UnexpectedCharacter );
  }

} // namespace KSieve
