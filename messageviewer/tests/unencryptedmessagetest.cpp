/*
  Copyright (c) 2010 Thomas McGuire <thomas.mcguire@kdab.com>

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
#include "viewer/objecttreeparser.h"
#include "util.h"

#include "viewer/objecttreeemptysource.h"

#include <messagecore/tests/util.h>

#include <qtest_kde.h>

using namespace MessageViewer;

class UnencryptedMessageTest : public QObject
{
  Q_OBJECT
  private slots:
    void initTestCase();
    void testMailWithoutEncryption();
    void testSMIMESignedEncrypted();
    void testOpenPGPSignedEncrypted();
    void testForwardedOpenPGPSignedEncrypted();
    void testSignedForwardedOpenPGPSignedEncrypted();
    void testOpenPGPEncrypted();
};

QTEST_KDEMAIN( UnencryptedMessageTest, GUI )

void UnencryptedMessageTest::initTestCase()
{
  MessageCore::Test::setupEnv();
}

void UnencryptedMessageTest::testMailWithoutEncryption()
{
  KMime::Message::Ptr originalMessage = readAndParseMail( "encapsulated-with-attachment.mbox" );
  NodeHelper nodeHelper;
  EmptySource emptySource;
  ObjectTreeParser otp( &emptySource, &nodeHelper );
  otp.parseObjectTree( originalMessage.get() );
  QVERIFY( !nodeHelper.unencryptedMessage( originalMessage ) );
}

void UnencryptedMessageTest::testSignedForwardedOpenPGPSignedEncrypted()
{
  KMime::Message::Ptr originalMessage = readAndParseMail( "signed-forward-openpgp-signed-encrypted.mbox" );

  NodeHelper nodeHelper;
    TestHtmlWriter testWriter;
  TestCSSHelper testCSSHelper;
  MessageCore::Test::TestObjectTreeSource emptySource( &testWriter, &testCSSHelper );
  ObjectTreeParser otp( &emptySource, &nodeHelper );
  otp.parseObjectTree( originalMessage.get() );

  QCOMPARE( otp.plainTextContent().toLatin1().data(), "bla bla bla" ); // The textual content doesn't include the encrypted encapsulated message by design
  QCOMPARE( nodeHelper.overallEncryptionState( originalMessage.get() ), KMMsgPartiallyEncrypted );
  QCOMPARE( nodeHelper.overallSignatureState( originalMessage.get() ), KMMsgFullySigned );

  KMime::Message::Ptr unencryptedMessage = nodeHelper.unencryptedMessage( originalMessage );
  QVERIFY( !unencryptedMessage ); // We must not invalidate the outer signature
}

void UnencryptedMessageTest::testForwardedOpenPGPSignedEncrypted()
{
  KMime::Message::Ptr originalMessage = readAndParseMail( "forward-openpgp-signed-encrypted.mbox" );

  NodeHelper nodeHelper;
    TestHtmlWriter testWriter;
  TestCSSHelper testCSSHelper;
  MessageCore::Test::TestObjectTreeSource emptySource( &testWriter, &testCSSHelper );
  emptySource.setAllowDecryption( true );
  ObjectTreeParser otp( &emptySource, &nodeHelper );
  otp.parseObjectTree( originalMessage.get() );

  QCOMPARE( otp.plainTextContent().toLatin1().data(), "bla bla bla" ); // The textual content doesn't include the encrypted encapsulated message by design
  QCOMPARE( nodeHelper.overallEncryptionState( originalMessage.get() ), KMMsgPartiallyEncrypted );

  // Signature state handling is broken. First, the state is apparently not calculated correctly,
  // and then the state is never stored somewhere so it can't be remembered.
  QEXPECT_FAIL( "", "Signature state handling broken!", Continue );
  QVERIFY( nodeHelper.overallSignatureState( originalMessage.get() ) != KMMsgNotSigned );

  // Now, test that the unencrypted message is generated correctly
  KMime::Message::Ptr unencryptedMessage = nodeHelper.unencryptedMessage( originalMessage );
  QVERIFY( unencryptedMessage.get() );
  QCOMPARE( unencryptedMessage->contentType()->mimeType().data(), "multipart/mixed" );
  QCOMPARE( unencryptedMessage->contents().size(), 2 );
  QCOMPARE( unencryptedMessage->contents().first()->contentType()->mimeType().data(), "text/plain" );
  QCOMPARE( unencryptedMessage->contents().first()->decodedContent().data(), "bla bla bla" );
  QCOMPARE( unencryptedMessage->contents().at( 1 )->contentType()->mimeType().data(), "message/rfc822" );
  KMime::Message::Ptr encapsulated = unencryptedMessage->contents().at( 1 )->bodyAsMessage();
  QCOMPARE( encapsulated->contentType()->mimeType().data(), "multipart/signed" );
  QCOMPARE( encapsulated->contents().size(), 2 );
  QCOMPARE( encapsulated->contents().first()->contentType()->mimeType().data(), "text/plain" );
  QCOMPARE( encapsulated->contents().at( 1 )->contentType()->mimeType().data(), "application/pgp-signature" );
  QCOMPARE( encapsulated->contents().first()->decodedContent().data(), "encrypted message text" );

  // TODO: Check that the signature is valid
}

void UnencryptedMessageTest::testSMIMESignedEncrypted()
{
  KMime::Message::Ptr originalMessage = readAndParseMail( "smime-signed-encrypted.mbox" );

  NodeHelper nodeHelper;
  EmptySource emptySource;
  emptySource.setAllowDecryption( true );
  ObjectTreeParser otp( &emptySource, &nodeHelper );
  otp.parseObjectTree( originalMessage.get() );

  QCOMPARE( otp.plainTextContent().toLatin1().data(), "encrypted message text" );
  QCOMPARE( nodeHelper.overallEncryptionState( originalMessage.get() ), KMMsgFullyEncrypted );

  // Signature state handling is broken. First, the state is apparently not calculated correctly,
  // and then the state is never stored somewhere so it can't be remembered.
  QEXPECT_FAIL( "", "Signature state handling broken!", Continue );
  QVERIFY( nodeHelper.overallSignatureState( originalMessage.get() ) != KMMsgNotSigned );

  // Now, test that the unencrypted message is generated correctly
  KMime::Message::Ptr unencryptedMessage = nodeHelper.unencryptedMessage( originalMessage );
  QCOMPARE( unencryptedMessage->contentType()->mimeType().data(), "multipart/signed" );
  QCOMPARE( unencryptedMessage->contents().size(), 2 );
  QCOMPARE( unencryptedMessage->contents().first()->contentType()->mimeType().data(), "text/plain" );
  QCOMPARE( unencryptedMessage->contents().at( 1 )->contentType()->mimeType().data(), "application/pkcs7-signature" );
  QCOMPARE( unencryptedMessage->contents().first()->decodedContent().data(), "encrypted message text" );

  // TODO: Check that the signature is valid
}

void UnencryptedMessageTest::testOpenPGPSignedEncrypted()
{
  KMime::Message::Ptr originalMessage = readAndParseMail( "openpgp-signed-encrypted.mbox" );

  NodeHelper nodeHelper;
  EmptySource emptySource;
  emptySource.setAllowDecryption( true );
  ObjectTreeParser otp( &emptySource, &nodeHelper );
  otp.parseObjectTree( originalMessage.get() );

  QCOMPARE( otp.plainTextContent().toLatin1().data(), "encrypted message text" );
  QCOMPARE( nodeHelper.overallEncryptionState( originalMessage.get() ), KMMsgFullyEncrypted );

  // Signature state handling is broken. First, the state is apparently not calculated correctly,
  // and then the state is never stored somewhere so it can't be remembered.
  QEXPECT_FAIL( "", "Signature state handling broken!", Continue );
  QVERIFY( nodeHelper.overallSignatureState( originalMessage.get() ) != KMMsgNotSigned );

  // Now, test that the unencrypted message is generated correctly
  KMime::Message::Ptr unencryptedMessage = nodeHelper.unencryptedMessage( originalMessage );
  QCOMPARE( unencryptedMessage->contentType()->mimeType().data(), "multipart/signed" );
  QCOMPARE( unencryptedMessage->contents().size(), 2 );
  QCOMPARE( unencryptedMessage->contents().first()->contentType()->mimeType().data(), "text/plain" );
  QCOMPARE( unencryptedMessage->contents().at( 1 )->contentType()->mimeType().data(), "application/pgp-signature" );
  QCOMPARE( unencryptedMessage->contents().first()->decodedContent().data(), "encrypted message text" );

  // TODO: Check that the signature is valid
}

void UnencryptedMessageTest::testOpenPGPEncrypted()
{
  KMime::Message::Ptr originalMessage = readAndParseMail( "openpgp-encrypted.mbox" );

  NodeHelper nodeHelper;
  EmptySource emptySource;
  emptySource.setAllowDecryption( true );
  ObjectTreeParser otp( &emptySource, &nodeHelper );
  otp.parseObjectTree( originalMessage.get() );

  QCOMPARE( otp.plainTextContent().toLatin1().data(), "encrypted message text" );
  QCOMPARE( nodeHelper.overallEncryptionState( originalMessage.get() ), KMMsgFullyEncrypted );

  // Now, test that the unencrypted message is generated correctly
  KMime::Message::Ptr unencryptedMessage = nodeHelper.unencryptedMessage( originalMessage );
  QCOMPARE( unencryptedMessage->contentType()->mimeType().data(), "text/plain" );
  QCOMPARE( unencryptedMessage->decodedContent().data(), "encrypted message text" );
  QCOMPARE( unencryptedMessage->contents().size(), 0 );
}

#include "unencryptedmessagetest.moc"
