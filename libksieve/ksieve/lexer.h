/*  -*- c++ -*-
    ksieve/lexer.h

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

#ifndef __KSIEVE_LEXER_H__
#define __KSIEVE_LEXER_H__

#include <ksieve/error.h>

#include <qvaluestack.h>

class QString;

namespace KSieve {

  class Lexer {
  public:
    enum Options {
      IncludeComments = 0,
      IgnoreComments = 1
    };

    Lexer( const char * scursor, const char * send, int options=0 );

    /** Return whether comments are returned by @ref
	nextToken. Default is to not ignore comments. Ignoring them
	can speed up script parsing a bit, and can be used when the
	internal representation of the scipt won't be serialized into
	string form again (or if you simply want to delete all
	comments)
    **/
    bool ignoreComments() const { return mIgnoreComments; }

    const Error & error() const { return mState.error; }

    bool atEnd() const { return mState.cursor >= mEnd; }
    int column() const { return mState.cursor - mState.beginOfLine; }
    int line() const { return mState.line; }

    enum Token {
      None = 0,
      Number,          // 1, 100, 1M, 10k, 1G, 2g, 3m
      Identifier,      // atom
      Tag,             // :tag
      Special,         // {} [] () ,;
      QuotedString,    // "foo\"bar" -> foo"bar
      MultiLineString, // text: \nfoo\n. -> foo
      HashComment,     // # foo
      BracketComment   // /* foo */
    };

    /** Parse the next token and return it's type. @p result will contain
	the value of the token. */
    Token nextToken( QString & result );

    void save() { mStateStack.push( mState ); }
    void restore() { mState = mStateStack.pop(); }
      
  private:

    /** @p mCursor must be positioned on the \r or the \n. */
    bool eatCRLF();

    /** @p scursor must be positioned after the opening hash (#). If
	parsing is successful, mCursor is positioned behind the CRLF
	that ended the comment's line (or past the end). */
    bool parseHashComment( QString & result, bool reallySave=false );
    
    /** @p scursor must be positioned after the opening slash-asterisk */
    bool parseBracketComment( QString & result, bool reallySave=false );
    
    /** @p mCursor must be positioned on the opening '/'or '#' */
    bool parseComment( QString & result, bool reallySave=false );

    /** Eats whitespace, but not comments */
    bool eatWS();

    /** Eats comments and whitespace */
    bool eatCWS();

    /** @p mCursor must be positioned on the first character */
    bool parseIdentifier( QString & result );

    /** @p mCursor must be positioned after the initial ':' */
    bool parseTag( QString & result );

    /** @p mCursor must be positioned on the first digit */
    bool parseNumber( QString & result );

    /** @p mCursor must be positioned after the "text:" token. */
    bool parseMultiLine( QString & result );

    /** @p mCursor must be positioned after the initial " */
    bool parseQuotedString( QString & result );

  private:
    struct State {
      State( const char * s=0 )
	: cursor( s ), line( 0 ), beginOfLine( s ), error() {}
      const char * cursor;
      int line;
      const char * beginOfLine;
      Error error;
    } mState;

    const char * const mEnd;
    const bool mIgnoreComments;
    QValueStack<State> mStateStack;

  private:
    const char * beginOfLine() const { return mState.beginOfLine; }

    int _strnicmp( const char * left, const char * right, size_t len ) const;

    void clearErrors() { mState.error = Error(); }

    unsigned int charsLeft() const {
      return mEnd - mState.cursor < 0 ? 0 : mEnd - mState.cursor ;
    }
    void makeError( Error::Type e ) {
      makeError( e, line(), column() );
    }
    void makeError( Error::Type e, int errorLine, int errorCol ) {
      mState.error = Error( e, errorLine, errorCol );
    }
    void makeIllegalCharError( char ch );
    void makeIllegalCharError() {
      makeIllegalCharError( *mState.cursor );
    }
    /** Defines the current char to end a line.
	Warning: increases @p mCursor!
    **/
    void newLine() {
      ++mState.line;
      mState.beginOfLine = ++mState.cursor;
    }
    bool skipTo( char c, bool acceptEnd=false ) {
      while( !atEnd() ) {
	if ( *mState.cursor == '\n' || *mState.cursor == '\r' ) {
	  if ( !eatCRLF() ) return false;
	} else if ( *mState.cursor == c ) {
	  return true;
	} else {
	  ++mState.cursor;
	}
      }
      return acceptEnd;
    }
    bool skipToCRLF( bool acceptEnd=true ) {
      for ( ; !atEnd() ; ++mState.cursor )
	if ( *mState.cursor == '\n' || *mState.cursor == '\r' )
	  return eatCRLF();
      return acceptEnd;
    }
    void skipTo8BitEnd() {
      while ( !atEnd() && (signed char)*mState.cursor < 0 )
	++mState.cursor;
    }
    void skipToDelim();

  private:
    const Lexer & operator=( const Lexer & );
    Lexer( const Lexer & );
  };

} // namespace KSieve

#endif // __KSIEVE_LEXER_H__
