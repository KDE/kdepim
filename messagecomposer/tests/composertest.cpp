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

#include "composertest.h"

#include "qtest_messagecomposer.h"
#include "cryptofunctions.h"

#include <KDebug>
#include <qtest_kde.h>

#include <kleo/enum.h>

#include <kmime/kmime_headers.h>
using namespace KMime;

#include <messagecomposer/composer.h>
#include <messagecomposer/globalpart.h>
#include <messagecomposer/infopart.h>
#include <messagecomposer/textpart.h>
using namespace Message;

#include <messagecore/nodehelper.h>

#include <messageviewer/objecttreeparser.h>
#include <messageviewer/nodehelper.h>


#include <messagecore/attachmentpart.h>
#include <boost/shared_ptr.hpp>
using KPIM::AttachmentPart;

#include <gpgme++/key.h>

QTEST_KDEMAIN( ComposerTest, GUI )

void ComposerTest::testAttachments()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  AttachmentPart::Ptr attachment = AttachmentPart::Ptr( new AttachmentPart );
  attachment->setData( "abc" );
  attachment->setMimeType( "x-some/x-type" );
  composer->addAttachmentPart( attachment );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );
  KMime::Message* message = composer->resultMessages().first();
  kDebug() << message->encodedContent();
  delete composer;
  composer = 0;

  // multipart/mixed
  {
    QVERIFY( message->contentType( false ) );
    QCOMPARE( message->contentType()->mimeType(), QByteArray( "multipart/mixed" ) );
    QCOMPARE( message->contents().count(), 2 );
    // text/plain
    {
      Content *plain = message->contents().at( 0 );
      QVERIFY( plain->contentType( false ) );
      QCOMPARE( plain->contentType()->mimeType(), QByteArray( "text/plain" ) );
    }
    // x-some/x-type (attachment)
    {
      Content *plain = message->contents().at( 1 );
      QVERIFY( plain->contentType( false ) );
      QCOMPARE( plain->contentType()->mimeType(), QByteArray( "x-some/x-type" ) );
    }
  }
}

