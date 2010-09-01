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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "email.h"

#include <kdebug.h>
#include <klocale.h>
#include <kidna.h>
#include <kmime_util.h>

#include <tqregexp.h>

//-----------------------------------------------------------------------------
TQStringList KPIM::splitEmailAddrList(const TQString& aStr)
{
  // Features:
  // - always ignores quoted characters
  // - ignores everything (including parentheses and commas)
  //   inside quoted strings
  // - supports nested comments
  // - ignores everything (including double quotes and commas)
  //   inside comments

  TQStringList list;

  if (aStr.isEmpty())
    return list;

  TQString addr;
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
    case ';' :
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
// Used by KPIM::splitAddress(...) and KPIM::getFirstEmailAddress(...).
KPIM::EmailParseResult splitAddressInternal( const TQCString& address,
                                             TQCString & displayName,
                                             TQCString & addrSpec,
                                             TQCString & comment,
                                             bool allowMultipleAddresses )
{
//  kdDebug() << "KMMessage::splitAddress( " << address << " )" << endl;

  displayName = "";
  addrSpec = "";
  comment = "";
  
  // these strings are later copied to displayName resp. addrSpec resp. comment
  // we don't operate directly on those variables, since as ByteArray deriverates
  // they have a miserable performance on operator+
  TQString dName;
  TQString aSpec;
  TQString cmmt;
  
  if ( address.isEmpty() )
    return KPIM::AddressEmpty;

  // The following is a primitive parser for a mailbox-list (cf. RFC 2822).
  // The purpose is to extract a displayable string from the mailboxes.
  // Comments in the addr-spec are not handled. No error checking is done.

  enum { TopLevel, InComment, InAngleAddress } context = TopLevel;
  bool inQuotedString = false;
  int commentLevel = 0;
  bool stop = false;

  for ( char* p = address.data(); *p && !stop; ++p ) {
    switch ( context ) {
    case TopLevel : {
      switch ( *p ) {
      case '"' : inQuotedString = !inQuotedString;
                 dName += *p;
                 break;
      case '(' : if ( !inQuotedString ) {
                   context = InComment;
                   commentLevel = 1;
                 }
                 else
                   dName += *p;
                 break;
      case '<' : if ( !inQuotedString ) {
                   context = InAngleAddress;
                 }
                 else
                   dName += *p;
                 break;
      case '\\' : // quoted character
                 dName += *p;
                 ++p; // skip the '\'
                 if ( *p )
                   dName += *p;
                 else
                   return KPIM::UnexpectedEnd;
                 break;
          case ',' :
          case ';' : if ( !inQuotedString ) {
                   if ( allowMultipleAddresses )
                     stop = true;
                   else
                     return KPIM::UnexpectedComma;
                 }
                 else
                   dName += *p;
                 break;
      default :  dName += *p;
      }
      break;
    }
    case InComment : {
      switch ( *p ) {
      case '(' : ++commentLevel;
                 cmmt += *p;
                 break;
      case ')' : --commentLevel;
                 if ( commentLevel == 0 ) {
                   context = TopLevel;
                   cmmt += ' '; // separate the text of several comments
                 }
                 else
                   cmmt += *p;
                 break;
      case '\\' : // quoted character
                 cmmt += *p;
                 ++p; // skip the '\'
                 if ( *p )
                   cmmt += *p;
                 else
                   return KPIM::UnexpectedEnd;
                 break;
      default :  cmmt += *p;
      }
      break;
    }
    case InAngleAddress : {
      switch ( *p ) {
      case '"' : inQuotedString = !inQuotedString;
                 aSpec += *p;
                 break;
      case '>' : if ( !inQuotedString ) {
                   context = TopLevel;
                 }
                 else
                   aSpec += *p;
                 break;
      case '\\' : // quoted character
                 aSpec += *p;
                 ++p; // skip the '\'
                 if ( *p )
                   aSpec += *p;
                 else
                   return KPIM::UnexpectedEnd;
                 break;
      default :  aSpec += *p;
      }
      break;
    }
    } // switch ( context )
  }
  // check for errors
  if ( inQuotedString )
    return KPIM::UnbalancedQuote;
  if ( context == InComment )
    return KPIM::UnbalancedParens;
  if ( context == InAngleAddress )
    return KPIM::UnclosedAngleAddr;

	
  displayName = dName.stripWhiteSpace().latin1();
  comment = cmmt.stripWhiteSpace().latin1();
  addrSpec = aSpec.stripWhiteSpace().latin1();

  if ( addrSpec.isEmpty() ) {
    if ( displayName.isEmpty() )
      return KPIM::NoAddressSpec;
    else {
      addrSpec = displayName;
      displayName.truncate( 0 );
    }
  }
/*
  kdDebug() << "display-name : \"" << displayName << "\"" << endl;
  kdDebug() << "comment      : \"" << comment << "\"" << endl;
  kdDebug() << "addr-spec    : \"" << addrSpec << "\"" << endl;
*/
  return KPIM::AddressOk;
}


//-----------------------------------------------------------------------------
KPIM::EmailParseResult KPIM::splitAddress( const TQCString& address,
                                           TQCString & displayName,
                                           TQCString & addrSpec,
                                           TQCString & comment )
{
  return splitAddressInternal( address, displayName, addrSpec, comment,
                               false /* don't allow multiple addresses */ );
}


//-----------------------------------------------------------------------------
KPIM::EmailParseResult KPIM::splitAddress( const TQString & address,
                                           TQString & displayName,
                                           TQString & addrSpec,
                                           TQString & comment )
{
  TQCString d, a, c;
  KPIM::EmailParseResult result = splitAddress( address.utf8(), d, a, c );
  if ( result == AddressOk ) {
    displayName = TQString::fromUtf8( d );
    addrSpec = TQString::fromUtf8( a );
    comment = TQString::fromUtf8( c );
  }
  return result;
}


//-----------------------------------------------------------------------------
KPIM::EmailParseResult KPIM::isValidEmailAddress( const TQString& aStr )
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
        case '[' :
          if ( !inQuotedString ) {
            return InvalidDisplayName;
          }
          break;
        case ']' :
          if ( !inQuotedString ) {
            return InvalidDisplayName;
          }
          break;
        case ':' :
          if ( !inQuotedString ) {
            return DisallowedChar;
          }
          break;
        case '<' :
          if ( !inQuotedString ) {
            context = InAngleAddress;
          }
          break;
        case '\\' : // quoted character
          ++index; // skip the '\'
          if (( index + 1 )> strlen ) {
            return UnexpectedEnd;
          }
          break;
            case ',' :
            case ';' :
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
          if (( index + 1 )> strlen ) {
            return UnexpectedEnd;
          }
          break;
        }
        break;
    }

    case InAngleAddress : {
      switch ( aStr[index] ) {
            case ',' :
            case ';' :
          if ( !inQuotedString ) {
            return UnexpectedComma;
          }
          break;
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
          if (( index + 1 )> strlen ) {
            return UnexpectedEnd;
          }
          break;
        }
        break;
      }
    }
  }

  if ( atCount == 0 && !inQuotedString )
    return TooFewAts;

  if ( inQuotedString )
    return UnbalancedQuote;

  if ( context == InComment )
    return UnbalancedParens;

  if ( context == InAngleAddress )
    return UnclosedAngleAddr;

  if ( tooManyAtsFlag ) {
    return TooManyAts;
  }
  return AddressOk;
}

