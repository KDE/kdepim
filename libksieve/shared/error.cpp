/*  -*- c++ -*-
    error.cpp

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

#include <ksieve/error.h>

#include <klocale.h> // i18n

#include <climits> // UINT_MAX

namespace KSieve {

  const char * Error::typeToString( Type t ) {
    switch ( t ) {
#define CASE(x) case x: return #x
      CASE( None );
      CASE( Custom );

      CASE( CRWithoutLF );
      CASE( SlashWithoutAsterisk );
      CASE( IllegalCharacter );
      CASE( UnexpectedCharacter );
      CASE( NoLeadingDigits );
      CASE( NonCWSAfterTextColon );

      CASE( NumberOutOfRange );
      CASE( InvalidUTF8 );

      CASE( UnfinishedBracketComment );
      CASE( PrematureEndOfMultiLine );
      CASE( PrematureEndOfQuotedString );
      CASE( PrematureEndOfStringList );
      CASE( PrematureEndOfTestList );
      CASE( PrematureEndOfBlock );
      CASE( MissingWhitespace );
      CASE( MissingSemicolonOrBlock );

      CASE( ExpectedBlockOrSemicolon );
      CASE( ExpectedCommand );
      CASE( ConsecutiveCommasInStringList );
      CASE( ConsecutiveCommasInTestList );
      CASE( MissingCommaInTestList );
      CASE( MissingCommaInStringList );
      CASE( NonStringInStringList );
      CASE( NonCommandInCommandList );
      CASE( NonTestInTestList );

      CASE( RequireNotFirst );
      CASE( RequireMissingForCommand );
      CASE( RequireMissingForTest );
      CASE( RequireMissingForComparator );
      CASE( UnsupportedCommand );
      CASE( UnsupportedTest );
      CASE( UnsupportedComparator );
      CASE( TestNestingTooDeep );
      CASE( BlockNestingTooDeep );
      CASE( InvalidArgument );
      CASE( ConflictingArguments );
      CASE( ArgumentsRepeated );
      CASE( CommandOrderingConstraintViolation );

      CASE( IncompatibleActionsRequested );
      CASE( MailLoopDetected );
      CASE( TooManyActions );
#undef CASE
    default:
      return "<unknown>";
    }
  }

  QString Error::asString() const {

    QString err;
    switch( type() ) {
    case None:
      return QString::null;
    case Custom:
      return mStringOne;

      // Parse errors:
    case CRWithoutLF:
      err = i18n("Parse error: Carriage Return (CR) without Line Feed (LF)");
      break;
    case SlashWithoutAsterisk:
      err = i18n("Parse error: Unquoted Slash ('/') without Asterisk ('*')."
		 "Broken Comment?");
      break;
    case IllegalCharacter:
      err = i18n("Parse error: Illegal Character");
      break;
    case UnexpectedCharacter:
      err = i18n("Parse error: Unexpected Character, probably a missing space?");
      break;
    case NoLeadingDigits:
      err = i18n("Parse error: Tag Name begins in Digits");
      break;
    case NonCWSAfterTextColon:
      err = i18n("Parse error: Only whitespace and #comments may "
		 "follow \"text:\" on the same line");
      break;
    case NumberOutOfRange:
      err = i18n("Parse error: Number out of Range (must be smaller than %1)").arg(UINT_MAX);
      break;
    case InvalidUTF8:
      err = i18n("Parse error: Invalid UTF-8 sequence");
      break;
    case PrematureEndOfMultiLine:
      err = i18n("Parse error: Premature end of Multiline String (did you forget the '.'?)");
      break;
    case PrematureEndOfQuotedString:
      err = i18n("Parse error: Premature end of Quoted String (missing closing '\"')");
      break;
    case PrematureEndOfStringList:
      err = i18n("Parse error: Premature end of String List (missing closing ']')");
      break;
    case PrematureEndOfTestList:
      err = i18n("Parse error: Premature end of Test List (missing closing ')')");
      break;
    case PrematureEndOfBlock:
      err = i18n("Parse error: Premature end of Block (missing closing '}')");
      break;
    case MissingWhitespace:
      err = i18n("Parse error: Missing Whitespace");
      break;
    case MissingSemicolonOrBlock:
      err = i18n("Parse error: Missing ';' or Block");
      break;
    case ExpectedBlockOrSemicolon:
      err = i18n("Parse error: Expected ';' or '{', got something else");
      break;
    case ExpectedCommand:
      err = i18n("Parse error: Expected Command, got something else");
      break;
    case ConsecutiveCommasInStringList:
      err = i18n("Parse error: Trailing, Leading or Duplicate Commas in String List");
      break;
    case ConsecutiveCommasInTestList:
      err = i18n("Parse error: Trailing, Leading or Duplicate Commas in Test List");
      break;
    case MissingCommaInStringList:
      err = i18n("Parse error: Missing ',' between Strings in String List");
      break;
    case MissingCommaInTestList:
      err = i18n("Parse error: Missing ',' between Tests in Test List");
      break;
    case NonCommandInCommandList:
      err = i18n("Parse error: Expected Command, got something else");
      break;
    case NonStringInStringList:
      err = i18n("Parse error: Only Strings allowed in String Lists");
      break;
    case NonTestInTestList:
      err = i18n("Parse error: Only Tests allowed in Test Lists");
      break;

      // validity errors:
    case RequireNotFirst:
      err = i18n("\"require\" must be first command");
      break;
    case RequireMissingForCommand:
      err = i18n("\"require\" missing for command \"%1\"").arg(mStringOne);
      break;
    case RequireMissingForTest:
      err = i18n("\"require\" missing for test \"%1\"").arg(mStringOne);
      break;
    case RequireMissingForComparator:
      err = i18n("\"require\" missing for comparator \"%1\"").arg(mStringOne);
      break;
    case UnsupportedCommand:
      err = i18n("Command \"%1\" not supported").arg(mStringOne);
      break;
    case UnsupportedTest:
      err = i18n("Test \"%1\" not supported").arg(mStringOne);
      break;
    case UnsupportedComparator:
      err = i18n("Comparator \"%1\" not supported").arg(mStringOne);
      break;
    case TestNestingTooDeep:
      err = i18n("Site Policy Limit Violation: Test nesting too deep (max. %1)").arg( mStringOne.toUInt() );
      break;
    case BlockNestingTooDeep:
      err = i18n("Site Policy Limit Violation: Block nesting too deep (max. %1)").arg( mStringOne.toUInt() );
      break;
    case InvalidArgument:
      err = i18n("Invalid Argument \"%1\" to \"%2\"").arg(mStringOne).arg(mStringTwo);
      break;
    case ConflictingArguments:
      err = i18n("Conflicting Arguments: \"%1\" and \"%2\"").arg(mStringOne).arg(mStringTwo);
      break;
    case ArgumentsRepeated:
      err = i18n("Argument \"%1\" Repeated").arg(mStringOne);
      break;
    case CommandOrderingConstraintViolation:
      err = i18n("Command \"%1\" violates command ordering constraints").arg(mStringOne);
      break;

      // runtime errors:
    case IncompatibleActionsRequested:
      err = i18n("Incompatible Actions \"%1\" and \"%2\" requested").arg(mStringOne).arg(mStringTwo);
      break;
    case MailLoopDetected:
      err = i18n("Mail Loop detected");
      break;
    case TooManyActions:
      err = i18n("Site Policy Limit Violation: Too many Actions requested (max. %1)").arg( mStringOne.toUInt() );
      break;
    default:
      err = i18n("Unknown error");
      break;
    }

    return err;
  }
  

} // namespace KSieve

