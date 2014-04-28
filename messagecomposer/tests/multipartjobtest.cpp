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

#include <KMime/kmime_content.h>
using namespace KMime;

#include <messagecomposer/composer/composer.h>
#include <messagecomposer/part/globalpart.h>
#include <messagecomposer/job/singlepartjob.h>
#include <messagecomposer/job/multipartjob.h>
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
    SinglepartJob *cjob = new SinglepartJob( mjob );
    cjob->setData( data1 );
    cjob->contentType()->setMimeType( type1 );
  }

  {
    SinglepartJob *cjob = new SinglepartJob( mjob );
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

void MultipartJobTest::test8BitPropagation()
{
  // If a subpart is 8bit, its parent must be 8bit too.

  Composer *composer = new Composer;
  composer->globalPart()->set8BitAllowed( true );
  MultipartJob *mjob = new MultipartJob( composer );
  mjob->setMultipartSubtype( "mixed" );
  MultipartJob *mjob2 = new MultipartJob( mjob );
  mjob2->setMultipartSubtype( "mixed" );
  SinglepartJob *cjob = new SinglepartJob( mjob2 );
  QByteArray data( "time is so short and I'm sure there must be something more" );
  cjob->setData( data );
  cjob->contentTransferEncoding()->setEncoding( Headers::CE8Bit );
  QVERIFY( mjob->exec() );
  Content *content = mjob->content();
  content->assemble();
  kDebug() << content->encodedContent();
  QVERIFY( content->contentTransferEncoding( false ) );
  QCOMPARE( content->contentTransferEncoding()->encoding(), Headers::CE8Bit );
}