//-----------------------------------------------------------------------------
TQString KPIM::emailParseResultToString( EmailParseResult errorCode )
{
  switch ( errorCode ) {
    case TooManyAts :
      return i18n("The email address you entered is not valid because it "
                "contains more than one @. "
                "You will not create valid messages if you do not "
                "change your address.");
    case TooFewAts :
      return i18n("The email address you entered is not valid because it "
                "does not contain a @."
                "You will not create valid messages if you do not "
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
                "unexpectedly, this probably means you have used an escaping type "
                "character like an \\  as the last character in your email "
                "address.");
    case UnbalancedQuote :
      return i18n("The email address you entered is not valid because it "
                  "contains quoted text which does not end.");
    case NoAddressSpec :
      return i18n("The email address you entered is not valid because it "
                  "does not seem to contain an actual email address, i.e. "
                  "something of the form joe@kde.org.");
    case DisallowedChar :
      return i18n("The email address you entered is not valid because it "
                  "contains an illegal character.");
    case InvalidDisplayName :
      return i18n("The email address you have entered is not valid because it "
                  "contains an invalid displayname.");
  }
  return i18n("Unknown problem with email address");
}

//-----------------------------------------------------------------------------
bool KPIM::isValidSimpleEmailAddress( const TQString& aStr )
{
  // If we are passed an empty string bail right away no need to process further
  // and waste resources
  if ( aStr.isEmpty() ) {
    return false;
  }

  int atChar = aStr.findRev( '@' );
  TQString domainPart = aStr.mid( atChar + 1);
  TQString localPart = aStr.left( atChar );
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

  TQString addrRx = "[a-zA-Z]*[~|{}`\\^?=/+*'&%$#!_\\w.-]*[~|{}`\\^?=/+*'&%$#!_a-zA-Z0-9-]@";
  if ( localPart[ 0 ] == '\"' || localPart[ localPart.length()-1 ] == '\"' ) {
    addrRx = "\"[a-zA-Z@]*[\\w.@-]*[a-zA-Z0-9@]\"@";
  }
  if ( domainPart[ 0 ] == '[' || domainPart[ domainPart.length()-1 ] == ']' ) {
    addrRx += "\\[[0-9]{,3}(\\.[0-9]{,3}){3}\\]";
  } else {
    addrRx += "[\\w-]+(\\.[\\w-]+)*";
  }
  TQRegExp rx( addrRx );
  return  rx.exactMatch( aStr ) && !tooManyAtsFlag;
}

