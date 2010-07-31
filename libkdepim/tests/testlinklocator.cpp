/* This file is part of the KDE project
   Copyright (C) 2005 Ingo Kloecker <kloecker@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Test program for libkdepim/linklocator.*
#include <linklocator.h>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static bool check(const TQString& txt, const TQString& a, const TQString& b)
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

static bool checkGetEmailAddress( const TQString & input,
                                  int atPos,
                                  const TQString & expRetVal,
                                  bool allowBadAtPos = false )
{
  if ( !allowBadAtPos && ( input[atPos] != '@' ) ) {
    kdDebug() << "atPos (" << atPos << ") doesn't point to '@' in \""
              << input << "\". Fix the check!" << endl;
    exit(1);
  }
  LinkLocator ll( input, atPos );
  const TQString retVal = ll.getEmailAddress();
  check( "getEmailAddress() \"" + input + "\", " + TQString::number( atPos ),
         retVal, expRetVal );
  return true;
}

int main(int argc, char *argv[])
{
  KApplication::disableAutoDcopRegistration();
  KCmdLineArgs::init( argc, argv, "testlinklocator", 0, 0, 0, 0 );
  KApplication app( false, false );

  // empty input
  checkGetEmailAddress( TQString(), 0, TQString(), true );

  // no '@' at scan position
  checkGetEmailAddress( "foo@bar.baz", 0, TQString(), true );

  // '@' in local part
  checkGetEmailAddress( "foo@bar@bar.baz", 7, TQString() );

  // empty local part
  checkGetEmailAddress( "@bar.baz", 0, TQString() );
  checkGetEmailAddress( ".@bar.baz", 1, TQString() );
  checkGetEmailAddress( " @bar.baz", 1, TQString() );
  checkGetEmailAddress( ".!#$%&'*+-/=?^_`{|}~@bar.baz", strlen(".!#$%&'*+-/=?^_`{|}~"), TQString() );

  // allowed special chars in local part of address
  checkGetEmailAddress( "a.!#$%&'*+-/=?^_`{|}~@bar.baz", strlen("a.!#$%&'*+-/=?^_`{|}~"), "a.!#$%&'*+-/=?^_`{|}~@bar.baz" );

  // '@' in domain part
  checkGetEmailAddress( "foo@bar@bar.baz", 3, TQString() );

  // domain part without dot
  checkGetEmailAddress( "foo@bar", 3, TQString() );
  checkGetEmailAddress( "foo@bar.", 3, TQString() );
  checkGetEmailAddress( ".foo@bar", 4, TQString() );
  checkGetEmailAddress( "foo@bar ", 3, TQString() );
  checkGetEmailAddress( " foo@bar", 4, TQString() );
  checkGetEmailAddress( "foo@bar-bar", 3, TQString() );

  // empty domain part
  checkGetEmailAddress( "foo@", 3, TQString() );
  checkGetEmailAddress( "foo@.", 3, TQString() );
  checkGetEmailAddress( "foo@-", 3, TQString() );

  // simple address
  checkGetEmailAddress( "foo@bar.baz", 3, "foo@bar.baz" );
  checkGetEmailAddress( "foo@bar.baz.", 3, "foo@bar.baz" );
  checkGetEmailAddress( ".foo@bar.baz", 4, "foo@bar.baz" );
  checkGetEmailAddress( "foo@bar.baz-", 3, "foo@bar.baz" );
  checkGetEmailAddress( "-foo@bar.baz", 4, "foo@bar.baz" );
  checkGetEmailAddress( "foo@bar.baz ", 3, "foo@bar.baz" );
  checkGetEmailAddress( " foo@bar.baz", 4, "foo@bar.baz" );
  checkGetEmailAddress( "foo@bar-bar.baz", 3, "foo@bar-bar.baz" );

  printf("\nTest OK !\n");

  return 0;
}

