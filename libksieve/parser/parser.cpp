/*  -*- c++ -*-
    parser/parser.cpp

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

#include <ksieve/parser.h>
#include <impl/parser.h>

#include <ksieve/error.h>

#include <qstring.h>

#include <assert.h>
#include <limits.h> // ULONG_MAX
#include <ctype.h> // isdigit

namespace KSieve {

  //
  //
  // Parser Bridge implementation
  //
  //

  Parser::Parser( const char * scursor, const char * const send, int options )
    : i( 0 )
  {
    i = new Impl( scursor, send, options );
  }

  Parser::~Parser() {
    delete i; i = 0;
  }

  void Parser::setScriptBuilder( ScriptBuilder * builder ) {
    assert( i );
    i->mBuilder = builder;
  }

  ScriptBuilder * Parser::scriptBuilder() const {
    assert( i );
    return i->mBuilder;
  }

  const Error & Parser::error() const {
    assert( i );
    return i->error();
  }

  bool Parser::parse() {
    assert( i );
    return i->parse();
  }

}

static inline unsigned long factorForQuantifier( char ch ) {
  switch ( ch ) {
  case 'g':
  case 'G':
    return 1024*1024*1024;
  case 'm':
  case 'M':
    return 1024*1024;
  case 'k':
  case 'K':
    return 1024;
  default:
    assert( 0 ); // lexer should prohibit this
    return 1; // make compiler happy
  }
}

static inline bool willOverflowULong( unsigned long result, unsigned long add ) {
  static const unsigned long maxULongByTen =
    (unsigned long)( double(ULONG_MAX) / 10.0 );
  return result > maxULongByTen || ULONG_MAX - 10 * result < add ;
}

namespace KSieve {

  //
  //
  // Parser Implementation
  //
  //

  Parser::Impl::Impl( const char * scursor, const char * const send, int options )
    : mToken( Lexer::None ),
      lexer( scursor, send, options ),
      mBuilder( 0 )
  {

  }

  bool Parser::Impl::isStringToken() const {
    return token() == Lexer::QuotedString ||
           token() == Lexer::MultiLineString ;
  }


  bool Parser::Impl::isArgumentToken() const {
    return isStringToken() ||
           token() == Lexer::Number ||
           token() == Lexer::Tag ||
           token() == Lexer::Special && mTokenValue == "[" ;
  }

  bool Parser::Impl::obtainToken() {
    while ( !mToken && !lexer.atEnd() && !lexer.error() ) {
      mToken = lexer.nextToken( mTokenValue );
      if ( lexer.error() )
	break;
      // comments and line feeds are semantically invisible and may
      // appear anywhere, so we handle them here centrally:
      switch ( token() ) {
      case Lexer::HashComment:
	if ( scriptBuilder() )
	  scriptBuilder()->hashComment( tokenValue() );
	consumeToken();
	break;
      case Lexer::BracketComment:
	if ( scriptBuilder() )
	  scriptBuilder()->bracketComment( tokenValue() );
	consumeToken();
	break;
      case Lexer::LineFeeds:
	for ( unsigned int i = tokenValue().toUInt() ; i > 0 ; --i )
	  if ( scriptBuilder() ) // better check every iteration, b/c
				 // we call out to ScriptBuilder,
				 // where nasty things might happen!
	    scriptBuilder()->lineFeed();
	consumeToken();
	break;
      default: ; // make compiler happy
      }
    }
    if ( lexer.error() && scriptBuilder() )
      scriptBuilder()->error( lexer.error() );
    return !lexer.error();
  }

  bool Parser::Impl::parse() {
    // this is the entry point: START := command-list
    if ( !parseCommandList() )
      return false;
    if ( !atEnd() ) {
      makeUnexpectedTokenError( Error::ExpectedCommand );
      return false;
    }
    if ( scriptBuilder() )
      scriptBuilder()->finished();
    return true;
  }


  bool Parser::Impl::parseCommandList() {
    // our ABNF:
    // command-list := *comand

    while ( !atEnd() ) {
      if ( !obtainToken() )
	return false;
      if ( token() == Lexer::None )
	continue;
      if ( token() != Lexer::Identifier )
	return true;
      if ( !parseCommand() ) {
	assert( error() );
	return false;
      }
    }
    return true;
  }


  bool Parser::Impl::parseCommand() {
    // command   := identifier arguments ( ";" / block )
    // arguments := *argument [ test / test-list ]
    // block     := "{" *command "}"
    // our ABNF:
    // block     := "{" [ command-list ] "}"

    if ( atEnd() )
      return false;

    //
    // identifier
    //

    if ( !obtainToken() || token() != Lexer::Identifier )
      return false;

    if ( scriptBuilder() )
      scriptBuilder()->commandStart( tokenValue() );
    consumeToken();

    //
    // *argument
    //

    if ( !obtainToken() )
      return false;

    if ( atEnd() ) {
      makeError( Error::MissingSemicolonOrBlock );
      return false;
    }

    if ( isArgumentToken() && !parseArgumentList() ) {
      assert( error() );
      return false;
    }

    //
    // test / test-list
    //

    if ( !obtainToken() )
      return false;

    if ( atEnd() ) {
      makeError( Error::MissingSemicolonOrBlock );
      return false;
    }

    if ( token() == Lexer::Special && tokenValue() == "(" ) { // test-list
      if ( !parseTestList() ) {
	assert( error() );
	return false;
      }
    } else if ( token() == Lexer::Identifier ) { // should be test:
      if ( !parseTest() ) {
	assert( error() );
	return false;
      }
    }

    //
    // ";" / block
    //

    if ( !obtainToken() )
      return false;

    if ( atEnd() ) {
      makeError( Error::MissingSemicolonOrBlock );
      return false;
    }

    if ( token() != Lexer::Special ) {
      makeUnexpectedTokenError( Error::ExpectedBlockOrSemicolon );
      return false;
    }

    if ( tokenValue() == ";" )
      consumeToken();
    else if ( tokenValue() == "{" ) { // block
      if ( !parseBlock() )
	return false; // it's an error since we saw '{'
    } else {
      makeError( Error::MissingSemicolonOrBlock );
      return false;
    }

    if ( scriptBuilder() )
      scriptBuilder()->commandEnd();
    return true;
  }


  bool Parser::Impl::parseArgumentList() {
    // our ABNF:
    // argument-list := *argument

    while ( !atEnd() ) {
      if ( !obtainToken() )
	return false;
      if ( !isArgumentToken() )
	return true;
      if ( !parseArgument() )
	return !error();
    }
    return true;
  }


  bool Parser::Impl::parseArgument() {
    // argument := string-list / number / tag

    if ( !obtainToken() || atEnd() )
      return false;

    if ( token() == Lexer::Number ) {
      if ( !parseNumber() ) {
	assert( error() );
	return false;
      }
      return true;
    } else if ( token() == Lexer::Tag ) {
      if ( scriptBuilder() )
	scriptBuilder()->taggedArgument( tokenValue() );
      consumeToken();
      return true;
    } else if ( isStringToken() ) {
      if ( scriptBuilder() )
	scriptBuilder()->stringArgument( tokenValue(), token() == Lexer::MultiLineString, QString::null );
      consumeToken();
      return true;
    } else if ( token() == Lexer::Special && tokenValue() == "[" ) {
      if ( !parseStringList() ) {
	assert( error() );
	return false;
      }
      return true;
    }

    return false;
  }


  bool Parser::Impl::parseTestList() {
    // test-list := "(" test *("," test) ")"
    
    if ( !obtainToken() || atEnd() )
      return false;
    
    if ( token() != Lexer::Special || tokenValue() != "(" )
      return false;
    if ( scriptBuilder() )
      scriptBuilder()->testListStart();
    consumeToken();
    
    // generic while/switch construct for comma-separated lists. See
    // parseStringList() for another one. Any fix here is like to apply there, too.
    bool lastWasComma = true;
    while ( !atEnd() ) {
      if ( !obtainToken() )
	return false;
      
      switch ( token() ) {
      case Lexer::None:
	break;
      case Lexer::Special:
	assert( tokenValue().length() == 1 );
	assert( tokenValue()[0].latin1() );
	switch ( tokenValue()[0].latin1() ) {
	case ')':
	  consumeToken();
	  if ( lastWasComma ) {
	    makeError( Error::ConsecutiveCommasInTestList );
	    return false;
	  }
	  if ( scriptBuilder() )
	    scriptBuilder()->testListEnd();
	  return true;
	case ',':
	  consumeToken();
	  if( lastWasComma ) {
	    makeError( Error::ConsecutiveCommasInTestList );
	    return false;
	  }
	  lastWasComma = true;
	  break;
	default:
	  makeError( Error::NonStringInStringList );
	  return false;
	}
	break;
	
      case Lexer::Identifier:
	if ( !lastWasComma ) {
	  makeError( Error::MissingCommaInTestList );
	  return false;
	} else {
	  lastWasComma = false;
	  if ( !parseTest() ) {
	    assert( error() );
	    return false;
	  }
	}
	break;
	
      default:
	makeUnexpectedTokenError( Error::NonTestInTestList );
	return false;
      }
    }
    
    makeError( Error::PrematureEndOfTestList );
    return false;
  }


  bool Parser::Impl::parseTest() {
    // test := identifier arguments
    // arguments := *argument [ test / test-list ]

    //
    // identifier
    //

    if ( !obtainToken() || atEnd() )
      return false;

    if ( token() != Lexer::Identifier )
      return false;

    if ( scriptBuilder() )
      scriptBuilder()->testStart( tokenValue() );
    consumeToken();

    //
    // *argument
    //

    if ( !obtainToken() )
      return false;

    if ( atEnd() ) // a test w/o args
      goto TestEnd;

    if ( isArgumentToken() && !parseArgumentList() ) {
      assert( error() );
      return false;
    }

    //
    // test / test-list
    //

    if ( !obtainToken() )
      return false;

    if ( atEnd() ) // a test w/o nested tests
      goto TestEnd;

    if ( token() == Lexer::Special && tokenValue() == "(" ) { // test-list
      if ( !parseTestList() ) {
	assert( error() );
	return false;
      }
    } else if ( token() == Lexer::Identifier ) { // should be test:
      if ( !parseTest() ) {
	assert( error() );
	return false;
      }
    }

  TestEnd:
    if ( scriptBuilder() )
      scriptBuilder()->testEnd();
    return true;
  }


  bool Parser::Impl::parseBlock() {
    // our ABNF:
    // block := "{" [ command-list ] "}"

    if ( !obtainToken() || atEnd() )
      return false;

    if ( token() != Lexer::Special || tokenValue() != "{" )
      return false;
    if ( scriptBuilder() )
      scriptBuilder()->blockStart();
    consumeToken();

    if ( !obtainToken() )
      return false;

    if ( atEnd() ) {
      makeError( Error::PrematureEndOfBlock );
      return false;
    }

    if ( token() == Lexer::Identifier ) {
      if ( !parseCommandList() ) {
	assert( error() );
	return false;
      }
    }

    if ( !obtainToken() )
      return false;

    if ( atEnd() ) {
      makeError( Error::PrematureEndOfBlock );
      return false;
    }

    if ( token() != Lexer::Special || tokenValue() != "}" ) {
      makeError( Error::NonCommandInCommandList );
      return false;
    }
    if ( scriptBuilder() )
      scriptBuilder()->blockEnd();
    consumeToken();
    return true;
  }

  bool Parser::Impl::parseStringList() {
    // string-list := "[" string *("," string) "]" / string
    //  ;; if there is only a single string, the brackets are optional
    //
    // However, since strings are already handled separately from
    // string lists in parseArgument(), our ABNF is modified to:
    // string-list := "[" string *("," string) "]"

    if ( !obtainToken() || atEnd() )
      return false;

    if ( token() != Lexer::Special || tokenValue() != "[" )
      return false;

    if ( scriptBuilder() )
      scriptBuilder()->stringListArgumentStart();
    consumeToken();

    // generic while/switch construct for comma-separated lists. See
    // parseTestList() for another one. Any fix here is like to apply there, too.
    bool lastWasComma = true;
    while ( !atEnd() ) {
      if ( !obtainToken() )
	return false;

      switch ( token() ) {
      case Lexer::None:
	break;
      case Lexer::Special:
	assert( tokenValue().length() == 1 );
	switch ( tokenValue()[0].latin1() ) {
	case ']':
	  consumeToken();
	  if ( lastWasComma ) {
	    makeError( Error::ConsecutiveCommasInStringList );
	    return false;
	  }
	  if ( scriptBuilder() )
	    scriptBuilder()->stringListArgumentEnd();
	  return true;
	case ',':
	  consumeToken();
	  if ( lastWasComma ) {
	    makeError( Error::ConsecutiveCommasInStringList );
	    return false;
	  }
	  lastWasComma = true;
	  break;
	default:
	  makeError( Error::NonStringInStringList );
	  return false;
	}
	break;

      case Lexer::QuotedString:
      case Lexer::MultiLineString:
	if ( !lastWasComma ) {
	  makeError( Error::MissingCommaInStringList );
	  return false;
	}
	lastWasComma = false;
	if ( scriptBuilder() )
	  scriptBuilder()->stringListEntry( tokenValue(), token() == Lexer::MultiLineString, QString::null );
	consumeToken();
	break;

      default:
	makeError( Error::NonStringInStringList );
	return false;
      }
    }

    makeError( Error::PrematureEndOfStringList );
    return false;
  }

  bool Parser::Impl::parseNumber() {
    // The lexer returns the number including the quantifier as a
    // single token value. Here, we split is an check that the number
    // is not out of range:

    if ( !obtainToken() || atEnd() )
      return false;

    if ( token() != Lexer::Number )
      return false;

    // number:
    unsigned long result = 0;
    unsigned int i = 0;
    QCString s = tokenValue().latin1();
    unsigned int sLength = s.length();
		for ( ; i < sLength && isdigit( s[i] ) ; ++i ) {
      unsigned long digitValue = s[i] - '0' ;
      if ( willOverflowULong( result, digitValue ) ) {
	makeError( Error::NumberOutOfRange );
	return false;
      } else {
	result *= 10 ; result += digitValue ;
      }
    }

    // optional quantifier:
    char quantifier = '\0';
    if ( i < s.length() ) {
      assert( i == sLength - 1 );
      quantifier = s[i];
      unsigned long factor = factorForQuantifier( quantifier );
      if ( result > double(ULONG_MAX) / double(factor) ) {
	makeError( Error::NumberOutOfRange );
	return false;
      }
      result *= factor;
    }

    if ( scriptBuilder() )
      scriptBuilder()->numberArgument( result, quantifier );
    consumeToken();
    return true;
  }

} // namespace KSieve
