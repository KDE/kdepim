/*  -*- c++ -*-
    ksieve_error.h

    KSieve, the KDE internet mail/usenet news message filtering library.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef __KSIEVE_ERROR_H__
#define __KSIEVE_ERROR_H__

#include <qstring.h>

#ifdef None // X headers
#  undef None
#endif

namespace KSieve {

  class Error {
  public:
    enum Type {
      None = 0,
      Custom,
      // parse (well-formedness in XML speak) errors:
      FirstParseError,

      CRWithoutLF = FirstParseError,
      IllegalCharacter,

      NumberOutOfRange,

      UnfinishedBracketComment,
      PrematureEndOfMultiLine,
      PrematureEndOfQuotedString,
      PrematureEndOfStringList,
      PrematureEndOfTestList,
      PrematureEndOfBlock,
      MissingSemicolonOrBlock,

      NonStringInStringList,
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

}; // namespace KSieve

#endif // __KSIEVE_ERROR_H__
