/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
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

#include "cryptocomposertest.h"

#include "qtest_messagecomposer.h"
#include "cryptofunctions.h"

#include <KDebug>
#include <qtest_kde.h>

#include <kleo/enum.h>

#include <kmime/kmime_headers.h>
using namespace KMime;

#include <messagecomposer/composer/composer.h>
#include <messagecomposer/part/globalpart.h>
#include <messagecomposer/part/infopart.h>
#include <messagecomposer/part/textpart.h>
using namespace MessageComposer;

#include <messagecore/helpers/nodehelper.h>

#include <messageviewer/viewer/objecttreeparser.h>
#include <messagecore/tests/util.h>
#include <messageviewer/viewer/nodehelper.h>


#include <messagecore/attachment/attachmentpart.h>
#include <boost/shared_ptr.hpp>
using MessageCore::AttachmentPart;

#include <gpgme++/key.h>

QTEST_KDEMAIN( CryptoComposerTest, GUI )

void CryptoComposerTest::initTestCase()
{
  MessageCore::Test::setupEnv();
}

void CryptoComposerTest::testSignOpenPGPMime()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  composer->setSignAndEncrypt( true, false );
  composer->setMessageCryptoFormat( Kleo::OpenPGPMIMEFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message::Ptr message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  QVERIFY( ComposerTestUtil::verifySignature( message.get(), QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::OpenPGPMIMEFormat, Headers::CE7Bit ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
}


void CryptoComposerTest::testEncryptOpenPGPMime()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  composer->setSignAndEncrypt( false, true );
  composer->setMessageCryptoFormat( Kleo::OpenPGPMIMEFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message::Ptr message = composer->resultMessages().first();
  delete composer;
  composer = 0;


  QVERIFY( ComposerTestUtil::verifyEncryption( message.get(), QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
}

void CryptoComposerTest::testSignEncryptOpenPGPMime()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  composer->setSignAndEncrypt( true, true );
  composer->setMessageCryptoFormat( Kleo::OpenPGPMIMEFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message::Ptr message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( message.get(), QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(), Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
}


void CryptoComposerTest::testSignEncryptSameAttachmentsOpenPGPMime()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );
  
  AttachmentPart::Ptr attachment = AttachmentPart::Ptr( new AttachmentPart );
  attachment->setData( "abc" );
  attachment->setMimeType( "x-some/x-type" );
  attachment->setFileName( QString::fromLocal8Bit( "anattachment.txt" ) );
  attachment->setEncrypted( true );
  attachment->setSigned( true );
  composer->addAttachmentPart( attachment );
  
  composer->setSignAndEncrypt( true, true );
  composer->setMessageCryptoFormat( Kleo::OpenPGPMIMEFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message::Ptr message = composer->resultMessages().first();
  delete composer;
  composer = 0;


  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( message.get(),
                                                           QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                                           Kleo::OpenPGPMIMEFormat, true ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );

}

void CryptoComposerTest::testSignEncryptLateAttachmentsOpenPGPMime()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  AttachmentPart::Ptr attachment = AttachmentPart::Ptr( new AttachmentPart );
  attachment->setData( "abc" );
  attachment->setMimeType( "x-some/x-type" );
  attachment->setFileName( QString::fromLocal8Bit( "anattachment.txt" ) );
  attachment->setEncrypted( false );
  attachment->setSigned( false );
  composer->addAttachmentPart( attachment );

  composer->setSignAndEncrypt( true, true );
  composer->setMessageCryptoFormat( Kleo::OpenPGPMIMEFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message::Ptr message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  // as we have an additional attachment, just ignore it when checking for sign/encrypt
  KMime::Content * b = MessageCore::NodeHelper::firstChild( message.get() );
  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( b,
                                                           QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                                           Kleo::OpenPGPMIMEFormat, true ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );

  // now check the attachment separately
  kDebug() << "message:" << message->encodedContent();
  QVERIFY( MessageCore::NodeHelper::nextSibling( MessageCore::NodeHelper::firstChild( message.get() ) )->body() == "abc" );
//   kDebug() << "attachment:" << attNode->encodedContent();

}


void CryptoComposerTest::testBCCEncrypt()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  composer->infoPart()->setBcc( QStringList( QString::fromLatin1( "bcc@bcc.org" ) ) );

  std::vector<GpgME::Key> keys = MessageCore::Test::getKeys();

  QStringList primRecipients;
  primRecipients << QString::fromLocal8Bit( "you@you.you" );
  std::vector< GpgME::Key > pkeys;
  pkeys.push_back( keys[1] );

  QStringList secondRecipients;
  secondRecipients << QString::fromLocal8Bit( "bcc@bcc.org" );
  std::vector< GpgME::Key > skeys;
  skeys.push_back( keys[2] );

  QList<QPair<QStringList, std::vector<GpgME::Key> > > data;
  data.append( QPair<QStringList, std::vector<GpgME::Key> >( primRecipients, pkeys ) );
  data.append( QPair<QStringList, std::vector<GpgME::Key> >( secondRecipients, skeys ) );
  
  composer->setSignAndEncrypt( true, true );
  composer->setMessageCryptoFormat( Kleo::OpenPGPMIMEFormat );

  composer->setEncryptionKeys( data );
  composer->setSigningKeys( keys );
  
  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 2 );

  KMime::Message::Ptr primMessage = composer->resultMessages().first();
  KMime::Message::Ptr secMessage = composer->resultMessages()[1];
  delete composer;
  composer = 0;

  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( primMessage.get(), QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(), Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( primMessage->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( primMessage->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
  

  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( secMessage.get(), QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(), Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( secMessage->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( secMessage->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );

}


void CryptoComposerTest::testSignInlinePGP()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  composer->setSignAndEncrypt( true, false );
  composer->setMessageCryptoFormat( Kleo::InlineOpenPGPFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message::Ptr message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  kDebug() << message->encodedContent();

  QVERIFY( ComposerTestUtil::verifySignature( message.get(), QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::InlineOpenPGPFormat, Headers::CE7Bit ) );
                                          
/*
  QVERIFY( ComposerTestUtil::verifySignature( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
  */
}

void CryptoComposerTest::testEncryptInlinePGP()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  composer->setSignAndEncrypt( false, true );
  composer->setMessageCryptoFormat( Kleo::InlineOpenPGPFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message::Ptr message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  kDebug() << message->encodedContent();

  QVERIFY( ComposerTestUtil::verifyEncryption( message.get(), QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::InlineOpenPGPFormat ) );

                                          /*
  QVERIFY( ComposerTestUtil::verifySignature( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
  */
}


void CryptoComposerTest::testSignEncryptInlinePGP()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  composer->setSignAndEncrypt( true, true );
  composer->setMessageCryptoFormat( Kleo::InlineOpenPGPFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message::Ptr message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  kDebug() << message->encodedContent();

  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( message.get(), QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(), Kleo::InlineOpenPGPFormat ) );
/*
  QVERIFY( ComposerTestUtil::verifySignature( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
  */
}

void CryptoComposerTest::testSignSMIME()
{
  runSMIMETest( true, false, false );
}

void CryptoComposerTest::testEncryptSMIME() {
  // Disable the test, for me it always hangs with
  // "Message::EncryptJob::process: HELP! Encrypt job but have no keys to encrypt with."
  // Probably a test setup problem.
  //runSMIMETest( false, true, false );
}

void CryptoComposerTest::testSignEncryptSMIME() {
  runSMIMETest( true, true, false );
  
}

void CryptoComposerTest::testSignSMIMEOpaque()
{
  runSMIMETest( true, false, true );
}

void CryptoComposerTest::testEncryptSMIMEOpaque() {
  // Disable the test, for me it always hangs with
  // "Message::EncryptJob::process: HELP! Encrypt job but have no keys to encrypt with."
  // Probably a test setup problem.
  //runSMIMETest( false, true, true );
}

void CryptoComposerTest::testSignEncryptSMIMEOpaque() {
  // Disable the test, for me it always hangs with
  // "Message::EncryptJob::process: HELP! Encrypt job but have no keys to encrypt with."
  // Probably a test setup problem.
  //runSMIMETest( true, true, true );
}


void CryptoComposerTest::fillComposerData( Composer* composer )
{
  composer->globalPart()->setFallbackCharsetEnabled( true );
  composer->infoPart()->setFrom( QString::fromLatin1( "me@me.me" ) );
  composer->infoPart()->setTo( QStringList( QString::fromLatin1( "you@you.you" ) ) );
  composer->textPart()->setWrappedPlainText( QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ) );
}

void CryptoComposerTest::fillComposerCryptoData( Composer* composer )
{
  std::vector<GpgME::Key> keys = MessageCore::Test::getKeys();

  kDebug() << "got num of keys:" << keys.size();
  
  QStringList recipients;
  recipients << QString::fromLocal8Bit( "you@you.you" );

  QList<QPair<QStringList, std::vector<GpgME::Key> > > data;
  data.append( QPair<QStringList, std::vector<GpgME::Key> >( recipients, keys ) );

  composer->setEncryptionKeys( data );
  composer->setSigningKeys( keys );
}

void CryptoComposerTest::runSMIMETest( bool sign, bool enc, bool opaque )
{

  Composer *composer = new Composer;
  fillComposerData( composer );
  composer->infoPart()->setFrom( QString::fromLatin1( "test@example.com" ) );

  std::vector<GpgME::Key> keys = MessageCore::Test::getKeys( true );
  QStringList recipients;
  recipients << QString::fromLocal8Bit( "you@you.you" );
  QList<QPair<QStringList, std::vector<GpgME::Key> > > data;
  data.append( QPair<QStringList, std::vector<GpgME::Key> >( recipients, keys ) );
  composer->setEncryptionKeys( data );
  composer->setSigningKeys( keys );
  composer->setSignAndEncrypt( sign, enc );
  Kleo::CryptoMessageFormat f;
  if( opaque ) {
    f = Kleo::SMIMEOpaqueFormat;
  } else {
    f = Kleo::SMIMEFormat;
  }
  composer->setMessageCryptoFormat( f );

  const bool result = composer->exec();
  QEXPECT_FAIL("", "GPG setup problems", Continue);
  QVERIFY( result );
  if ( result ) {
    QCOMPARE( composer->resultMessages().size(), 1 );
    KMime::Message::Ptr message = composer->resultMessages().first();
    delete composer;
    composer = 0;

    kDebug() << "message:" << message->encodedContent();

    if( sign && !enc ) {
      QVERIFY( ComposerTestUtil::verifySignature( message.get(), QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),  f, Headers::CE7Bit ) );
    } else if( !sign && enc ) {
      QVERIFY( ComposerTestUtil::verifyEncryption( message.get(), QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),  f ) );
    } else if( sign && enc ) {
      QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( message.get(), QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(), f ) );
    }

    QCOMPARE( message->from()->asUnicodeString(), QString::fromLocal8Bit( "test@example.com" ) );
    QCOMPARE( message->to()->asUnicodeString(), QString::fromLocal8Bit( "you@you.you" ) );
  }
}

#include "cryptocomposertest.moc"
