/*  -*- c++ -*-
    ksieve_lexer.h

    KSieve, the KDE internet mail/usenet news message filtering library.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef __KSIEVE_LEXER_H__
#define __KSIEVE_LEXER_H__

#include "ksieve_error.h"

class QString;

namespace KSieve {

  class Lexer {
  public:
    Lexer( const char * scursor, const char * const send )
      : mCursor( scursor ), mEnd( send ), mLine( 0 ), mBeginOfLine( scursor )
    {
    }

    const Error & error() const { return mError; }

    bool atEnd() const { return mCursor == mEnd; }

  protected:

    /** @p mCursor must be positioned on the \r or the \n. */
    bool eatCRLF();

    /** @p scursor must be positioned after the opening hash (#) */
    bool parseHashComment( QString & result, bool reallySave=false );
    
    /** @p scursor must be positioned after the opening slash-asterisk */
    bool parseBracketComment( QString & result, bool reallySave=false );
    
    /** @p mCursor must be positioned on the opening '/'or '#' */
    bool parseComment( QString & result, bool reallySave=false );

    /** Eats comments and whitespace */
    bool eatCWS();

    bool parseIdentifier( QString & result );

    /** @p mCursor must be positioned on the initial ':' */
    bool parseTag( QString & result );

    bool parseNumber( unsigned int & result, char & quantifier );

    bool parseMultiLine( QString & result );

    /** @p mCursor must be positioned after the initial " */
    bool parseQuotedString( QString & result );

  protected:
    const char * mCursor;
    const char * const mEnd;
    int mLine;
    const char * mBeginOfLine;

    Error mError;

  protected:
    int charsLeft() const {
      return mEnd - mCursor;
    }
    int column() const {
      return mCursor - mBeginOfLine;
    }
    void makeError( Error::Type e ) {
      makeError( e, mLine, column() );
    }
    void makeError( Error::Type e, int errorLine, int errorCol ) {
      mError = Error( e, errorLine, errorCol );
    }
    /** Defines the current char to end a line.
	Warning: increases @p mCursor!
    **/
    void newLine() {
      ++mLine;
      mBeginOfLine = ++mCursor;
    }
    bool skipTo( char c, bool acceptEnd=false ) {
      while( !atEnd() ) {
	if ( *mCursor == '\n' || *mCursor == '\r' ) {
	  if ( !eatCRLF() ) return false;
	} else if ( *mCursor == c ) {
	  return true;
	}
      }
      return acceptEnd;
    }
    bool skipToCRLF( bool acceptEnd=true ) {
      while ( !atEnd() )
	if ( *mCursor == '\n' || *mCursor == '\r' )
	  return eatCRLF();
      return acceptEnd;
    }
  };

}; // namespace KSieve

#endif // __KSIEVE_LEXER_H__
