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

#include "contentjobtest.h"

#include <KDebug>
#include <qtest_kde.h>

#include <kmime/kmime_content.h>
using namespace KMime;

#include <messagecomposer/composer.h>
#include <messagecomposer/contentjob.h>
using namespace MessageComposer;

QTEST_KDEMAIN( ContentJobTest, NoGUI )

void ContentJobTest::testContent()
{
  Composer *composer = new Composer;
  ContentJob *cjob = new ContentJob( composer );
  QByteArray data( "birds came flying from the underground");
  cjob->setData( data );
  QVERIFY( cjob->exec() );
  Content *result = cjob->content();
  result->assemble();
  kDebug() << result->encodedContent();
  QCOMPARE( result->body(), data );
  QVERIFY( result->contentType( false ) == 0 ); // Not created unless demanded.
  QVERIFY( result->contentTransferEncoding( false ) ); // KMime gives it a default one (7bit).
}

void ContentJobTest::testContentType()
{
  Composer *composer = new Composer;
  ContentJob *cjob = new ContentJob( composer );
  QByteArray data( "birds came flying from the underground");
  cjob->setData( data );
  QByteArray mimeType( "text/plain" );
  cjob->contentType()->setMimeType( mimeType );
  QVERIFY( cjob->exec() );
  Content *result = cjob->content();
  result->assemble();
  kDebug() << result->encodedContent();
  QCOMPARE( result->body(), data );
  QVERIFY( result->contentType( false ) );
  QCOMPARE( result->contentType()->mimeType(), mimeType );
}

void ContentJobTest::testContentTransferEncoding()
{
  Composer *composer = new Composer;
  ContentJob *cjob = new ContentJob( composer );
  QByteArray data( "birds came flying from the underground");
  cjob->setData( data );
  cjob->contentTransferEncoding()->setEncoding( Headers::CEquPr );
  QVERIFY( cjob->exec() );
  Content *result = cjob->content();
  result->assemble();
  kDebug() << result->encodedContent();
  QCOMPARE( result->body(), data );
  QVERIFY( result->contentType( false ) == 0 ); // Not created unless demanded.
  QVERIFY( result->contentTransferEncoding( false ) );
  QCOMPARE( result->contentTransferEncoding()->encoding(), Headers::CEquPr );
}

#include "contentjobtest.moc"
