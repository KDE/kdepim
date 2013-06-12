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

#include "attachmentjobtest.h"
#include "qtest_messagecomposer.h"

#include <KDebug>
#include <qtest_kde.h>

#include <kmime/kmime_content.h>
#include <kmime/kmime_headers.h>
using namespace KMime;

#include <messagecomposer/composer/composer.h>
#include <messagecomposer/part/globalpart.h>
#include <messagecomposer/job/attachmentjob.h>
using namespace MessageComposer;

#include <messagecore/attachment/attachmentfromurljob.h>
#include <messagecore/attachment/attachmentpart.h>
#include <boost/shared_ptr.hpp>
using namespace MessageCore;

#define PATH_ATTACHMENTS QLatin1String( KDESRCDIR "/attachments/" )

QTEST_KDEMAIN( AttachmentJobTest, NoGUI )

void AttachmentJobTest::testAttachment()
{
  const QString name = QString::fromLatin1( "name" );
  const QString fileName = QString::fromLatin1( "filename" );
  const QString description = QString::fromLatin1( "long long long description..." );
  const QByteArray mimeType( "x-some/x-type" );
  const QByteArray data( "la la la" );

  AttachmentPart::Ptr part = AttachmentPart::Ptr( new AttachmentPart );
  part->setName( name );
  part->setFileName( fileName );
  part->setDescription( description );
  part->setMimeType( mimeType );
  part->setData( data );

  Composer *composer = new Composer;
  composer->globalPart()->setFallbackCharsetEnabled( true );
  AttachmentJob *ajob = new AttachmentJob( part, composer );
  QVERIFY( ajob->exec() );
  Content *result = ajob->content();
  delete ajob;
  ajob = 0;
  result->assemble();
  kDebug() << result->encodedContent();

  QCOMPARE( result->contentType( false )->name(), name );
  QCOMPARE( result->contentDisposition( false )->filename(), fileName );
  QCOMPARE( result->contentDescription( false )->asUnicodeString(), description );
  QCOMPARE( result->contentType( false )->mimeType(), mimeType );
  QCOMPARE( result->body(), data );
  QVERIFY( result->contentDisposition( false )->disposition() == Headers::CDattachment );
}

#if 0
// Disabled: using UTF-8 instead of trying to detect charset.

void AttachmentJobTest::testTextCharsetAutodetect_data()
{
  QTest::addColumn<KUrl>( "url" );
  QTest::addColumn<QByteArray>( "charset" );

  // PATH_ATTACHMENTS is defined by CMake.
  QTest::newRow( "ascii" ) << KUrl::fromPath( PATH_ATTACHMENTS + QString::fromLatin1( "ascii.txt" ) )
                           << QByteArray( "us-ascii" );
  QTest::newRow( "iso8859-2" ) << KUrl::fromPath( PATH_ATTACHMENTS + QString::fromLatin1( "iso8859-2.txt" ) )
                               << QByteArray( "iso-8859-2" );
  // TODO not sure how to test utf-16.
}

void AttachmentJobTest::testTextCharsetAutodetect()
{
  QFETCH( KUrl, url );
  QFETCH( QByteArray, charset );

  AttachmentFromUrlJob *ljob = new AttachmentFromUrlJob( url );
  VERIFYEXEC( ljob );
  AttachmentPart::Ptr part = ljob->attachmentPart();
  delete ljob;
  ljob = 0;

  Composer *composer = new Composer;
  composer->globalPart()->setFallbackCharsetEnabled( true );
  AttachmentJob *ajob = new AttachmentJob( part, composer );
  VERIFYEXEC( ajob );
  Content *result = ajob->content();
  delete ajob;
  ajob = 0;
  result->assemble();
  kDebug() << result->encodedContent();

  QCOMPARE( result->contentType( false )->charset(), charset );
}
#endif

#include "attachmentjobtest.moc"
