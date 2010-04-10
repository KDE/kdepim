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

#include "signencrypttest.h"

#include <KDebug>
#include <qtest_kde.h>
#include "qtest_messagecomposer.h"
#include "cryptofunctions.h"

#include <akonadi/item.h>

#include <kmime/kmime_content.h>

#include <kleo/enum.h>
#include <kleo/keylistjob.h>
#include <kleo/cryptobackendfactory.h>
#include <kjob.h>

#include <messagecomposer/composer.h>
#include <messagecomposer/encryptjob.h>
#include <messagecomposer/signjob.h>

#include <messageviewer/objecttreeparser.h>
#include <messageviewer/objecttreeemptysource.h>
#include <messageviewer/nodehelper.h>

#include <messagecore/nodehelper.h>

QTEST_KDEMAIN( SignEncryptTest, GUI )


void SignEncryptTest::testContent() {

  std::vector< GpgME::Key > keys = ComposerTestUtil::getKeys();

  Message::Composer *composer = new Message::Composer;
  Message::SignJob* sJob = new Message::SignJob( composer );
  Message::EncryptJob* eJob = new Message::EncryptJob( composer );

  QVERIFY( composer );
  QVERIFY( sJob );
  QVERIFY( eJob );

  QByteArray data( QString::fromLocal8Bit( "one flew over the cuckoo's nest" ).toUtf8() );
  KMime::Content* content = new KMime::Content;
  content->setBody( data );

  QStringList recipients;
  recipients << QString::fromLocal8Bit( "test@kolab.org" );

  sJob->setContent( content );
  sJob->setSigningKeys( keys );
  sJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );
  
  eJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );
  eJob->setRecipients( recipients );
  eJob->setEncryptionKeys( keys );

  eJob->appendSubjob( sJob );

  VERIFYEXEC( eJob );

  KMime::Content* result = eJob->content();
  QVERIFY( result );
  result->assemble();

  kDebug() << "result:" << result->encodedContent();
  
  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption(
              result,
              QString::fromLocal8Bit( "one flew over the cuckoo's nest" ).toUtf8(),
              Kleo::OpenPGPMIMEFormat ) );
  
}


void SignEncryptTest::testHeaders()
{
  std::vector< GpgME::Key > keys = ComposerTestUtil::getKeys();

  Message::Composer *composer = new Message::Composer;
  Message::SignJob* sJob = new Message::SignJob( composer );
  Message::EncryptJob* eJob = new Message::EncryptJob( composer );

  QVERIFY( composer );
  QVERIFY( sJob );
  QVERIFY( eJob );

  QByteArray data( QString::fromLocal8Bit( "one flew over the cuckoo's nest" ).toUtf8() );
  KMime::Content* content = new KMime::Content;
  content->setBody( data );

  QStringList recipients;
  recipients << QString::fromLocal8Bit( "test@kolab.org" );

  sJob->setContent( content );
  sJob->setSigningKeys( keys );
  sJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );

  eJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );
  eJob->setRecipients( recipients );
  eJob->setEncryptionKeys( keys );

  eJob->appendSubjob( sJob );

  VERIFYEXEC( eJob );

  KMime::Content* result = eJob->content();
  QVERIFY( result );
  result->assemble();

  QByteArray mimeType( "multipart/encrypted" );
  QByteArray charset( "ISO-8859-1" );

  QVERIFY( result->contentType( false ) );
  QCOMPARE( result->contentType()->mimeType(), mimeType );
  QCOMPARE( result->contentType()->charset(), charset );
  QCOMPARE( result->contentType()->parameter( QString::fromLocal8Bit( "protocol" ) ), QString::fromLocal8Bit( "application/pgp-encrypted" ) );
  QCOMPARE( result->contentTransferEncoding()->encoding(), KMime::Headers::CE7Bit );

  // now unwrap the encrypted message to get the signed one, and check those headers

  KMime::Message* resultMessage =  new KMime::Message;
  resultMessage->setContent( result->encodedContent() );
  resultMessage->parse();
  MessageViewer::EmptySource es;
  MessageViewer::NodeHelper* nh = new MessageViewer::NodeHelper;
  MessageViewer::ObjectTreeParser otp( &es, nh, 0, false, false, true );
  KMime::Content* encPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pgp-encrypted", true, true );
  QVERIFY( encPart );
  MessageViewer::ProcessResult pResult( nh );
  otp.processMultiPartEncryptedSubtype( Akonadi::Item(), resultMessage, pResult );
  QVERIFY( nh->encryptionState( resultMessage ) == MessageViewer::KMMsgFullyEncrypted );
  KMime::Content* signedPart = MessageCore::NodeHelper::firstChild( resultMessage );

  mimeType = QString::fromLocal8Bit( "multipart/signed" ).toUtf8();
  QVERIFY( signedPart->contentType( false ) );
  QCOMPARE( signedPart->contentType()->mimeType(), mimeType );
  QCOMPARE( signedPart->contentType()->charset(), charset );
  QCOMPARE( signedPart->contentType()->parameter( QString::fromLocal8Bit( "micalg" ) ), QString::fromLocal8Bit( "pgp-sha1" ) );
  QCOMPARE( signedPart->contentType()->parameter( QString::fromLocal8Bit( "protocol" ) ), QString::fromLocal8Bit( "application/pgp-signature" ) );
  QCOMPARE( signedPart->contentTransferEncoding()->encoding(), KMime::Headers::CE7Bit );
}

#include "signencrypttest.moc"
