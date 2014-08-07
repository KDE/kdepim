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

#include "cryptofunctions.h"

#include "testhtmlwriter.h"
#include "testcsshelper.h"

#include <kleo/enum.h>
#include <kleo/keylistjob.h>
#include <kleo/cryptobackendfactory.h>
#include <kmime/kmime_message.h>
#include <kmime/kmime_content.h>

#include <messageviewer/viewer/objecttreeparser.h>
#include <messageviewer/viewer/objecttreeemptysource.h>
#include <messageviewer/viewer/csshelper.h>

#include <messagecore/helpers/nodehelper.h>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QtTest>

#include <stdlib.h>
#include <gpgme++/keylistresult.h>
#include <messagecore/autotests/util.h>

void ComposerTestUtil::verify( bool sign, bool encrypt, KMime::Content* content, QByteArray origContent, Kleo::CryptoMessageFormat f, KMime::Headers::contentEncoding encoding ) {
  if ( sign && encrypt ) {
    verifySignatureAndEncryption( content, origContent, f, false );
  } else if ( sign ) {
    verifySignature( content, origContent, f, encoding );
  } else {
    verifyEncryption(  content, origContent, f );
  }
}

void ComposerTestUtil::verifySignature( KMime::Content* content, QByteArray signedContent, Kleo::CryptoMessageFormat f, KMime::Headers::contentEncoding encoding ) {
  qDebug() << "verifySignature";
  // store it in a KMime::Message, that's what OTP needs
  KMime::Message* resultMessage =  new KMime::Message;
  resultMessage->setContent( content->encodedContent() );
  resultMessage->parse();

  // parse the result and make sure it is valid in various ways
  TestHtmlWriter testWriter;
  TestCSSHelper testCSSHelper;
  MessageCore::Test::TestObjectTreeSource testSource( &testWriter, &testCSSHelper );
  MessageViewer::NodeHelper* nh = new MessageViewer::NodeHelper;
  MessageViewer::ObjectTreeParser otp( &testSource, nh );
  MessageViewer::ProcessResult pResult( nh );

  // ensure the signed part exists and is parseable
  if( f & Kleo::OpenPGPMIMEFormat ) {
    // process the result..
    otp.parseObjectTree( resultMessage );

    KMime::Content* signedPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pgp-signature", true, true );
    Q_ASSERT( signedPart );
    QCOMPARE( signedPart->contentTransferEncoding()->encoding(), KMime::Headers::CE7Bit );
    Q_UNUSED( signedPart );

    //Q_ASSERT( nh->signatureState( resultMessage ) == MessageViewer::KMMsgFullySigned );

    QCOMPARE( MessageCore::NodeHelper::firstChild( resultMessage )->contentTransferEncoding()->encoding(), encoding );
  } else if( f & Kleo::InlineOpenPGPFormat ) {
    // process the result..
    otp.parseObjectTree( resultMessage );
    QCOMPARE( nh->signatureState( resultMessage ), MessageViewer::KMMsgFullySigned );

    QCOMPARE( resultMessage->contentTransferEncoding()->encoding(), encoding );
  } else if( f & Kleo::AnySMIME ) {
    if( f & Kleo::SMIMEFormat ) {
      KMime::Content* signedPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pkcs7-signature", true, true );
      Q_ASSERT( signedPart );
      QCOMPARE( signedPart->contentTransferEncoding()->encoding(), KMime::Headers::CEbase64 );
      QCOMPARE( signedPart->contentType()->mimeType(), QByteArray( "application/pkcs7-signature" ) );
      QCOMPARE( signedPart->contentType()->name(), QString::fromLatin1( "smime.p7s" ) );
      QCOMPARE( signedPart->contentDisposition()->disposition(), KMime::Headers::CDattachment );
      QCOMPARE( signedPart->contentDisposition()->filename(), QString::fromLatin1( "smime.p7s" ) );
      Q_UNUSED( signedPart );

      QCOMPARE( MessageCore::NodeHelper::firstChild( resultMessage )->contentTransferEncoding()->encoding(), encoding );

      QCOMPARE( resultMessage->contentType()->mimeType(), QByteArray( "multipart/signed" ) );
      QCOMPARE( resultMessage->contentType()->parameter( QString::fromLatin1( "protocol" ) ), QString::fromLatin1( "application/pkcs7-signature" ) );
      QCOMPARE( resultMessage->contentType()->parameter( QString::fromLatin1( "micalg" ) ), QString::fromLatin1( "sha1" ) );

    } else if( f & Kleo::SMIMEOpaqueFormat ) {
      KMime::Content* signedPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pkcs7-mime", true, true );
      Q_ASSERT( signedPart );
      QCOMPARE( signedPart->contentTransferEncoding()->encoding(), KMime::Headers::CEbase64 );
      QCOMPARE( signedPart->contentType()->mimeType(), QByteArray( "application/pkcs7-mime" ) );
      QCOMPARE( signedPart->contentType()->name(), QString::fromLatin1( "smime.p7m" ) );
      QCOMPARE( signedPart->contentType()->parameter( QString::fromLatin1( "smime-type" ) ), QString::fromLatin1( "signed-data" ) );
      QCOMPARE( signedPart->contentDisposition()->disposition(), KMime::Headers::CDattachment );
      QCOMPARE( signedPart->contentDisposition()->filename(), QString::fromLatin1( "smime.p7m" ) );
      Q_UNUSED( signedPart );
    }
    // process the result..
    otp.parseObjectTree( resultMessage );

    //Q_ASSERT( nh->signatureState( resultMessage ) == MessageViewer::KMMsgFullySigned );
  }

  // make sure the good sig is of what we think it is
  QCOMPARE( otp.plainTextContent(), QString::fromUtf8( signedContent ) );
  Q_UNUSED( signedContent );

}

