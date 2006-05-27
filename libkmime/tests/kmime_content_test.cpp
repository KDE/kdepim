/*
    Copyright (c) 2006 Volker Krause <volker.krause@rwth-aachen.de>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "kmime_content_test.h"
#include <qtest_kde.h>

#include <kmime_content.h>
#include <kmime_headers.h>
#include <kmime_message.h>
#include <kmime_headers.h>
using namespace KMime;

QTEST_KDEMAIN( KMimeContentTest, NoGUI )

void KMimeContentTest::testGetHeaderInstance( )
{
  // this fails with libkmime4...
  Headers::From *myfrom = new Headers::From();
  QCOMPARE( myfrom->type(), "From" );
  Headers::Base *mybase = myfrom;
  QCOMPARE( mybase->type(), "From" );

  // getHeaderInstance() is protected, so we need to test it via KMime::Message
  Message *c = new Message();
  Headers::From *from;
  Headers::From *f1 = c->from( true );
  Headers::From *f2 = c->from( true );
  QCOMPARE( f1, f2 );
  delete c;
}



#include "kmime_content_test.moc"
