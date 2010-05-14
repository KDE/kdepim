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

#include <messageviewer/objecttreeparser.h>
#include <messageviewer/objecttreeemptysource.h>
#include <messageviewer/nodehelper.h>
#include <messageviewer/csshelper.h>

#include <messagecore/nodehelper.h>

#include <KDebug>
#include <QDir>
#include <QFile>

#include <stdlib.h>
#include <gpgme++/keylistresult.h>

std::vector<GpgME::Key> ComposerTestUtil::getKeys( bool smime )
{

  setenv("GNUPGHOME", KDESRCDIR "/gnupg_home" , 1 );
  setenv("LC_ALL", "C", 1); 
  setenv("KDEHOME", QFile::encodeName( QDir::homePath() + QString::fromAscii( "/.kde-unit-test" ) ), 1);

  Kleo::KeyListJob * job = 0;

  if( smime ) {
    const Kleo::CryptoBackend::Protocol * const backend = Kleo::CryptoBackendFactory::instance()->protocol( "smime" );
    job = backend->keyListJob( false );
  } else {
    const Kleo::CryptoBackend::Protocol * const backend = Kleo::CryptoBackendFactory::instance()->protocol( "openpgp" );
    job = backend->keyListJob( false );
  }
  Q_ASSERT( job );

  std::vector< GpgME::Key > keys;
  GpgME::KeyListResult res = job->exec( QStringList(), true, keys );

  if( !smime )
    Q_ASSERT( keys.size() == 3 );

  Q_ASSERT( !res.error() );
  kDebug() << "got private keys:" << keys.size();

  for(std::vector< GpgME::Key >::iterator i = keys.begin(); i != keys.end(); ++i ) {
    kDebug() << "key isnull:" << i->isNull() << "isexpired:" << i->isExpired();
    kDebug() << "key numuserIds:" << i->numUserIDs();
    for(uint k = 0; k < i->numUserIDs(); ++k ) {
      kDebug() << "userIDs:" << i->userID( k ).email();
    }
  }

  return keys;
}

bool ComposerTestUtil::verifySignature( KMime::Content* content, QByteArray signedContent, Kleo::CryptoMessageFormat f ) {

  // store it in a KMime::Message, that's what OTP needs
  KMime::Message* resultMessage =  new KMime::Message;
  resultMessage->setContent( content->encodedContent() );
  resultMessage->parse();
  
  // parse the result and make sure it is valid in various ways
  TestHtmlWriter testWriter;
  TestCSSHelper testCSSHelper;
  TestObjectTreeSource testSource( &testWriter, &testCSSHelper );
  MessageViewer::NodeHelper* nh = new MessageViewer::NodeHelper;
  MessageViewer::ObjectTreeParser otp( &testSource, nh, 0, false, false, true, 0 );
  MessageViewer::ProcessResult pResult( nh );

  // ensure the signed part exists and is parseable
  if( f & Kleo::OpenPGPMIMEFormat ) {
    KMime::Content* signedPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pgp-signature", true, true );
    Q_ASSERT( signedPart );

    // process the result..
    kDebug() << resultMessage->topLevel();
    otp.parseObjectTree( resultMessage );
    Q_ASSERT( nh->signatureState( resultMessage ) == MessageViewer::KMMsgFullySigned );

    // make sure the good sig is of what we think it is
    Q_ASSERT( MessageCore::NodeHelper::firstChild( resultMessage )->body() == signedContent );

    return true;
  } else if( f & Kleo::InlineOpenPGPFormat ) {
    otp.processTextPlainSubtype( resultMessage, pResult );

    otp.writeBodyString( resultMessage->encodedContent(),
                           resultMessage->from()->asUnicodeString() ,
                           nh->codec( resultMessage ),
                           pResult, true );
                          
    Q_ASSERT( pResult.inlineSignatureState() == MessageViewer::KMMsgPartiallySigned );

    return true;
  } else if( f & Kleo::AnySMIME ) {
    if( f & Kleo::SMIMEFormat ) {
      KMime::Content* signedPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pkcs7-signature", true, true );
      Q_ASSERT( signedPart );
    } else if( f & Kleo::SMIMEOpaqueFormat ) {
      KMime::Content* signedPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pkcs7-mime", true, true );
      Q_ASSERT( signedPart );
    }
    // process the result..
    otp.parseObjectTree( resultMessage );
    Q_ASSERT( nh->signatureState( resultMessage ) == MessageViewer::KMMsgFullySigned );

    // make sure the good sig is of what we think it is
    Q_ASSERT( otp.rawReplyString() == signedContent );

    return true;
  }

  return false;
}

