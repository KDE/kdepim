/*  -*- c++ -*-
    ksieve_parsing.h

    KSieve, the KDE internet mail/usenet news message filtering library.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef __KSIEVE_PARSING_H__
#define __KSIEVE_PARSING_H__

#include "ksieve_lexer.h"

#include "ksieve_argument.h" // can't forward decl. nested classes :-((
#include "ksieve_test.h"
#include "ksieve_command.h"

class QStringList;

namespace KSieve {

  /** @short Parser for the Sieve grammar.
      @author Marc Mutz <mutz@kde.org>
  **/
  class Parser : public Lexer {
  public:
    typedef Lexer base;

    Parser( const char * scursor, const char * const send )
      : Lexer( scursor, send ) {}

    bool parseString( QString & result );

    bool parseStringList( QStringList & result );

    bool parseArgument( KSieve::Argument & result );

    bool parseArgumentList( KSieve::Argument::List & result );

    bool parseTest( KSieve::Test & result );

    bool parseTestList( KSieve::Test::List & result );

    bool parseCommand( KSieve::Command & result );

    bool parseCommandList( KSieve::Command::List & result );

    bool parseBlock( KSieve::Command::List & result );
  };

}; // namespace KSieve

#endif // __KSIEVE_PARSING_H__
