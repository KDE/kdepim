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

#include <messagecore/attachmentpart.h>
#include <boost/shared_ptr.hpp>
using KPIM::AttachmentPart;

#include <gpgme++/key.h>

QTEST_KDEMAIN( ComposerTest, NoGUI )

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

/*
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


  QVERIFY( ComposerTestUtil::verifySignatureAndEncryption( message, QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ).toUtf8(),
                                          Kleo::OpenPGPMIMEFormat ) );

  QVERIFY( message->from()->asUnicodeString() == QString::fromLocal8Bit( "me@me.me" ) );
  QVERIFY( message->to()->asUnicodeString() == QString::fromLocal8Bit( "you@you.you" ) );

}

void ComposerTest::testSignEncryptLateAttachmentsOpenPGPMime()
{


}*/
    
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

  QStringList recipients;
  recipients << QString::fromLocal8Bit( "you@you.you" );

  QList<QPair<QStringList, std::vector<GpgME::Key> > > data;
  data.append( QPair<QStringList, std::vector<GpgME::Key> >( recipients, keys ) );

  composer->setEncryptionKeys( data );
  composer->setSigningKeys( keys );
}

#include "composertest.moc"
