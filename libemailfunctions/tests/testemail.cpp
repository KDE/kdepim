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

// Test program for libkdepim/email.h
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

static bool checkGetNameAndEmail(const QString& input, const QString& expName, const QString& expEmail, bool expRetVal)
{
  QString name, email;
  bool retVal = KPIM::getNameAndMail(input, name, email);
  check( "getNameAndMail " + input + " retVal", retVal?"true":"false", expRetVal?"true":"false" );
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
  }
  return "unknown errror code";
}

static QString simpleEmailTestParseResultToString( bool validEmail )
{
  if ( validEmail ) {
    return "true"; 
  } else if ( !validEmail ) {
    return "false";
  }
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

  // a , inside a double quoted string is OK, how do I know this? well Ingo says so
  // and it makes sense since it is also a seperator of email addresses
  checkIsValidEmailAddress( "\"Douhan, Matt\" <matt@fruitsalad.org>", "AddressOk" );

  // checks for "pure" email addresses in the form of xxx@yyy.tld
  checkIsValidSimpleEmailAddress( "matt@fruitsalad.org", "true" );
  checkIsValidSimpleEmailAddress( QString::fromUtf8("test@täst.invalid"), "true" );
  // non-ASCII char as first char of IDN
  checkIsValidSimpleEmailAddress( QString::fromUtf8("i_want@øl.invalid"), "true" );

  // check if the pure email address is wrong
  checkIsValidSimpleEmailAddress( "mattfruitsalad.org", "false" );

  printf("\nTest OK !\n");

  return 0;
}

