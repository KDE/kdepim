/*  -*- mode: C++; c-file-style: "gnu" -*-

    This file is part of kdepim.
    Copyright (c) 2004 KDEPIM developers

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "email.h"
#include <kdebug.h>
#include <klocale.h>
#include <qregexp.h>

//-----------------------------------------------------------------------------
QStringList KPIM::splitEmailAddrList(const QString& aStr)
{
  // Features:
  // - always ignores quoted characters
  // - ignores everything (including parentheses and commas)
  //   inside quoted strings
  // - supports nested comments
  // - ignores everything (including double quotes and commas)
  //   inside comments

  QStringList list;

  if (aStr.isEmpty())
    return list;

  QString addr;
  uint addrstart = 0;
  int commentlevel = 0;
  bool insidequote = false;

  for (uint index=0; index<aStr.length(); index++) {
    // the following conversion to latin1 is o.k. because
    // we can safely ignore all non-latin1 characters
    switch (aStr[index].latin1()) {
    case '"' : // start or end of quoted string
      if (commentlevel == 0)
        insidequote = !insidequote;
      break;
    case '(' : // start of comment
      if (!insidequote)
        commentlevel++;
      break;
    case ')' : // end of comment
      if (!insidequote) {
        if (commentlevel > 0)
          commentlevel--;
        else {
          kdDebug(5300) << "Error in address splitting: Unmatched ')'"
                        << endl;
          return list;
        }
      }
      break;
    case '\\' : // quoted character
      index++; // ignore the quoted character
      break;
    case ',' :
      if (!insidequote && (commentlevel == 0)) {
        addr = aStr.mid(addrstart, index-addrstart);
        if (!addr.isEmpty())
          list += addr.simplifyWhiteSpace();
        addrstart = index+1;
      }
      break;
    }
  }
  // append the last address to the list
  if (!insidequote && (commentlevel == 0)) {
    addr = aStr.mid(addrstart, aStr.length()-addrstart);
    if (!addr.isEmpty())
      list += addr.simplifyWhiteSpace();
  }
  else
    kdDebug(5300) << "Error in address splitting: "
                  << "Unexpected end of address list"
                  << endl;

  return list;
}

//-----------------------------------------------------------------------------
KPIM::EmailParseResult KPIM::isValidEmailAddress( const QString& aStr )
{
  // If we are passed an empty string bail right away no need to process further 
  // and waste resources
  if ( aStr.isEmpty() ) {
    return AddressEmpty;
  }

  // count how many @'s are in the string that is passed to us
  // if 0 or > 1 take action
  // at this point to many @'s cannot bail out right away since
  // @ is allowed in qoutes, so we use a bool to keep track
  // and then make a judgement further down in the parser
  // FIXME count only @ not in double quotes

  bool tooManyAtsFlag = false;

  int atCount = aStr.contains('@');
  if ( atCount > 1 ) {
    tooManyAtsFlag = true;;
  } else if ( atCount == 0 ) {
	  return TooFewAts;
  }

  // The main parser, try and catch all weird and wonderful
  // mistakes users and/or machines can create

  enum { TopLevel, InComment, InAngleAddress } context = TopLevel;
  bool inQuotedString = false;
  int commentLevel = 0;

  unsigned int strlen = aStr.length();

  for ( unsigned int index=0; index < strlen; index++ ) {
    switch ( context ) {
    case TopLevel : {
      switch ( aStr[index].latin1() ) {
        case '"' : inQuotedString = !inQuotedString; 
          break;
        case '(' : 
          if ( !inQuotedString ) {
            context = InComment;
            commentLevel = 1;
          }
          break;
        case '<' : 
          if ( !inQuotedString ) {
            context = InAngleAddress;
          }
          break;
        case '\\' : // quoted character
          ++index; // skip the '\'
          if ( ++index > strlen )
            return UnexpectedEnd;
          break;
        case ',' : 
          if ( !inQuotedString )
            return UnexpectedComma;
          break;
        case ')' : 
          if ( !inQuotedString )
            return UnbalancedParens;
          break;
        case '>' : 
          if ( !inQuotedString )
            return UnopenedAngleAddr;
          break;
        case '@' : 
          if ( !inQuotedString ) {
            if ( index == 0 ) {  // Missing local part
              return MissingLocalPart;
            } else if( index == strlen-1 ) {
              return MissingDomainPart;
              break;
              } 
            } else if ( inQuotedString ) {
              --atCount;
              if ( atCount == 1 ) {
                tooManyAtsFlag = false;
              }
            }
            break;
      } 
      break; 
    }
    case InComment : {
      switch ( aStr[index] ) {
        case '(' : ++commentLevel;
          break;
        case ')' : --commentLevel;
          if ( commentLevel == 0 ) {
            context = TopLevel;
          }
          break;
        case '\\' : // quoted character
          ++index; // skip the '\'
          if ( ++index > strlen )
            return UnexpectedEnd;
            break;
        }
        break;
    }

    case InAngleAddress : {
      switch ( aStr[index] ) {
        case '"' : inQuotedString = !inQuotedString;
            break;
        case '@' :
          if ( inQuotedString ) {
            --atCount;
            if ( atCount == 1 ) {
              tooManyAtsFlag = false;
            }
          }
          break;
        case '>' : 
          if ( !inQuotedString ) {
            context = TopLevel;
            break;
          }
          break;
        case '\\' : // quoted character
          ++index; // skip the '\'
          if ( ++index > strlen )
            return UnexpectedEnd;
          break;
          }
        break;
      }
    }
  }
  if ( context == InComment )
    return UnbalancedParens;
 
  if ( context == TopLevel && !tooManyAtsFlag ) {
    return AddressOk;
  }

  if ( context == InAngleAddress )
    return UnclosedAngleAddr;

  if ( tooManyAtsFlag ) {
    return TooManyAts;
  }
} 

//-----------------------------------------------------------------------------
QString KPIM::emailParseResultToString( EmailParseResult errorCode )
{
  switch ( errorCode ) {
    case TooManyAts : 
      return i18n("The email address you entered is not valid because it "
                "contains more than one @ "
                "you will not create valid messages if you do not "
                "change your address.");
    case TooFewAts : 
      return i18n("The email address you entered is not valid because it "
                "does not contain a @ "
                "you will not create valid messages if you do not "
                "change your address.");
    case AddressEmpty : 
      return i18n("You have to enter something in the email address field.");
    case MissingLocalPart : 
      return i18n("The email address you entered is not valid because it "
                "does not contain a local part.");
    case MissingDomainPart : 
      return i18n("The email address you entered is not valid because it "
                "does not contain a domain part.");
    case UnbalancedParens : 
      return i18n("The email address you entered is not valid because it "
                "contains unclosed comments/brackets.");
    case AddressOk : 
      return i18n("The email address you entered is valid.");
    case UnclosedAngleAddr : 
      return i18n("The email address you entered is not valid because it "
                "contains an unclosed anglebracket.");
    case UnopenedAngleAddr : 
      return i18n("The email address you entered is not valid because it "
                "contains an unopened anglebracket.");
    case UnexpectedComma :
      return i18n("The email address you have entered is not valid because it "
                "contains an unexpected comma.");
    case UnexpectedEnd : 
      return i18n("The email address you entered is not valid because it ended "
                "unexpectadly, this probably means you have used an escaping type "
                "character like an \\  as the last character in your email "
                "address.");
  }
  return i18n("Unknown problem with email address");
}

//-----------------------------------------------------------------------------
bool KPIM::isValidSimpleEmailAddress( const QString& aStr )
{
  // If we are passed an empty string bail right away no need to process further�
  // and waste resources
  if ( aStr.isEmpty() ) {
    return false;
  }

  int atChar = aStr.findRev( '@' );
  QString domainPart = aStr.mid( atChar + 1);
  QString localPart = aStr.left( atChar );
  bool tooManyAtsFlag = false;
  bool inQuotedString = false;
  int atCount = localPart.contains( '@' );

  unsigned int strlen = localPart.length();
  for ( unsigned int index=0; index < strlen; index++ ) {
    switch( localPart[ index ].latin1() ) {
      case '"' : inQuotedString = !inQuotedString;
        break;
      case '@' :
        if ( inQuotedString ) {
          --atCount;
          if ( atCount == 0 ) {
            tooManyAtsFlag = false;
          }
        }
        break;
      }
  }

  QString addrRx = "[a-zA-Z]*[\\w.-]*[a-zA-Z0-9]@";
  if ( localPart[ 0 ] == '\"' || localPart[ localPart.length()-1 ] == '\"' ) {
    addrRx = "\"[a-zA-Z@]*[\\w.@-]*[a-zA-Z0-9@]\"@";
  }
  if ( domainPart[ 0 ] == '[' || domainPart[ domainPart.length()-1 ] == ']' ) {
    addrRx += "\\[[0-9]{,3}(\\.[0-9]{,3}){3}\\]";
  } else {
    addrRx += "[\\w-]+(\\.[\\w-]+)";
  }
  QRegExp rx( addrRx );
  return  rx.exactMatch( aStr ) && !tooManyAtsFlag;
}

//-----------------------------------------------------------------------------
QCString KPIM::getEmailAddr(const QString& aStr)
{
  int a, i, j, len, found = 0;
  QChar c;
  // Find the '@' in the email address:
  a = aStr.find('@');
  if (a<0) return aStr.latin1();
  // Loop backwards until we find '<', '(', ' ', or beginning of string.
  for (i = a - 1; i >= 0; i--) {
    c = aStr[i];
    if (c == '<' || c == '(' || c == ' ') found = 1;
    if (found) break;
  }
  // Reset found for next loop.
  found = 0;
  // Loop forwards until we find '>', ')', ' ', or end of string.
  for (j = a + 1; j < (int)aStr.length(); j++) {
    c = aStr[j];
    if (c == '>' || c == ')' || c == ' ') found = 1;
    if (found) break;
  }
  // Calculate the length and return the result.
  len = j - (i + 1);
  return aStr.mid(i+1,len).latin1();
}

bool KPIM::getNameAndMail(const QString& aStr, QString& name, QString& mail)
{
  name = QString::null;
  mail = QString::null;

  const int len=aStr.length();
  const char cQuotes = '"';

  bool bInComment, bInQuotesOutsideOfEmail;
  int i=0, iAd=0, iMailStart=0, iMailEnd=0;
  QChar c;

  // Find the '@' of the email address
  // skipping all '@' inside "(...)" comments:
  bInComment = false;
  while( i < len ){
    c = aStr[i];
    if( !bInComment ){
      if( '(' == c ){
        bInComment = true;
      }else{
        if( '@' == c ){
          iAd = i;
          break; // found it
        }
      }
    }else{
      if( ')' == c ){
        bInComment = false;
      }
    }
    ++i;
  }

  if( !iAd ){
    // We suppose the user is typing the string manually and just
    // has not finished typing the mail address part.
    // So we take everything that's left of the '<' as name and the rest as mail
    for( i = 0; len > i; ++i ) {
      c = aStr[i];
      if( '<' != c )
        name.append( c );
      else
        break;
    }
    mail = aStr.mid( i+1 );
    if ( mail.endsWith( ">" ) )
      mail.truncate( mail.length() - 1 );

  }else{

    // Loop backwards until we find the start of the string
    // or a ',' that is outside of a comment
    //          and outside of quoted text before the leading '<'.
    bInComment = false;
    bInQuotesOutsideOfEmail = false;
    for( i = iAd-1; 0 <= i; --i ) {
      c = aStr[i];
      if( bInComment ){
        if( '(' == c ){
          if( !name.isEmpty() )
            name.prepend( ' ' );
          bInComment = false;
        }else{
          name.prepend( c ); // all comment stuff is part of the name
        }
      }else if( bInQuotesOutsideOfEmail ){
        if( cQuotes == c )
          bInQuotesOutsideOfEmail = false;
        else
          name.prepend( c );
      }else{
        // found the start of this addressee ?
        if( ',' == c )
          break;
        // stuff is before the leading '<' ?
        if( iMailStart ){
          if( cQuotes == c )
            bInQuotesOutsideOfEmail = true; // end of quoted text found
          else
            name.prepend( c );
        }else{
          switch( c ){
            case '<':
              iMailStart = i;
              break;
            case ')':
              if( !name.isEmpty() )
                name.prepend( ' ' );
              bInComment = true;
              break;
            default:
              if( ' ' != c )
                mail.prepend( c );
          }
        }
      }
    }

    name = name.simplifyWhiteSpace();
    mail = mail.simplifyWhiteSpace();

    if( mail.isEmpty() )
      return false;

    mail.append('@');

    // Loop forward until we find the end of the string
    // or a ',' that is outside of a comment
    //          and outside of quoted text behind the trailing '>'.
    bInComment = false;
    bInQuotesOutsideOfEmail = false;
    for( i = iAd+1; len > i; ++i ) {
      c = aStr[i];
      if( bInComment ){
        if( ')' == c ){
          if( !name.isEmpty() )
            name.append( ' ' );
          bInComment = false;
        }else{
          name.append( c ); // all comment stuff is part of the name
        }
      }else if( bInQuotesOutsideOfEmail ){
        if( cQuotes == c )
          bInQuotesOutsideOfEmail = false;
        else
          name.append( c );
      }else{
        // found the end of this addressee ?
        if( ',' == c )
          break;
        // stuff is behind the trailing '>' ?
        if( iMailEnd ){
          if( cQuotes == c )
            bInQuotesOutsideOfEmail = true; // start of quoted text found
          else
            name.append( c );
        }else{
          switch( c ){
            case '>':
              iMailEnd = i;
              break;
            case '(':
              if( !name.isEmpty() )
                name.append( ' ' );
              bInComment = true;
              break;
            default:
              if( ' ' != c )
                mail.append( c );
          }
        }
      }
    }
  }

  name = name.simplifyWhiteSpace();
  mail = mail.simplifyWhiteSpace();

  return ! (name.isEmpty() || mail.isEmpty());
}

bool KPIM::compareEmail( const QString& email1, const QString& email2,
                         bool matchName )
{
  QString e1Name, e1Email, e2Name, e2Email;

  getNameAndMail( email1, e1Name, e1Email );
  getNameAndMail( email2, e2Name, e2Email );

  return e1Email == e2Email &&
    ( !matchName || ( e1Name == e2Name ) );
}