void ComposerTestUtil::verifyEncryption( KMime::Content* content, QByteArray encrContent, Kleo::CryptoMessageFormat f , bool withAttachment)
{
  // store it in a KMime::Message, that's what OTP needs
  KMime::Message::Ptr resultMessage = KMime::Message::Ptr( new KMime::Message );
  resultMessage->setContent( content->encodedContent() );
  resultMessage->parse();

  // parse the result and make sure it is valid in various ways
  TestHtmlWriter testWriter;
  TestCSSHelper testCSSHelper;
  MessageCore::Test::TestObjectTreeSource testSource( &testWriter, &testCSSHelper );
  testSource.setAllowDecryption( true );
  MessageViewer::NodeHelper* nh = new MessageViewer::NodeHelper;
  MessageViewer::ObjectTreeParser otp( &testSource, nh );
  MessageViewer::ProcessResult pResult( nh );

  if( f & Kleo::OpenPGPMIMEFormat ) {
    // ensure the enc part exists and is parseable
    KMime::Content* encPart = MessageViewer::ObjectTreeParser::findType( resultMessage.get(), "application", "pgp-encrypted", true, true );
    Q_ASSERT( encPart );
    Q_UNUSED( encPart );

    // process the result..
    otp.parseObjectTree( resultMessage.get() );
    QCOMPARE( nh->encryptionState( resultMessage.get() ), MessageViewer::KMMsgFullyEncrypted );

  } else if( f & Kleo::InlineOpenPGPFormat ) {
    if (  withAttachment ) {
      //Only first MimePart is the encrypted Text
      KMime::Content *cContent = MessageCore::NodeHelper::firstChild(  resultMessage.get() );
      resultMessage->setContent( cContent->encodedContent() );
      resultMessage->parse();
    }

    otp.processTextPlainSubtype( resultMessage.get(), pResult );

    QCOMPARE( pResult.inlineEncryptionState(), MessageViewer::KMMsgFullyEncrypted );

  } else if( f & Kleo::AnySMIME) {
    // ensure the enc part exists and is parseable
    KMime::Content* encPart = MessageViewer::ObjectTreeParser::findType( resultMessage.get(), "application", "pkcs7-mime", true, true );
    Q_ASSERT( encPart );

    QCOMPARE( encPart->contentType()->mimeType(), QByteArray( "application/pkcs7-mime" ) );
    QCOMPARE( encPart->contentType()->name(), QString::fromLatin1( "smime.p7m" ) );
    QCOMPARE( encPart->contentType()->parameter( QString::fromLatin1( "smime-type" ) ), QString::fromLatin1( "enveloped-data" ) );
    QCOMPARE( encPart->contentDisposition()->disposition(), KMime::Headers::CDattachment );
    QCOMPARE( encPart->contentDisposition()->filename(), QString::fromLatin1( "smime.p7m" ) );
    Q_UNUSED( encPart );

    otp.parseObjectTree( resultMessage.get() );
    QCOMPARE( nh->encryptionState( resultMessage.get() ), MessageViewer::KMMsgFullyEncrypted );

  }
  QCOMPARE( otp.plainTextContent(), QString::fromUtf8( encrContent ) );
  Q_UNUSED( encrContent );
}

