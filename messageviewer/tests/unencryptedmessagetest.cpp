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
#include "objecttreeparser.h"
#include "util.h"

#include "objecttreeemptysource.h"

#include <qtest_kde.h>

using namespace MessageViewer;

class UnencryptedMessageTest : public QObject
{
  Q_OBJECT
  private slots:
    void initTestCase();
    void testOpenPGPSignedEncrypted();
};

QTEST_KDEMAIN( UnencryptedMessageTest, GUI )

void UnencryptedMessageTest::initTestCase()
{
  setenv("GNUPGHOME", KDESRCDIR "../../messagecore/tests/gnupg_home" , 1 );
  setenv("LC_ALL", "C", 1);
}

void UnencryptedMessageTest::testOpenPGPSignedEncrypted()
{
  KMime::Message::Ptr originalMessage = readAndParseMail( "openpgp-signed-encrypted.mbox" );

  NodeHelper nodeHelper;
  EmptySource emptySource;
  ObjectTreeParser otp( &emptySource, &nodeHelper );
  otp.parseObjectTree( originalMessage.get() );

  QCOMPARE( otp.textualContent().toAscii().data(), "encrypted message text" );
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

#include "unencryptedmessagetest.moc"