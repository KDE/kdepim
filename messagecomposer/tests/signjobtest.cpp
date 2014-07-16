/*
  Copyright (C) 2009 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>

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

#include "signjobtest.h"

#include <KDebug>
#include <qtest_kde.h>
#include "qtest_messagecomposer.h"
#include "cryptofunctions.h"

#include <kmime/kmime_content.h>

#include <kleo/enum.h>
#include <kleo/keylistjob.h>
#include <kleo/cryptobackendfactory.h>
#include <kjob.h>

#include <messagecomposer/composer/composer.h>
#include <messagecomposer/job/signjob.h>
#include <messagecomposer/job/transparentjob.h>

#include <messagecore/helpers/nodehelper.h>
#include <messagecore/tests/util.h>

#include <stdlib.h>

QTEST_KDEMAIN( SignJobTest, GUI )

void SignJobTest::initTestCase()
{
  MessageCore::Test::setupEnv();
}

void SignJobTest::testContentDirect() {

  std::vector< GpgME::Key > keys = MessageCore::Test::getKeys();

  MessageComposer::Composer *composer = new MessageComposer::Composer;
  MessageComposer::SignJob* sJob = new MessageComposer::SignJob( composer );

  QVERIFY( composer );
  QVERIFY( sJob );

  QByteArray data( QString::fromLocal8Bit( "one flew over the cuckoo's nest" ).toUtf8() );
  KMime::Content* content = new KMime::Content;
  content->setBody( data );

  sJob->setContent( content );
  sJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );
  sJob->setSigningKeys( keys );

  checkSignJob( sJob );
}

void SignJobTest::testContentChained()
{
  std::vector< GpgME::Key > keys = MessageCore::Test::getKeys();

  QByteArray data( QString::fromLocal8Bit( "one flew over the cuckoo's nest" ).toUtf8() );
  KMime::Content* content = new KMime::Content;
  content->setBody( data );

  MessageComposer::TransparentJob* tJob =  new MessageComposer::TransparentJob;
  tJob->setContent( content );

  MessageComposer::Composer *composer = new MessageComposer::Composer;
  MessageComposer::SignJob* sJob = new MessageComposer::SignJob( composer );

  sJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );
  sJob->setSigningKeys( keys );

  sJob->appendSubjob( tJob );

  checkSignJob( sJob );
}


void SignJobTest::testHeaders()
{
  std::vector< GpgME::Key > keys = MessageCore::Test::getKeys();

  MessageComposer::Composer *composer = new MessageComposer::Composer;
  MessageComposer::SignJob* sJob = new MessageComposer::SignJob( composer );

  QVERIFY( composer );
  QVERIFY( sJob );

  QByteArray data( QString::fromLocal8Bit( "one flew over the cuckoo's nest" ).toUtf8() );
  KMime::Content* content = new KMime::Content;
  content->setBody( data );

  sJob->setContent( content );
  sJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );
  sJob->setSigningKeys( keys );

  VERIFYEXEC( sJob );

  QByteArray mimeType( "multipart/signed" );
  QByteArray charset( "ISO-8859-1" );

  KMime::Content *result = sJob->content();
  result->assemble();
  kDebug() << result->encodedContent();

  QVERIFY( result->contentType( false ) );
  QCOMPARE( result->contentType()->mimeType(), mimeType );
  QCOMPARE( result->contentType()->charset(), charset );
  QCOMPARE( result->contentType()->parameter( QString::fromLocal8Bit( "micalg" ) ), QString::fromLocal8Bit( "pgp-sha1" ) );
  QCOMPARE( result->contentType()->parameter( QString::fromLocal8Bit( "protocol" ) ), QString::fromLocal8Bit( "application/pgp-signature" ) );
  QCOMPARE( result->contentTransferEncoding()->encoding(), KMime::Headers::CE7Bit );
}

void SignJobTest::testRecommentationRFC3156()
{
  std::vector< GpgME::Key > keys = MessageCore::Test::getKeys();

  QString data = QString::fromUtf8( "=2D Magic foo\nFrom test\n\n-- quaak\nOhno");
  KMime::Headers::contentEncoding cte = KMime::Headers::CEquPr;

  MessageComposer::Composer *composer = new MessageComposer::Composer;
  MessageComposer::SignJob* sJob = new MessageComposer::SignJob( composer );

  QVERIFY( composer );
  QVERIFY( sJob );

  KMime::Content* content = new KMime::Content;
  content->setBody( data.toUtf8() );

  sJob->setContent( content );
  sJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );
  sJob->setSigningKeys( keys );

  VERIFYEXEC( sJob );

  KMime::Content *result = sJob->content();
  result->assemble();
  kDebug() << result->encodedContent();

  QByteArray body = MessageCore::NodeHelper::firstChild( result )->body();
  QCOMPARE( QString::fromUtf8( body ),
      QString::fromUtf8( "=3D2D Magic foo\nFrom=20test\n\n=2D- quaak\nOhno" ) );

  ComposerTestUtil::verify( true, false, result, data.toUtf8(),
           Kleo::OpenPGPMIMEFormat, cte );

}

void SignJobTest::checkSignJob( MessageComposer::SignJob* sJob )
{

  VERIFYEXEC( sJob );

  KMime::Content* result = sJob->content();
  Q_ASSERT( result );
  result->assemble();

  ComposerTestUtil::verifySignature( result, QString::fromLocal8Bit( "one flew over the cuckoo's nest" ).toUtf8(), Kleo::OpenPGPMIMEFormat, KMime::Headers::CE7Bit );
}

