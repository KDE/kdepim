/*  -*- c++ -*-
    ksieve/parser.h

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

#ifndef __KSIEVE_PARSING_H__
#define __KSIEVE_PARSING_H__

#include <kdepimmacros.h>

class QString;

namespace KSieve {

  class ScriptBuilder;
  class Error;

  /** @short Parser for the Sieve grammar.
      @author Marc Mutz <mutz@kde.org>
  **/
  class KDE_EXPORT Parser {
  public:

    Parser( const char * scursor, const char * const send, int options=0 );
    ~Parser();

    void setScriptBuilder( ScriptBuilder * builder );
    ScriptBuilder * scriptBuilder() const;

    bool parse();

    const Error & error() const;

    class Impl;
  private:
    Impl * i;

  private:
    const Parser & operator=( const Parser & );
    Parser( const Parser & );
  };

} // namespace KSieve

#endif // __KSIEVE_PARSING_H__