bool ComposerTestUtil::verifyEncryption( KMime::Content* content, QByteArray encrContent, Kleo::CryptoMessageFormat f )
{

  // store it in a KMime::Message, that's what OTP needs
  KMime::Message* resultMessage =  new KMime::Message;
  resultMessage->setContent( content->encodedContent() );
  resultMessage->parse();

  // parse the result and make sure it is valid in various ways
  TestHtmlWriter testWriter;
  TestCSSHelper testCSSHelper;
  TestObjectTreeSource testSource( &testWriter, &testCSSHelper );
  MessageViewer::NodeHelper* nh = new MessageViewer::NodeHelper;
  MessageViewer::ObjectTreeParser otp( &testSource, nh, 0, false, false, true, 0 );
  MessageViewer::ProcessResult pResult( nh );

  if( f & Kleo::OpenPGPMIMEFormat ) {
    // ensure the enc part exists and is parseable
    KMime::Content* encPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pgp-encrypted", true, true );
    Q_ASSERT( encPart );

    // process the result..
   // kDebug() << resultMessage->topLevel();
    otp.parseObjectTree( resultMessage );
    Q_ASSERT( nh->encryptionState( resultMessage ) == MessageViewer::KMMsgFullyEncrypted );
//     kDebug() << "msg:" << resultMessage->encodedContent();

    // make sure the decoded content is what we encrypted
    // processMultiPartEncrypted will add a child part with the unencrypted data
    /*     // strip the extra newline from the end of the body
    QString body = QString::fromUtf8( MessageViewer::NodeHelper::firstChild( resultMessage )->encodedContent() ).trimmed();
    QString ref = QString::fromUtf8( encrContent ).trimmed();
    Q_ASSERT( body == ref );
*/
//     kDebug() << "raw:" << otp.rawReplyString();
//     kDebug() << "textContent:" << otp.textualContent();
    Q_ASSERT( otp.rawReplyString()== encrContent );

    return true;
    
  } else if( f & Kleo::InlineOpenPGPFormat ) {
    otp.processTextPlainSubtype( resultMessage, pResult );

    otp.writeBodyString( resultMessage->encodedContent(),
                           resultMessage->from()->asUnicodeString() ,
                           nh->codec( resultMessage ),
                           pResult, true );

    Q_ASSERT( pResult.inlineEncryptionState() == MessageViewer::KMMsgPartiallyEncrypted );

    return true;
  } else if( f & Kleo::AnySMIME) {
    // ensure the enc part exists and is parseable
    KMime::Content* encPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pkcs7-mime", true, true );
    Q_ASSERT( encPart );
    otp.parseObjectTree( resultMessage );
    Q_ASSERT( nh->encryptionState( resultMessage ) == MessageViewer::KMMsgFullyEncrypted );
    Q_ASSERT( otp.rawReplyString() == encrContent );

    return true;
  }

  return false;
}

bool ComposerTestUtil::verifySignatureAndEncryption( KMime::Content* content, QByteArray origContent, Kleo::CryptoMessageFormat f, bool withAttachment )
{
  // store it in a KMime::Message, that's what OTP needs
  KMime::Message::Ptr resultMessage =  KMime::Message::Ptr( new KMime::Message );
  resultMessage->setContent( content->encodedContent() );
  resultMessage->parse();
  
  // parse the result and make sure it is valid in various ways
  TestHtmlWriter testWriter;
  TestCSSHelper testCSSHelper;
  TestObjectTreeSource testSource( &testWriter, &testCSSHelper );
  MessageViewer::NodeHelper* nh = new MessageViewer::NodeHelper;
  MessageViewer::ObjectTreeParser otp( &testSource, nh, 0, false, false, true, 0  );
  MessageViewer::ProcessResult pResult( nh );

  if( f & Kleo::OpenPGPMIMEFormat ) {
    // ensure the enc part exists and is parseable
    KMime::Content* encPart = MessageViewer::ObjectTreeParser::findType( resultMessage.get(), "application", "pgp-encrypted", true, true );
    Q_ASSERT( encPart );

    otp.parseObjectTree( resultMessage.get() );
//     kDebug() << "message:" << resultMessage->encodedContent();
    Q_ASSERT( nh->encryptionState( resultMessage.get() ) == MessageViewer::KMMsgFullyEncrypted );

    KMime::Content* signedPart = MessageCore::NodeHelper::nextSibling( encPart );
//       kDebug() << "resultMessage:" << resultMessage->encodedContent();
//       kDebug() << "resultMessage rawresultstr:" << otp.rawReplyString() << "signed?" << resultMessage;
//       kDebug() << "signed Part:" << signedPart->encodedContent() << "Signed?" << ( nh->signatureState( signedPart ) == MessageViewer::KMMsgFullySigned );
    QList< KMime::Content* > extra = nh->extraContents( resultMessage.get() );
    kDebug() << "got extra:" << extra.size() << "for" << resultMessage;
    foreach( KMime::Content* e, extra )
        kDebug() << "extran:" << e->encodedContent();
    Q_ASSERT( extra.size() == 1 );
    Q_ASSERT( nh->signatureState( extra[ 0 ]  ) == MessageViewer::KMMsgFullySigned );

//     kDebug() << "raw reply and orig: " << otp.rawReplyString() << origContent;
    Q_ASSERT( otp.rawReplyString() == origContent );
    return true;
  } else if( f & Kleo::InlineOpenPGPFormat ) {
    otp.processTextPlainSubtype( resultMessage.get(), pResult );

    otp.writeBodyString( resultMessage->encodedContent(),
                           resultMessage->from()->asUnicodeString() ,
                           nh->codec( resultMessage.get() ),
                           pResult, true );

    Q_ASSERT( pResult.inlineEncryptionState() == MessageViewer::KMMsgPartiallyEncrypted );
    Q_ASSERT( pResult.inlineSignatureState() == MessageViewer::KMMsgPartiallySigned );

    return true;
  } else if( f & Kleo::AnySMIME ) {
    KMime::Content* encPart = MessageViewer::ObjectTreeParser::findType( resultMessage.get(), "application", "pkcs7-mime", true, true );
    Q_ASSERT( encPart );

    otp.parseObjectTree( resultMessage.get() );
    Q_ASSERT( nh->encryptionState( resultMessage.get() ) == MessageViewer::KMMsgFullyEncrypted );

    QList< KMime::Content* > extra = nh->extraContents( resultMessage.get() );
    kDebug() << "size:" << extra.size();
    Q_ASSERT( nh->signatureState( extra[ extra.size() - 1 ] ) == MessageViewer::KMMsgFullySigned );
    Q_ASSERT( otp.rawReplyString() == origContent );

    return true;
  }

  return false;
}