void ComposerTest::testSignOpenPGPMime()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  composer->setSignAndEncrypt( true, false );
  composer->setMessageCryptoFormat( Kleo::OpenPGPMIMEFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message* message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  QVERIFY( ComposerTestUtil::verifySignature( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
}


void ComposerTest::testEncryptOpenPGPMime()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  composer->setSignAndEncrypt( false, true );
  composer->setMessageCryptoFormat( Kleo::OpenPGPMIMEFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message* message = composer->resultMessages().first();
  delete composer;
  composer = 0;


  QVERIFY( ComposerTestUtil::verifyEncryption( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
}

void ComposerTest::testSignEncryptOpenPGPMime()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  composer->setSignAndEncrypt( true, true );
  composer->setMessageCryptoFormat( Kleo::OpenPGPMIMEFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message* message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(), Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
}


void ComposerTest::testSignEncryptSameAttachmentsOpenPGPMime()
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

  KMime::Message* message = composer->resultMessages().first();
  delete composer;
  composer = 0;


  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( message,
                                                           QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                                           Kleo::OpenPGPMIMEFormat, true ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );

}

void ComposerTest::testSignEncryptLateAttachmentsOpenPGPMime()
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

  KMime::Message* message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  // as we have an additional attachment, just ignore it when checking for sign/encrypt
  KMime::Content * b = MessageCore::NodeHelper::firstChild( message );
  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( b,
                                                           QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                                           Kleo::OpenPGPMIMEFormat, true ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );

  // now check the attachment separately
  kDebug() << "message:" << message->encodedContent();
  QVERIFY( MessageCore::NodeHelper::nextSibling( MessageCore::NodeHelper::firstChild( message ) )->body() == "abc" );
//   kDebug() << "attachment:" << attNode->encodedContent();

}


void ComposerTest::testBCCEncrypt()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  composer->infoPart()->setBcc( QStringList( QString::fromLatin1( "bcc@bcc.org" ) ) );

  std::vector<GpgME::Key> keys = ComposerTestUtil::getKeys();

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

  KMime::Message* primMessage = composer->resultMessages().first();
  KMime::Message* secMessage = composer->resultMessages()[1];
  delete composer;
  composer = 0;

  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( primMessage, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(), Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( primMessage->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( primMessage->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
  

  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( secMessage, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(), Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( secMessage->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( secMessage->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
  QVERIFY( secMessage->bcc()->asUnicodeString() == QString::fromLocal8Bit( "bcc@bcc.org" ) );

}


void ComposerTest::testSignInlinePGP()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  composer->setSignAndEncrypt( true, false );
  composer->setMessageCryptoFormat( Kleo::InlineOpenPGPFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message* message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  kDebug() << message->encodedContent();

  QVERIFY( ComposerTestUtil::verifySignature( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::InlineOpenPGPFormat ) );
                                          
/*
  QVERIFY( ComposerTestUtil::verifySignature( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
  */
}

void ComposerTest::testEncryptInlinePGP()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  composer->setSignAndEncrypt( false, true );
  composer->setMessageCryptoFormat( Kleo::InlineOpenPGPFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message* message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  kDebug() << message->encodedContent();

  QVERIFY( ComposerTestUtil::verifyEncryption( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::InlineOpenPGPFormat ) );

                                          /*
  QVERIFY( ComposerTestUtil::verifySignature( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
  */
}


void ComposerTest::testSignEncryptInlinePGP()
{
  Composer *composer = new Composer;
  fillComposerData( composer );
  fillComposerCryptoData( composer );

  composer->setSignAndEncrypt( true, true );
  composer->setMessageCryptoFormat( Kleo::InlineOpenPGPFormat );

  QVERIFY( composer->exec() );
  QCOMPARE( composer->resultMessages().size(), 1 );

  KMime::Message* message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  kDebug() << message->encodedContent();

  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(), Kleo::InlineOpenPGPFormat ) );
/*
  QVERIFY( ComposerTestUtil::verifySignature( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );
  */
}

void ComposerTest::testSignSMIME()
{
  runSMIMETest( true, false, false );
}

void ComposerTest::testEncryptSMIME() {
  QVERIFY( runSMIMETest( false, true, false ) );
}

void ComposerTest::testSignEncryptSMIME() {
  QVERIFY( runSMIMETest( true, true, false ) );
  
}

void ComposerTest::testSignSMIMEOpaque()
{
  QVERIFY( runSMIMETest( true, false, true ) );
}

void ComposerTest::testEncryptSMIMEOpaque() {
  QVERIFY( runSMIMETest( false, true, true ) );
}

void ComposerTest::testSignEncryptSMIMEOpaque() {
   QVERIFY( runSMIMETest( true, true, true ) );

}


void ComposerTest::fillComposerData( Composer* composer )
{
  composer->globalPart()->setFallbackCharsetEnabled( true );
  composer->infoPart()->setFrom( QString::fromLatin1( "me@me.me" ) );
  composer->infoPart()->setTo( QStringList( QString::fromLatin1( "you@you.you" ) ) );
  composer->textPart()->setWrappedPlainText( QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ) );
}

void ComposerTest::fillComposerCryptoData( Composer* composer )
{
  std::vector<GpgME::Key> keys = ComposerTestUtil::getKeys();

  kDebug() << "got num of keys:" << keys.size();
  
  QStringList recipients;
  recipients << QString::fromLocal8Bit( "you@you.you" );

  QList<QPair<QStringList, std::vector<GpgME::Key> > > data;
  data.append( QPair<QStringList, std::vector<GpgME::Key> >( recipients, keys ) );

  composer->setEncryptionKeys( data );
  composer->setSigningKeys( keys );
}

bool ComposerTest::runSMIMETest( bool sign, bool enc, bool opaque )
{

  Composer *composer = new Composer;
  fillComposerData( composer );

  std::vector<GpgME::Key> keys = ComposerTestUtil::getKeys( true );
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
  
  Q_ASSERT( composer->exec() );
  Q_ASSERT( composer->resultMessages().size() == 1 );
  KMime::Message* message = composer->resultMessages().first();
  delete composer;
  composer = 0;

  kDebug() << "message:" << message->encodedContent();

  if( sign && !enc ) {
    Q_ASSERT( ComposerTestUtil::verifySignature( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),  f ) );
  } else if( !sign && enc ) {
    Q_ASSERT( ComposerTestUtil::verifyEncryption( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),  f ) );
  } else if( sign && enc ) {
    Q_ASSERT( ComposerTestUtil::verifySignatureAndEncryption( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(), f ) );
  }
    

  Q_ASSERT( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  Q_ASSERT( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );

  return true;

}

#include "composertest.moc"
