/*  -*- c++ -*-
    ksieve/error.h

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

#ifndef __KSIEVE_ERROR_H__
#define __KSIEVE_ERROR_H__

#include <qstring.h>

#include <kdepimmacros.h>

#ifdef None // X headers
#  undef None
#endif

namespace KSieve {

  class KDE_EXPORT Error {
  public:
    enum Type {
      None = 0,
      Custom,
      // parse (well-formedness in XML speak) errors:
      FirstParseError,

      CRWithoutLF = FirstParseError,
      SlashWithoutAsterisk,
      IllegalCharacter,
      UnexpectedCharacter,
      NoLeadingDigits,
      NonCWSAfterTextColon,

      NumberOutOfRange,
      InvalidUTF8,

      UnfinishedBracketComment,
      PrematureEndOfMultiLine,
      PrematureEndOfQuotedString,
      PrematureEndOfStringList,
      PrematureEndOfTestList,
      PrematureEndOfBlock,
      MissingWhitespace,
      MissingSemicolonOrBlock,

      ExpectedBlockOrSemicolon,
      ExpectedCommand,
      ConsecutiveCommasInStringList,
      ConsecutiveCommasInTestList,
      MissingCommaInTestList,
      MissingCommaInStringList,
      NonStringInStringList,
      NonCommandInCommandList,
      NonTestInTestList,
      LastParseError = NonTestInTestList,
      // validity errors:
      FirstValidityError,
      RequireNotFirst = FirstValidityError, // rfc3028, 3.2
      RequireMissingForCommand,
      RequireMissingForTest,
      RequireMissingForComparator,
      UnsupportedCommand,
      UnsupportedTest,
      UnsupportedComparator,
      TestNestingTooDeep,  // site policy
      BlockNestingTooDeep, // site policy
      InvalidArgument,
      ConflictingArguments, // e.g. rfc3028, 2.7.{1,3}
      ArgumentsRepeated, // similar to ConflictingArguments, e.g. :is :is
      CommandOrderingConstraintViolation, // e.g. else w/o if, rfc3028, 3.1
      LastValidityError = CommandOrderingConstraintViolation,
      // runtime errors:
      FirstRuntimeError,
      IncompatibleActionsRequested = FirstRuntimeError,
      MailLoopDetected,
      TooManyActions,
      LastRuntimeError = TooManyActions
    };

    static const char * typeToString( Type type );

    Error( Type type=None,
	   const QString & s1=QString::null, const QString & s2=QString::null,
	   int line=-1, int col=-1 )
      : mType( type ), mLine( line ), mCol( col ),
        mStringOne( s1 ), mStringTwo( s2 ) {}
    Error( Type type, int line, int col )
      : mType( type ), mLine( line ), mCol( col ) {}

    QString asString() const;

    /** So you can write <pre>if( error() )</pre> with e.g. @ref Lexer */
    operator bool() const {
      return type() != None;
    }

    Type type() const { return mType; }
    int line() const { return mLine; }
    int column() const { return mCol; }
    QString firstString() const { return mStringOne; }
    QString secondString() const { return mStringTwo; }

  protected:
    Type mType;
    int mLine;
    int mCol;
    QString mStringOne, mStringTwo;
  };

} // namespace KSieve

#endif // __KSIEVE_ERROR_H__
