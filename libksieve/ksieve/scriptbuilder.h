/*  -*- c++ -*-
    ksieve/interfaces/scriptbuilder.h

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

#ifndef __KSIEVE_INTERFACES_SCRIPTBUILDER_H__
#define __KSIEVE_INTERFACES_SCRIPTBUILDER_H__

class QString;

namespace KSieve {

  class Error;

  class ScriptBuilder {
  public:
    virtual ~ScriptBuilder() {}

    virtual void taggedArgument( const QString & tag ) = 0;
    virtual void stringArgument( const QString & string, bool multiLine, const QString & embeddedHashComment ) = 0;
    virtual void numberArgument( unsigned long number, char quantifier ) = 0;

    virtual void stringListArgumentStart() = 0;
    virtual void stringListEntry( const QString & string, bool multiLine, const QString & embeddedHashComment ) = 0;
    virtual void stringListArgumentEnd() = 0;

    virtual void commandStart( const QString & identifier ) = 0;
    virtual void commandEnd() = 0;

    virtual void testStart( const QString & identifier ) = 0;
    virtual void testEnd() = 0;

    virtual void testListStart() = 0;
    virtual void testListEnd() = 0;

    virtual void blockStart() = 0;
    virtual void blockEnd() = 0;

    /** A hash comment always includes an implicit lineFeed() at it's end. */
    virtual void hashComment( const QString & comment ) = 0;
    /** Bracket comments inclde explicit lineFeed()s in their content */
    virtual void bracketComment( const QString & comment ) = 0;

    virtual void lineFeed() = 0;

    virtual void error( const Error & error ) = 0;

    virtual void finished() = 0;
  };

} // namespace KSieve

#endif // __KSIEVE_INTERFACES_SCRIPTBUILDER_H__
