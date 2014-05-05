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

#include "singlepartjobtest.h"

#include <QDebug>
#include <qtest_kde.h>

#include <kmime/kmime_content.h>
using namespace KMime;

#include <messagecomposer/composer/composer.h>
#include <messagecomposer/part/globalpart.h>
#include <messagecomposer/job/singlepartjob.h>
using namespace MessageComposer;

QTEST_KDEMAIN( SinglepartJobTest, NoGUI )

void SinglepartJobTest::testContent()
{
  Composer *composer = new Composer;
  SinglepartJob *cjob = new SinglepartJob( composer );
  QByteArray data( "birds came flying from the underground" );
  cjob->setData( data );
  QVERIFY( cjob->exec() );
  Content *result = cjob->content();
  result->assemble();
  qDebug() << result->encodedContent();
  QCOMPARE( result->body(), data );
  QVERIFY( result->contentDisposition( false ) == 0 ); // Not created unless demanded.
  QVERIFY( result->contentType( false ) == 0 ); // Not created unless demanded.
  QVERIFY( result->contentTransferEncoding( false ) ); // KMime gives it a default one (7bit).
}

void SinglepartJobTest::testContentDisposition()
{
  Composer *composer = new Composer;
  SinglepartJob *cjob = new SinglepartJob( composer );
  QByteArray data( "birds came flying from the underground" );
  cjob->setData( data );
  QString filename = QString::fromUtf8( "test_ăîşţâ.txt" );
  cjob->contentDisposition()->setDisposition( Headers::CDattachment );
  cjob->contentDisposition()->setFilename( filename );
  QVERIFY( cjob->exec() );
  Content *result = cjob->content();
  result->assemble();
  qDebug() << result->encodedContent();
  QCOMPARE( result->body(), data );
  QVERIFY( result->contentDisposition( false ) );
  QCOMPARE( result->contentDisposition()->disposition(), Headers::CDattachment );
  QCOMPARE( result->contentDisposition()->filename(), filename );
}

void SinglepartJobTest::testContentID()
{
  Composer *composer = new Composer;
  SinglepartJob *cjob = new SinglepartJob( composer );
  QByteArray data( "birds came flying from the underground" );
  QByteArray id( "play@cold" );
  cjob->setData( data );
  cjob->contentID()->setIdentifier( id );
  QVERIFY( cjob->exec() );
  Content *result = cjob->content();
  result->assemble();
  qDebug() << result->encodedContent();
  QCOMPARE( result->body(), data );
  QVERIFY( result->header<Headers::ContentID>() );
  QCOMPARE( result->header<Headers::ContentID>()->identifier(), id );
}

void SinglepartJobTest::testContentType()
{
  Composer *composer = new Composer;
  SinglepartJob *cjob = new SinglepartJob( composer );
  QByteArray data( "birds came flying from the underground" );
  cjob->setData( data );
  QByteArray mimeType( "text/plain" );
  QByteArray charset( "utf-8" );
  cjob->contentType()->setMimeType( mimeType );
  cjob->contentType()->setCharset( charset );
  QVERIFY( cjob->exec() );
  Content *result = cjob->content();
  result->assemble();
  qDebug() << result->encodedContent();
  QCOMPARE( result->body(), data );
  QVERIFY( result->contentType( false ) );
  QCOMPARE( result->contentType()->mimeType(), mimeType );
  QCOMPARE( result->contentType()->charset(), charset );
}

void SinglepartJobTest::testContentTransferEncoding()
{
  Composer *composer = new Composer;
  QVERIFY( !composer->globalPart()->is8BitAllowed() );
  composer->globalPart()->setFallbackCharsetEnabled( true );
  
  // 7bit if possible.
  {
    SinglepartJob *cjob = new SinglepartJob( composer );
    QByteArray data( "and the sun will set for you..." );
    cjob->setData( data );
    QVERIFY( cjob->exec() );
    Content *result = cjob->content();
    result->assemble();
    qDebug() << result->encodedContent();
    QVERIFY( result->contentTransferEncoding( false ) );
    QCOMPARE( result->contentTransferEncoding()->encoding(), Headers::CE7Bit );
    QCOMPARE( result->body(), data );
  }

  // quoted-printable if text doesn't fit in 7bit.
  {
    SinglepartJob *cjob = new SinglepartJob( composer );
    QByteArray data( "some long text to make qupr more compact than base64 [ăîşţâ]" ); // utf-8
    cjob->setData( data );
    QVERIFY( cjob->exec() );
    Content *result = cjob->content();
    result->assemble();
    qDebug() << result->encodedContent();
    QVERIFY( result->contentTransferEncoding( false ) );
    QCOMPARE( result->contentTransferEncoding()->encoding(), Headers::CEquPr );
    QCOMPARE( result->body(), data );
  }

  // base64 if it's shorter than quoted-printable
  {
    SinglepartJob *cjob = new SinglepartJob( composer );
    QByteArray data( "[ăîşţâ]" ); // utf-8
    cjob->setData( data );
    QVERIFY( cjob->exec() );
    Content *result = cjob->content();
    result->assemble();
    qDebug() << result->encodedContent();
    QVERIFY( result->contentTransferEncoding( false ) );
    QCOMPARE( result->contentTransferEncoding()->encoding(), Headers::CEbase64 );
    QCOMPARE( result->body(), data );
  }

  // 8bit if asked for and allowed.
  {
    composer->globalPart()->set8BitAllowed( true );
    QByteArray data( "[ăîşţâ]" ); // utf-8
    SinglepartJob *cjob = new SinglepartJob( composer );
    cjob->setData( data );
    QVERIFY( cjob->exec() );
    Content *result = cjob->content();
    result->assemble();
    qDebug() << result->encodedContent();
    QVERIFY( result->contentTransferEncoding( false ) );
    QCOMPARE( result->contentTransferEncoding()->encoding(), Headers::CE8Bit );
    QCOMPARE( result->body(), data );
  }
}

