/*  -*- c++ -*-
    impl/parser.h

    Internal header file. Subject to change without notice. DO NOT USE.

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

#ifndef __KSIEVE_IMPL_PARSER_H__
#define __KSIEVE_IMPL_PARSER_H__

#include <ksieve/parser.h>

#include <ksieve/error.h>
#include <ksieve/lexer.h>
#include <impl/lexer.h>

#include <ksieve/scriptbuilder.h>


namespace KSieve {

  class Parser::Impl {
    friend class Parser;
  private:
    Impl( const char * scursor, const char * const send );

    void setScriptBuilder( ScriptBuilder * builder ) {
      mBuilder = builder;
    }
    ScriptBuilder * scriptBuilder() const {
      return mBuilder;
    }

    bool parse();

    const Error & error() const { return mError == Error::None ? lexer.error() : mError ; }

    bool parseCommandList();

    bool parseCommand();

    bool parseArgumentList();

    bool parseArgument();

    bool parseTestList();

    bool parseTest();

    bool parseBlock();

    bool parseStringList();

    bool parseNumber();


    Lexer::Token token() const { return mToken; }
    QString tokenValue() const { return mTokenValue; }

    bool atEnd() const {
      return !mToken && lexer.atEnd() ;
    }
    bool obtainToken();
    void consumeToken() {
      mToken = Lexer::None;
      mTokenValue = QString::null;
    }
    void makeError( Error::Type e, int line, int col ) {
      mError = Error( e, line, col );
      if ( scriptBuilder() )
	scriptBuilder()->error( mError );
    }
    void makeError( Error::Type e ) {
      makeError( e, lexer.line(), lexer.column() );
    }
    void makeUnexpectedTokenError( Error::Type e ) {
      makeError( e ); // ### save wrong token...
    }
    bool isArgumentToken() const;
    bool isStringToken() const;

    Error mError;
    Lexer::Token mToken;
    QString mTokenValue;
    Lexer::Impl lexer;
    ScriptBuilder * mBuilder;
  };

}

#endif // __KSIEVE_IMPL_PARSER_H__
