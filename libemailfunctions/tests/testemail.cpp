/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// Test program for libemailfunctions/email.h
#include "email.h"

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

using namespace KPIM;

static bool check(const QString& txt, const QString& a, const QString& b)
{
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

static bool check(const QString& txt, const QStringList& a, const QStringList& b)
{
  if ( a.join("\n") == b.join("\n") ) {
    kdDebug() << txt << " : checking list [ " << a.join( ", " ) << " ] against expected value [ " << b.join( ", " ) << " ]... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking list [ " << a.join( ",\n" ) << " ] against expected value [ " << b.join( ",\n" ) << " ]... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

static bool checkGetNameAndEmail(const QString& input, const QString& expName, const QString& expEmail, bool expRetVal)
{
  QString name, email;
  bool retVal = KPIM::getNameAndMail(input, name, email);
  check( "getNameAndMail " + input + " retVal", retVal?QString::fromLatin1( "true" ):QString::fromLatin1( "false" ), expRetVal?QString::fromLatin1( "true" ):QString::fromLatin1( "false" ) );
  check( "getNameAndMail " + input + " name", name, expName );
  check( "getNameAndMail " + input + " email", email, expEmail );
  return true;
}

// convert this to a switch instead but hey, nothing speedy in here is needed but still.. it would be nice
static QString emailTestParseResultToString( EmailParseResult errorCode )
{
  if( errorCode == TooManyAts ) {
    return "TooManyAts";
  } else if( errorCode == TooFewAts ) {
    return "TooFewAts";
  } else if( errorCode == AddressEmpty ) {
    return "AddressEmpty";
  } else if( errorCode == MissingLocalPart ) {
    return "MissingLocalPart";
  } else if( errorCode == MissingDomainPart ) {
    return "MissingDomainPart";
  } else if( errorCode == UnbalancedParens ) {
    return "UnbalancedParens";
  } else if( errorCode == AddressOk ) {
    return "AddressOk";
  } else if( errorCode == UnclosedAngleAddr ) {
    return "UnclosedAngleAddr";
  } else if( errorCode == UnexpectedEnd ) {
    return "UnexpectedEnd";
  } else if( errorCode == UnopenedAngleAddr ) {
    return "UnopenedAngleAddr";
  } else if( errorCode == DisallowedChar ) {
    return "DisallowedChar";
  }
  return "unknown error code";
}

static QString simpleEmailTestParseResultToString( bool validEmail )
{
  if ( validEmail ) {
    return "true";
  }
  return "false";
}

static QString getEmailParseResultToString( QCString emailAddress )
{
  return QString( emailAddress );
}

static QString getSplitEmailParseResultToString( QStringList emailAddresses )
{
  return QString( emailAddresses.join( "," ) );
}

static bool checkIsValidEmailAddress( const QString& input, const QString&  expErrorCode )
{
  EmailParseResult errorCode = KPIM::isValidEmailAddress( input );
  QString errorC = emailTestParseResultToString( errorCode );
  check( "isValidEmailAddress " + input + " errorCode ", errorC , expErrorCode );
  return true;
}

static bool checkIsValidSimpleEmailAddress( const QString& input, const QString& expResult )
{
  bool validEmail = KPIM::isValidSimpleEmailAddress( input );
  QString result = simpleEmailTestParseResultToString( validEmail );
  check( "isValidSimpleEmailAddress " + input + " result ", result, expResult );
  return true;
}

static bool checkGetEmailAddress( const QString& input, const QString& expResult )
{
  QString emailAddress = KPIM::getEmailAddress( input );
  QString result = emailAddress;
  check( "getEmailAddress " + input + " result ", result, expResult );
  return true;
}

static bool checkSplitEmailAddrList( const QString& input, const QStringList& expResult )
{
  QStringList emailAddresses = KPIM::splitEmailAddrList( input );
  check( "splitEmailAddrList( \"" + input + "\" ) result ", emailAddresses, expResult );
  return true;
}

static bool checkNormalizeAddressesAndEncodeIDNs( const QString& input, const QString& expResult )
{
  QString result = KPIM::normalizeAddressesAndEncodeIDNs( input );
  check( "normalizeAddressesAndEncodeIDNs( \"" + input + "\" ) result ", result, expResult );
  return true;
}

static bool checkQuoteIfNecessary( const QString& input, const QString& expResult )
{
  QString result = quoteNameIfNecessary( input );
  check( "quoteNameIfNecessary " + input + " result ", result, expResult );
  return true;
}


int main(int argc, char *argv[])
{
  KApplication::disableAutoDcopRegistration();
  KCmdLineArgs::init( argc, argv, "testemail", 0, 0, 0, 0 );
  KApplication app( false, false );

  // Empty input
  checkGetNameAndEmail( QString::null, QString::null, QString::null, false );

  // Email only
  checkGetNameAndEmail( "faure@kde.org", QString::null, "faure@kde.org", false );

  // Normal case
  checkGetNameAndEmail( "David Faure <faure@kde.org>", "David Faure", "faure@kde.org", true );

  // Double-quotes
  checkGetNameAndEmail( "\"Faure, David\" <faure@kde.org>", "Faure, David", "faure@kde.org", true );
  checkGetNameAndEmail( "<faure@kde.org> \"David Faure\"", "David Faure", "faure@kde.org", true );

  // Parenthesis
  checkGetNameAndEmail( "faure@kde.org (David Faure)", "David Faure", "faure@kde.org", true );
  checkGetNameAndEmail( "(David Faure) faure@kde.org", "David Faure", "faure@kde.org", true );
  checkGetNameAndEmail( "My Name (me) <me@home.net>", "My Name (me)", "me@home.net", true ); // #93513

  // Double-quotes inside parenthesis
  checkGetNameAndEmail( "faure@kde.org (David \"Crazy\" Faure)", "David \"Crazy\" Faure", "faure@kde.org", true );
  checkGetNameAndEmail( "(David \"Crazy\" Faure) faure@kde.org", "David \"Crazy\" Faure", "faure@kde.org", true );

  // Parenthesis inside double-quotes
  checkGetNameAndEmail( "\"Faure (David)\" <faure@kde.org>", "Faure (David)", "faure@kde.org", true );
  checkGetNameAndEmail( "<faure@kde.org> \"Faure (David)\"", "Faure (David)", "faure@kde.org", true );

  // Space in email
  checkGetNameAndEmail( "David Faure < faure@kde.org >", "David Faure", "faure@kde.org", true );

  // Check that '@' in name doesn't confuse it
  checkGetNameAndEmail( "faure@kde.org (a@b)", "a@b", "faure@kde.org", true );
  // Interestingly, this isn't supported.
  //checkGetNameAndEmail( "\"a@b\" <faure@kde.org>", "a@b", "faure@kde.org", true );

  // While typing, when there's no '@' yet
  checkGetNameAndEmail( "foo", "foo", QString::null, false );
  checkGetNameAndEmail( "foo <", "foo", QString::null, false );
  checkGetNameAndEmail( "foo <b", "foo", "b", true );

  // If multiple emails are there, only return the first one
  checkGetNameAndEmail( "\"Faure, David\" <faure@kde.org>, KHZ <khz@khz.khz>", "Faure, David", "faure@kde.org", true );

  // domain literals also need to work
  checkGetNameAndEmail( "Matt Douhan <matt@[123.123.123.123]>", "Matt Douhan", "matt@[123.123.123.123]", true );

  // No '@'
  checkGetNameAndEmail(  "foo <distlist>", "foo", "distlist", true );

  // To many @'s
  checkIsValidEmailAddress( "matt@@fruitsalad.org", "TooManyAts" );

  // To few @'s
  checkIsValidEmailAddress( "mattfruitsalad.org", "TooFewAts" );

  // An empty string
  checkIsValidEmailAddress( QString::null , "AddressEmpty" );

  // email address starting with a @
  checkIsValidEmailAddress( "@mattfruitsalad.org", "MissingLocalPart" );

  // make sure that starting @ and an additional @ in the same email address don't conflict
  // trap the starting @ first and break
  checkIsValidEmailAddress( "@matt@fruitsalad.org", "MissingLocalPart" );

  // email address ending with a @
  checkIsValidEmailAddress( "mattfruitsalad.org@", "MissingDomainPart" );

  // make sure that ending with@ and an additional @ in the email address don't conflict
  checkIsValidEmailAddress( "matt@fruitsalad.org@", "MissingDomainPart" );

  // unbalanced Parens
  checkIsValidEmailAddress( "mattjongel)@fruitsalad.org", "UnbalancedParens" );

  // unbalanced Parens the other way around
  checkIsValidEmailAddress( "mattjongel(@fruitsalad.org", "UnbalancedParens" );

  // Correct parens just to make sure it works
  checkIsValidEmailAddress( "matt(jongel)@fruitsalad.org", "AddressOk" );

  // Check that anglebrackets are closed
  checkIsValidEmailAddress( "matt douhan<matt@fruitsalad.org", "UnclosedAngleAddr" );

  // Check that angle brackets are closed the other way around
  checkIsValidEmailAddress( "matt douhan>matt@fruitsalad.org", "UnopenedAngleAddr" );

  // Check that angle brackets are closed the other way around, and anglebrackets in domainpart
  // instead of local part
  // checkIsValidEmailAddress( "matt douhanmatt@<fruitsalad.org", "UnclosedAngleAddr" );

  // check that a properly formated anglebrackets situation is OK
  checkIsValidEmailAddress( "matt douhan<matt@fruitsalad.org>", "AddressOk" );

  // a full email address with comments angle brackets and the works should be valid too
  checkIsValidEmailAddress( "Matt (jongel) Douhan <matt@fruitsalad.org>", "AddressOk" );

  // Double quotes
  checkIsValidEmailAddress( "\"Matt Douhan\" <matt@fruitsalad.org>", "AddressOk" );

  // Double quotes inside parens
  checkIsValidEmailAddress( "Matt (\"jongel\") Douhan <matt@fruitsalad.org>", "AddressOk" );

  // Parens inside double quotes
  checkIsValidEmailAddress( "Matt \"(jongel)\" Douhan <matt@fruitsalad.org>", "AddressOk" );

  // Space in email
  checkIsValidEmailAddress( "Matt Douhan < matt@fruitsalad.org >", "AddressOk" );

  // @ is allowed inisde doublequotes
  checkIsValidEmailAddress( "\"matt@jongel\" <matt@fruitsalad.org>", "AddressOk" );

  // anglebrackets inside dbl quotes
  checkIsValidEmailAddress( "\"matt<blah blah>\" <matt@fruitsalad.org>", "AddressOk" );

  // a , inside a double quoted string is OK, how do I know this? well Ingo says so
  // and it makes sense since it is also a seperator of email addresses
  checkIsValidEmailAddress( "\"Douhan, Matt\" <matt@fruitsalad.org>", "AddressOk" );

  // Domains literals also need to work
  checkIsValidEmailAddress( "Matt Douhan <matt@[123.123.123.123]>", "AddressOk" );

  // Some more insane tests but still valid so they must work
  checkIsValidEmailAddress( "Matt Douhan <\"m@att\"@jongel.com>", "AddressOk" );

  // BUG 99657
  checkIsValidEmailAddress( "matt@jongel.fibbel.com", "AddressOk" );

  // BUG 98720
  checkIsValidEmailAddress( "mailto:@mydomain", "DisallowedChar" );

  // checks for "pure" email addresses in the form of xxx@yyy.tld
  checkIsValidSimpleEmailAddress( "matt@fruitsalad.org", "true" );
  checkIsValidSimpleEmailAddress( QString::fromUtf8("test@täst.invalid"), "true" );
  // non-ASCII char as first char of IDN
  checkIsValidSimpleEmailAddress( QString::fromUtf8("i_want@øl.invalid"), "true" );
  checkIsValidSimpleEmailAddress( "matt@[123.123.123.123]", "true" );
  checkIsValidSimpleEmailAddress( "\"matt\"@fruitsalad.org", "true" );
  checkIsValidSimpleEmailAddress( "-matt@fruitsalad.org", "true" );
  checkIsValidSimpleEmailAddress( "\"-matt\"@fruitsalad.org", "true" );
  checkIsValidSimpleEmailAddress( "matt@jongel.fibbel.com", "true" );
  checkIsValidSimpleEmailAddress( "Matt Douhan <matt@fruitsalad.org>", "false" );

  // check if the pure email address is wrong
  checkIsValidSimpleEmailAddress( "mattfruitsalad.org", "false" );
  checkIsValidSimpleEmailAddress( "matt@[123.123.123.123", "false" );
  checkIsValidSimpleEmailAddress( "matt@123.123.123.123]", "false" );
  checkIsValidSimpleEmailAddress( "\"matt@fruitsalad.org", "false" );
  checkIsValidSimpleEmailAddress( "matt\"@fruitsalad.org", "false" );
  checkIsValidSimpleEmailAddress( QString::null, "false" );

  // and here some insane but still valid cases
  checkIsValidSimpleEmailAddress( "\"m@tt\"@fruitsalad.org", "true" );

  // check the getEmailAddress address method
  checkGetEmailAddress( "matt@fruitsalad.org", "matt@fruitsalad.org" );
  checkGetEmailAddress( "Matt Douhan <matt@fruitsalad.org>", "matt@fruitsalad.org" );
  checkGetEmailAddress( "\"Matt Douhan <blah blah>\" <matt@fruitsalad.org>", "matt@fruitsalad.org" );
  checkGetEmailAddress( "\"Matt <blah blah>\" <matt@fruitsalad.org>", "matt@fruitsalad.org" );
  checkGetEmailAddress( "Matt Douhan (jongel) <matt@fruitsalad.org", QString() );
  checkGetEmailAddress( "Matt Douhan (m@tt) <matt@fruitsalad.org>", "matt@fruitsalad.org" );
  checkGetEmailAddress( "\"Douhan, Matt\" <matt@fruitsalad.org>", "matt@fruitsalad.org" );
  checkGetEmailAddress( "\"Matt Douhan (m@tt)\" <matt@fruitsalad.org>", "matt@fruitsalad.org" );
  checkGetEmailAddress( "\"Matt Douhan\" (matt <matt@fruitsalad.org>", QString() );
  checkGetEmailAddress( "Matt Douhan <matt@[123.123.123.123]>", "matt@[123.123.123.123]" );

  // check the splitEmailAddrList method
  checkSplitEmailAddrList( "kloecker@kde.org (Kloecker, Ingo)", QStringList() << "kloecker@kde.org (Kloecker, Ingo)" );
  checkSplitEmailAddrList( "Matt Douhan <matt@fruitsalad.org>, Foo Bar <foo@bar.com>", QStringList() << "Matt Douhan <matt@fruitsalad.org>" << "Foo Bar <foo@bar.com>" );
  checkSplitEmailAddrList( "\"Matt, Douhan\" <matt@fruitsalad.org>, Foo Bar <foo@bar.com>", QStringList() << "\"Matt, Douhan\" <matt@fruitsalad.org>" << "Foo Bar <foo@bar.com>" );

  // check checkNormalizeAddressesAndEncodeIDNs
  checkNormalizeAddressesAndEncodeIDNs( "matt@fruitsalad.org", "matt@fruitsalad.org" );
  checkNormalizeAddressesAndEncodeIDNs( "Matt Douhan <matt@fruitsalad.org>", "Matt Douhan <matt@fruitsalad.org>" );
  checkNormalizeAddressesAndEncodeIDNs( "Matt Douhan (jongel) <matt@fruitsalad.org>", "Matt Douhan (jongel) <matt@fruitsalad.org>" );
  checkNormalizeAddressesAndEncodeIDNs( "Matt Douhan (jongel,fibbel) <matt@fruitsalad.org>", "Matt Douhan (jongel,fibbel) <matt@fruitsalad.org>" );
  checkNormalizeAddressesAndEncodeIDNs( "matt@fruitsalad.org (jongel,fibbel)", "\"jongel,fibbel\" <matt@fruitsalad.org>" );
  checkNormalizeAddressesAndEncodeIDNs( "matt@fruitsalad.org (\"jongel,fibbel\")", "\"jongel,fibbel\" <matt@fruitsalad.org>" );

  // check the "quote if necessary" method
  checkQuoteIfNecessary( "Matt Douhan", "Matt Douhan");
  checkQuoteIfNecessary( "Douhan, Matt", "\"Douhan, Matt\"");
  checkQuoteIfNecessary( "Matt \"jongel\" Douhan", "\"Matt \\\"jongel\\\" Douhan\"");
  checkQuoteIfNecessary( "Matt \\\"jongel\\\" Douhan", "\"Matt \\\"jongel\\\" Douhan\"");
  checkQuoteIfNecessary( "trailing '\\\\' should never occur \\", "\"trailing '\\\\' should never occur \\\"");

  printf("\nTest OK !\n");

  return 0;
}