void ComposerTestUtil::verifySignatureAndEncryption( KMime::Content* content, QByteArray origContent, Kleo::CryptoMessageFormat f, bool withAttachment )
{
  Q_UNUSED( withAttachment );
  // store it in a KMime::Message, that's what OTP needs
  KMime::Message::Ptr resultMessage =  KMime::Message::Ptr( new KMime::Message );
  resultMessage->setContent( content->encodedContent() );
  resultMessage->parse();

  // parse the result and make sure it is valid in various ways
  TestHtmlWriter testWriter;
  TestCSSHelper testCSSHelper;
  MessageCore::Test::TestObjectTreeSource testSource( &testWriter, &testCSSHelper );
  testSource.setAllowDecryption( true );
  MessageViewer::NodeHelper* nh = new MessageViewer::NodeHelper;
  MessageViewer::ObjectTreeParser otp( &testSource, nh );
  MessageViewer::ProcessResult pResult( nh );

  if( f & Kleo::OpenPGPMIMEFormat ) {
    // ensure the enc part exists and is parseable
    KMime::Content* encPart = MessageViewer::ObjectTreeParser::findType( resultMessage.get(), "application", "pgp-encrypted", true, true );
    Q_ASSERT( encPart );
    Q_UNUSED( encPart );
    otp.parseObjectTree( resultMessage.get() );
    QCOMPARE( nh->encryptionState( resultMessage.get() ), MessageViewer::KMMsgFullyEncrypted );

    QList< KMime::Content* > extra = nh->extraContents( resultMessage.get() );
    qDebug() << "size:" << extra.size();
    QCOMPARE( extra.size(), 1 );
    QCOMPARE( nh->signatureState( extra[ 0 ]  ), MessageViewer::KMMsgFullySigned );
  } else if( f & Kleo::InlineOpenPGPFormat ) {
    otp.processTextPlainSubtype( resultMessage.get(), pResult );

    QCOMPARE( pResult.inlineEncryptionState(), MessageViewer::KMMsgFullyEncrypted );
    QCOMPARE( pResult.inlineSignatureState(), MessageViewer::KMMsgFullySigned );
  } else if( f & Kleo::AnySMIME ) {
    KMime::Content* encPart = MessageViewer::ObjectTreeParser::findType( resultMessage.get(), "application", "pkcs7-mime", true, true );
    Q_ASSERT( encPart );
    QCOMPARE( encPart->contentType()->mimeType(), QByteArray( "application/pkcs7-mime" ) );
    QCOMPARE( encPart->contentType()->name(), QString::fromLatin1( "smime.p7m" ) );
    QCOMPARE( encPart->contentType()->parameter( QString::fromLatin1( "smime-type" ) ), QString::fromLatin1( "enveloped-data" ) );
    QCOMPARE( encPart->contentDisposition()->disposition(), KMime::Headers::CDattachment );
    QCOMPARE( encPart->contentDisposition()->filename(), QString::fromLatin1( "smime.p7m" ) );
    Q_UNUSED( encPart );

    otp.parseObjectTree( resultMessage.get() );
    QCOMPARE( nh->encryptionState( resultMessage.get() ), MessageViewer::KMMsgFullyEncrypted );

    QList< KMime::Content* > extra = nh->extraContents( resultMessage.get() );
    qDebug() << "size:" << extra.size();
    QCOMPARE( extra.size(), 1 );
    QCOMPARE( nh->signatureState( extra[ 0 ] ), MessageViewer::KMMsgFullySigned );
  }
  QCOMPARE( otp.plainTextContent(), QString::fromUtf8( origContent ) );
  Q_UNUSED( origContent );
}

