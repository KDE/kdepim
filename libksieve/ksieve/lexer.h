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

class QString;

namespace KSieve {

  class Error;

  class Lexer {
  public:
    enum Options {
      IncludeComments = 0,
      IgnoreComments = 1,
      IncludeLineFeeds = 0,
      IgnoreLineFeeds = 2
    };

    Lexer( const char * scursor, const char * send, int options=0 );
    ~Lexer();

    /** Return whether comments are returned by @ref
	nextToken. Default is to not ignore comments. Ignoring them
	can speed up script parsing a bit, and can be used when the
	internal representation of the script won't be serialized into
	string form again (or if you simply want to delete all
	comments)
    **/
    bool ignoreComments() const;

    /** Return whether line feeds are returned by @ref
	nextToken. Default is to not ignore line feeds. Ignoring them
	can speed up script parsing a bit, and can be used when the
	internal representation of the script won't be serialized into
	string form again.
    **/
    bool ignoreLineFeeds() const;

    const Error & error() const;

    bool atEnd() const;
    int column() const;
    int line() const;

    enum Token {
      None = 0,
      Number,          // 1, 100, 1M, 10k, 1G, 2g, 3m
      Identifier,      // atom
      Tag,             // :tag
      Special,         // {} [] () ,;
      QuotedString,    // "foo\"bar" -> foo"bar
      MultiLineString, // text: \nfoo\n. -> foo
      HashComment,     // # foo
      BracketComment,  // /* foo */
      LineFeeds        // the number of line feeds encountered
    };

    /** Parse the next token and return it's type. @p result will contain
	the value of the token. */
    Token nextToken( QString & result );

    void save();
    void restore();
      
    class Impl;
  private:
    Impl * i;

  private:
    const Lexer & operator=( const Lexer & );
    Lexer( const Lexer & );
  };

} // namespace KSieve

#endif // __KSIEVE_LEXER_H__
