/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "multipartjobtest.h"

#include <KDebug>
#include <qtest_kde.h>

#include <kmime/kmime_content.h>
using namespace KMime;

#include <messagecomposer/composer.h>
#include <messagecomposer/contentjob.h>
#include <messagecomposer/multipartjob.h>
using namespace MessageComposer;

QTEST_KDEMAIN( MultipartJobTest, NoGUI )

void MultipartJobTest::testMultipartMixed()
{
  Composer *composer = new Composer;
  MultipartJob *mjob = new MultipartJob( composer );
  mjob->setMultipartSubtype( "mixed" );

  QByteArray data1( "one" );
  QByteArray data2( "two" );
  QByteArray type1( "text/plain" );
  QByteArray type2( "application/x-mors-ontologica" );
  
  {
    ContentJob *cjob = new ContentJob( mjob );
    cjob->setData( data1 );
    cjob->contentType()->setMimeType( type1 );
  }

  {
    ContentJob *cjob = new ContentJob( mjob );
    cjob->setData( data2 );
    cjob->contentType()->setMimeType( type2 );
  }

  QVERIFY( mjob->exec() );
  Content *result = mjob->content();
  result->assemble();
  kDebug() << result->encodedContent();

  QVERIFY( result->contentType( false ) );
  QCOMPARE( result->contentType()->mimeType(), QByteArray( "multipart/mixed" ) );
  QCOMPARE( result->contents().count(), 2 );

  {
    Content *c = result->contents().at( 0 );
    QCOMPARE( c->body(), data1 );
    QVERIFY( c->contentType( false ) );
    QCOMPARE( c->contentType()->mimeType(), type1 );
  }

  {
    Content *c = result->contents().at( 1 );
    QCOMPARE( c->body(), data2 );
    QVERIFY( c->contentType( false ) );
    QCOMPARE( c->contentType()->mimeType(), type2 );
  }
}

#include "multipartjobtest.moc"
