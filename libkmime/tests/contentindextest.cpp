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

#include "contentindextest.h"

#include <kmime_content.h>
#include <kmime_contentindex.h>

using namespace KMime;

#include <qtest_kde.h>

QTEST_KDEMAIN( ContentIndexTest, NoGUI )

void ContentIndexTest::testToString()
{
  KMime::ContentIndex ci;
  QCOMPARE( ci.toString(), QString() );
  ci.push( 1 );
  QCOMPARE( ci.toString(), QString( "1" ) );
  ci.push( 2 );
  QCOMPARE( ci.toString(), QString( "2.1" ) );
}

void ContentIndexTest::testFromString( )
{
  ContentIndex ci1;
  QVERIFY( !ci1.isValid() );

  ContentIndex ci2( "1.2.bla" );
  QVERIFY( !ci2.isValid() );

  ContentIndex ci3( "1" );
  QVERIFY( ci3.isValid() );
  QCOMPARE( ci3.pop(), 1u );
  QVERIFY( !ci3.isValid() );

  ContentIndex ci4( "3.2" );
  QVERIFY( ci4.isValid() );
  QCOMPARE( ci4.pop(), 3u );
  QCOMPARE( ci4.pop(), 2u );
  QVERIFY( !ci4.isValid() );
}

void ContentIndexTest::testContent( )
{
  Content *c1 = new Content();
  QCOMPARE( c1->content( ContentIndex() ), c1 );
  QCOMPARE( c1->content( ContentIndex( "1" ) ), (Content*)0 );
  QCOMPARE( c1->indexForContent( c1 ), ContentIndex() );

  Content *c11 = new Content();
  // this makes c1 multipart/mixed, ie. c11 will be the second child!
  c1->addContent( c11 );
  QCOMPARE( c1->content( ContentIndex( "2" ) ), c11 );
  QCOMPARE( c1->content( ContentIndex( "3" ) ), (Content*)0 );
  QCOMPARE( c1->content( ContentIndex( "2.1" ) ), (Content*)0 );
  QCOMPARE( c1->indexForContent( c1 ), ContentIndex() );
  QCOMPARE( c1->indexForContent( c11 ), ContentIndex( "2" ) );

  Content *c12 = new Content();
  c1->addContent( c12 );
  // c12 becomes multipart/mixed, ie. c12 will be the second child!
  Content *c121 = new Content();
  c12->addContent( c121 );
  QCOMPARE( c1->content( ContentIndex( "3" ) ), c12 );
  QCOMPARE( c1->content( ContentIndex( "3.2" ) ), c121 );
  QCOMPARE( c1->indexForContent( c121 ), ContentIndex( "3.2" ) );

  QCOMPARE( c1->indexForContent( (Content*)0 ), ContentIndex() );
}

#include "contentindextest.moc"