//-----------------------------------------------------------------------------
TQString KPIM::simpleEmailAddressErrorMsg()
{
      return i18n("The email address you entered is not valid because it "
                  "does not seem to contain an actual email address, i.e. "
                  "something of the form joe@kde.org.");
}
//-----------------------------------------------------------------------------
TQCString KPIM::getEmailAddress( const TQCString & address )
{
  TQCString dummy1, dummy2, addrSpec;
  KPIM::EmailParseResult result =
    splitAddressInternal( address, dummy1, addrSpec, dummy2,
                          false /* don't allow multiple addresses */ );
  if ( result != AddressOk ) {
    addrSpec = TQCString();
    kdDebug() // << k_funcinfo << "\n"
              << "Input: aStr\nError:"
              << emailParseResultToString( result ) << endl;
  }

  return addrSpec;
}


//-----------------------------------------------------------------------------
TQString KPIM::getEmailAddress( const TQString & address )
{
  return TQString::fromUtf8( getEmailAddress( address.utf8() ) );
}


//-----------------------------------------------------------------------------
TQCString KPIM::getFirstEmailAddress( const TQCString & addresses )
{
  TQCString dummy1, dummy2, addrSpec;
  KPIM::EmailParseResult result =
    splitAddressInternal( addresses, dummy1, addrSpec, dummy2,
                          true /* allow multiple addresses */ );
  if ( result != AddressOk ) {
    addrSpec = TQCString();
    kdDebug() // << k_funcinfo << "\n"
              << "Input: aStr\nError:"
              << emailParseResultToString( result ) << endl;
  }

  return addrSpec;
}


//-----------------------------------------------------------------------------
TQString KPIM::getFirstEmailAddress( const TQString & addresses )
{
  return TQString::fromUtf8( getFirstEmailAddress( addresses.utf8() ) );
}


