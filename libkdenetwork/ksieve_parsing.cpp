/*  -*- c++ -*-
    ksieve_parsing.cpp

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

#include "ksieve_parsing.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluevector.h>

#include <cassert>
#include <cctype>
using std::isdigit;


namespace KSieve {

  bool Parser::parseString( QString & result ) {
    // string := quoted-string / multi-line

    if ( !eatCWS() ) return false;
    if ( atEnd() ) return false;

    if ( *mCursor == '"' ) { // quoted-string
      ++mCursor; // satisfy pQS's constraints
      return parseQuotedString( result );
    } else if ( *mCursor == 't' )
      return parseMultiLine( result );
    else
      return false;
  }

  bool Parser::parseStringList( QStringList & result ) {
    // string-list := "[" string *("," string) "]" / string
    //  ;; if there is only a single string, the brackets are optional

    if ( !eatCWS() ) return false;
    if ( atEnd() ) return false;

    if ( *mCursor != '[' ) { // maybe lone string...
      QString tmp;
      if ( !parseString( tmp ) )
	return false;
      result.push_back( tmp );
      return true;
    } else /* if ( *mCursor == '[' ) */ { // really a string list
      ++mCursor; // eat '['
      while ( !atEnd() ) {
	if ( !eatCWS() ) return false;
	if ( atEnd() ) {
	  makeError( Error::PrematureEndOfStringList );
	  return false;
	}
	QString string;
	if ( !parseString( string ) ) {
	  if ( !error() )
	    makeError( Error::NonStringInStringList );
	  return false;
	}
	result.push_back( string );
	if ( !eatCWS() ) return false;
	if ( *mCursor == ',' ) {
	  ++mCursor;
	  continue;
	} else if ( *mCursor == ']' ) {
	  ++mCursor;
	  return true;
	} else {
	  makeError( Error::NonStringInStringList );
	  return false;
	}
      }
      makeError( Error::PrematureEndOfStringList );
    }
    return false;
  }

  bool Parser::parseArgument( Argument & result ) {
    // argument := string-list / number / tag

    if ( !eatCWS() ) return false;
    if ( atEnd() ) return false;

    if ( isdigit( *mCursor ) ) { // number
      unsigned int number = 0;
      char quantifier = '\0';
      if ( !parseNumber( number, quantifier ) )
	return false;
      result.setNumber( number, quantifier );
      return true;
    } else if ( *mCursor == ':' ) { // tag
      QString tag;
      ++mCursor;
      if ( !parseTag( tag ) )
	return false;
      result.setTag( tag );
      return true;
    } else { // maybe string-list
      QStringList list;
      if ( !parseStringList( list ) )
	return false;
      result.setStringList( list );
      return true;
    }
  }

  bool Parser::parseArgumentList( Argument::List & result ) {
    // our ABNF:
    // argument-list := 1*argument

    result.clear();

    do {
      if ( !eatCWS() ) return false;
      if ( atEnd() ) return !result.empty();

      Argument arg;
      if ( !parseArgument( arg ) ) {
	if ( error() )
	  return false;
	return !result.empty();
      }
      result.push_back( arg );
    } while ( !atEnd() );

    return !result.empty();
  }

  bool Parser::parseTest( Test & result ) {
    // test := identifier arguments
    // arguments := *argument [ test / test-list ]

    //
    // identifier
    //

    if ( !eatCWS() ) return false;
    if ( atEnd() ) return false;

    QString identifier;
    if ( !parseIdentifier( identifier ) )
      return false;

    result.setIdentifier( identifier );

    //
    // *argument
    //

    if ( !eatCWS() ) return false;
    if ( atEnd() ) return true; // a test w/o args

    Argument::List arguments;
    if ( !parseArgumentList( arguments ) ) {
      if ( error() ) return false;
      // else: just didn't find one - that's perfectly OK!
    }

    result.setArgumentList( arguments );

    //
    // test / test-list
    //

    if ( !eatCWS() ) return false;
    if ( atEnd() ) return true; // a test w/o nested tests

    Test::List tests;
    if ( *mCursor == '(' ) { // test-list
      if ( !parseTestList( tests ) )
	return false; // it's an error since we saw '('
    } else {
      Test test;
      if ( !parseTest( test ) ) {
	if ( error() ) return false;
      } else {
	tests.push_back( test );
      }
    }

    result.setTestList( tests );
    return true;
  }


  bool Parser::parseTestList( Test::List & result ) {
    // test-list := "(" test *("," test) ")"

    if ( !eatCWS() ) return false;
    if ( atEnd() ) return false;

    if ( *mCursor != '(' ) return false;

    do {
      if ( !eatCWS() ) return false;
      if ( atEnd() ) {
	makeError( Error::PrematureEndOfTestList );
	return false;
      }

      Test test;
      if ( !parseTest( test ) ) {
	if ( !error() )
	  makeError( Error::NonTestInTestList );
	return false;
      }
      result.push_back( test );

      if ( !eatCWS() ) return false;
      if ( atEnd() ) {
	makeError( Error::PrematureEndOfTestList );
	return false;
      }

      if ( *mCursor == ',' ) {
	++mCursor;
	continue;
      } else if ( *mCursor == ')' ) {
	++mCursor;
	return true;
      } else {
	makeError( Error::NonTestInTestList );
	return false;
      }
    } while ( !atEnd() );
    makeError( Error::PrematureEndOfTestList );
    return false;
  }

  bool Parser::parseCommand( Command & result ) {
    // command   := identifier arguments ( ";" / block )
    // arguments := *argument [ test / test-list ]
    // block     := "{" *command "}"
    // our ABNF:
    // block     := "{" [ command-list ] "}"

    //
    // identifier
    //

    if ( !eatCWS() ) return false;
    if ( atEnd() ) return false;

    QString identifier;
    if ( !parseIdentifier( identifier ) )
      return false;

    result.setIdentifier( identifier );

    //
    // *argument
    //

    if ( !eatCWS() ) return false;
    if ( atEnd() ) {
      makeError( Error::MissingSemicolonOrBlock );
      return false;
    }

    Argument::List arguments;
    if ( !parseArgumentList( arguments ) ) {
      if ( error() ) return false;
      // else: just didn't find one - that's perfectly OK!
    }

    result.setArgumentList( arguments );

    //
    // test / test-list
    //

    if ( !eatCWS() ) return false;
    if ( atEnd() ) {
      makeError( Error::MissingSemicolonOrBlock );
      return false;
    }

    Test::List tests;
    if ( *mCursor == '(' ) { // test-list
      if ( !parseTestList( tests ) )
	return false; // it's an error since we saw '('
    } else {
      Test test;
      if ( !parseTest( test ) ) {
	if ( error() ) return false;
	// else: just didn't find one - that's perfectly OK!
      } else {
	tests.push_back( test );
      }
    }

    result.setTestList( tests );

    //
    // ";" / block
    //

    if ( !eatCWS() ) return false;
    if ( atEnd() ) {
      makeError( Error::MissingSemicolonOrBlock );
      return false;
    }

    if ( *mCursor == ';' ) {
      ++mCursor; // eat ';'
      return true;
    } else if ( *mCursor == '{' ) { // block
      Command::List commands;
      if ( !parseBlock( commands ) )
	return false; // it's an error since we saw '{'

      result.setBlock( commands );
      return true;
    } else {
      makeError( Error::MissingSemicolonOrBlock );
      return false;
    }
    // make compiler happy. This return will actually never be taken,
    // see the if-elsif-else clauses above.
    assert( 0 );
    return false;
    // end make compiler happy
  }

  bool Parser::parseCommandList( Command::List & result ) {
    // our ABNF:
    // command-list := 1*comand

    result.clear();

    do {
      if ( !eatCWS() ) return false;
      if ( atEnd() ) return !result.empty();

      Command cmd;
      if ( !parseCommand( cmd ) ) {
	if ( error() )
	  return false;
	return !result.empty();
      }
      result.push_back( cmd );
    } while ( !atEnd() );

    return !result.empty();
  }

  bool Parser::parseBlock( Command::List & result ) {
    // our ABNF:
    // block := "{" [ command-list ] "}"

    result.clear();

    if ( !eatCWS() ) return false;
    if ( atEnd() ) return false;
    if ( *mCursor != '{' ) return false;

    if ( !parseCommandList( result ) ) {
      if ( error() ) return false;
      // else: there just weren't any - that's perfectly OK!
    }

    if ( !eatCWS() ) return false;
    if ( atEnd() ) {
      makeError( Error::PrematureEndOfBlock );
      return false;
    }
    if ( *mCursor == '}' ) {
      ++mCursor;
      return true;
    } else {
      makeError( Error::PrematureEndOfBlock );
      return false;
    }
  }

      

}; // namespace KSieve