//-----------------------------------------------------------------------------
bool KPIM::getNameAndMail(const TQString& aStr, TQString& name, TQString& mail)
{
  name = TQString::null;
  mail = TQString::null;

  const int len=aStr.length();
  const char cQuotes = '"';

  bool bInComment = false;
  bool bInQuotesOutsideOfEmail = false;
  int i=0, iAd=0, iMailStart=0, iMailEnd=0;
  TQChar c;
  unsigned int commentstack = 0;

  // Find the '@' of the email address
  // skipping all '@' inside "(...)" comments:
  while( i < len ){
    c = aStr[i];
    if( '(' == c ) commentstack++;
    if( ')' == c ) commentstack--;
    bInComment = commentstack != 0;
    if( '"' == c && !bInComment ) 
        bInQuotesOutsideOfEmail = !bInQuotesOutsideOfEmail;

    if( !bInComment && !bInQuotesOutsideOfEmail ){
      if( '@' == c ){
        iAd = i;
        break; // found it
      }
    }
    ++i;
  }

  if ( !iAd ) {
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

  } else {
    // Loop backwards until we find the start of the string
    // or a ',' that is outside of a comment
    //          and outside of quoted text before the leading '<'.
    bInComment = false;
    bInQuotesOutsideOfEmail = false;
    for( i = iAd-1; 0 <= i; --i ) {
      c = aStr[i];
      if( bInComment ) {
        if( '(' == c ) {
          if( !name.isEmpty() )
            name.prepend( ' ' );
          bInComment = false;
        } else {
          name.prepend( c ); // all comment stuff is part of the name
        }
      }else if( bInQuotesOutsideOfEmail ){
        if( cQuotes == c )
          bInQuotesOutsideOfEmail = false;
        else if ( c != '\\' )
          name.prepend( c );
      }else{
        // found the start of this addressee ?
        if( ',' == c )
          break;
        // stuff is before the leading '<' ?
        if( iMailStart ){
          if( cQuotes == c )
            bInQuotesOutsideOfEmail = true; // end of quoted text found
          else {
            name.prepend( c );
          }
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
    int parenthesesNesting = 0;
    for( i = iAd+1; len > i; ++i ) {
      c = aStr[i];
      if( bInComment ){
        if( ')' == c ){
          if ( --parenthesesNesting == 0 ) {
            bInComment = false;
            if( !name.isEmpty() )
              name.append( ' ' );
          } else {
            // nested ")", add it
            name.append( ')' ); // name can't be empty here
          }
        } else {
          if( '(' == c ) {
            // nested "("
            ++parenthesesNesting;
          }
          name.append( c ); // all comment stuff is part of the name
        }
      }else if( bInQuotesOutsideOfEmail ){
        if( cQuotes == c )
          bInQuotesOutsideOfEmail = false;
        else  if ( c != '\\' )
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
              if ( ++parenthesesNesting > 0 )
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


//-----------------------------------------------------------------------------
bool KPIM::compareEmail( const TQString& email1, const TQString& email2,
                         bool matchName )
{
  TQString e1Name, e1Email, e2Name, e2Email;

  getNameAndMail( email1, e1Name, e1Email );
  getNameAndMail( email2, e2Name, e2Email );

  return e1Email == e2Email &&
    ( !matchName || ( e1Name == e2Name ) );
}


//-----------------------------------------------------------------------------
TQString KPIM::normalizedAddress( const TQString & displayName,
                                 const TQString & addrSpec,
                                 const TQString & comment )
{
  TQString realDisplayName = displayName;
  realDisplayName.remove( TQChar( 0x202D ) );
  realDisplayName.remove( TQChar( 0x202E ) );
  realDisplayName.remove( TQChar( 0x202A ) );
  realDisplayName.remove( TQChar( 0x202B ) );

  if ( realDisplayName.isEmpty() && comment.isEmpty() )
    return addrSpec;
  else if ( comment.isEmpty() )
    return quoteNameIfNecessary( realDisplayName ) + " <" + addrSpec + ">";
  else if ( realDisplayName.isEmpty() ) {
    TQString commentStr = comment;
    return quoteNameIfNecessary( commentStr ) + " <" + addrSpec + ">";
  }
  else
    return realDisplayName + " (" + comment + ") <" + addrSpec + ">";
}


//-----------------------------------------------------------------------------
TQString KPIM::decodeIDN( const TQString & addrSpec )
{
  const int atPos = addrSpec.findRev( '@' );
  if ( atPos == -1 )
    return addrSpec;

  TQString idn = KIDNA::toUnicode( addrSpec.mid( atPos + 1 ) );
  if ( idn.isEmpty() )
    return TQString::null;

  return addrSpec.left( atPos + 1 ) + idn;
}


//-----------------------------------------------------------------------------
TQString KPIM::encodeIDN( const TQString & addrSpec )
{
  const int atPos = addrSpec.findRev( '@' );
  if ( atPos == -1 )
    return addrSpec;

  TQString idn = KIDNA::toAscii( addrSpec.mid( atPos + 1 ) );
  if ( idn.isEmpty() )
    return addrSpec;

  return addrSpec.left( atPos + 1 ) + idn;
}


//-----------------------------------------------------------------------------
TQString KPIM::normalizeAddressesAndDecodeIDNs( const TQString & str )
{
//  kdDebug() << "KPIM::normalizeAddressesAndDecodeIDNs( \""
//                << str << "\" )" << endl;
  if( str.isEmpty() )
    return str;

  const TQStringList addressList = KPIM::splitEmailAddrList( str );
  TQStringList normalizedAddressList;

  TQCString displayName, addrSpec, comment;

  for( TQStringList::ConstIterator it = addressList.begin();
       ( it != addressList.end() );
       ++it ) {
    if( !(*it).isEmpty() ) {
      if ( KPIM::splitAddress( (*it).utf8(), displayName, addrSpec, comment )
           == AddressOk ) {

        displayName = KMime::decodeRFC2047String(displayName).utf8();
        comment = KMime::decodeRFC2047String(comment).utf8();

        normalizedAddressList <<
          normalizedAddress( TQString::fromUtf8( displayName ),
                             decodeIDN( TQString::fromUtf8( addrSpec ) ),
                             TQString::fromUtf8( comment ) );
      }
      else {
        kdDebug() << "splitting address failed: " << *it << endl;
      }
    }
  }
/*
  kdDebug() << "normalizedAddressList: \""
                << normalizedAddressList.join( ", " )
                << "\"" << endl;
*/
  return normalizedAddressList.join( ", " );
}

//-----------------------------------------------------------------------------
TQString KPIM::normalizeAddressesAndEncodeIDNs( const TQString & str )
{
  //kdDebug() << "KPIM::normalizeAddressesAndEncodeIDNs( \""
  //              << str << "\" )" << endl;
  if( str.isEmpty() )
    return str;

  const TQStringList addressList = KPIM::splitEmailAddrList( str );
  TQStringList normalizedAddressList;

  TQCString displayName, addrSpec, comment;

  for( TQStringList::ConstIterator it = addressList.begin();
       ( it != addressList.end() );
       ++it ) {
    if( !(*it).isEmpty() ) {
      if ( KPIM::splitAddress( (*it).utf8(), displayName, addrSpec, comment )
           == AddressOk ) {

        normalizedAddressList <<
          normalizedAddress( TQString::fromUtf8( displayName ),
                             encodeIDN( TQString::fromUtf8( addrSpec ) ),
                             TQString::fromUtf8( comment ) );
      }
      else {
        kdDebug() << "splitting address failed: " << *it << endl;
      }
    }
  }

  /*
  kdDebug() << "normalizedAddressList: \""
                << normalizedAddressList.join( ", " )
                << "\"" << endl;
  */
  return normalizedAddressList.join( ", " );
}


//-----------------------------------------------------------------------------
// Escapes unescaped doublequotes in str.
static TQString escapeQuotes( const TQString & str )
{
  if ( str.isEmpty() )
    return TQString();

  TQString escaped;
  // reserve enough memory for the worst case ( """..."" -> \"\"\"...\"\" )
  escaped.reserve( 2*str.length() );
  unsigned int len = 0;
  for ( unsigned int i = 0; i < str.length(); ++i, ++len ) {
    if ( str[i] == '"' ) { // unescaped doublequote
      escaped[len] = '\\';
      ++len;
    }
    else if ( str[i] == '\\' ) { // escaped character
      escaped[len] = '\\';
      ++len;
      ++i;
      if ( i >= str.length() ) // handle trailing '\' gracefully
        break;
    }
    escaped[len] = str[i];
  }
  escaped.truncate( len );
  return escaped;
}

//-----------------------------------------------------------------------------
TQString KPIM::quoteNameIfNecessary( const TQString &str )
{
  TQString quoted = str;

  TQRegExp needQuotes(  "[^ 0-9A-Za-z\\x0080-\\xFFFF]" );
  // avoid double quoting
  if ( ( quoted[0] == '"' ) && ( quoted[quoted.length() - 1] == '"' ) ) {
    quoted = "\"" + escapeQuotes( quoted.mid( 1, quoted.length() - 2 ) ) + "\"";
  }
  else if ( quoted.find( needQuotes ) != -1 ) {
    quoted = "\"" + escapeQuotes( quoted ) + "\"";
  }

  return quoted;
}

