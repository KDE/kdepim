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

#include <kleo/enum.h>
#include <kleo/keylistjob.h>
#include <kleo/cryptobackendfactory.h>
#include <kmime/kmime_message.h>
#include <kmime/kmime_content.h>

#include <messageviewer/objecttreeparser.h>
#include <messageviewer/objecttreeemptysource.h>
#include <messageviewer/nodehelper.h>

#include <KDebug>
#include <QDir>
#include <QFile>

#include <stdlib.h>
#include <gpgme++/keylistresult.h>

std::vector<GpgME::Key> ComposerTestUtil::getKeys()
{

  setenv("GNUPGHOME", KDESRCDIR "/gnupg_home" , 1 );
  setenv("LC_ALL", "C", 1); \
  setenv("KDEHOME", QFile::encodeName( QDir::homePath() + QString::fromAscii( "/.kde-unit-test" ) ), 1);

  const Kleo::CryptoBackend::Protocol * const backend = Kleo::CryptoBackendFactory::instance()->protocol( "openpgp" );
  Kleo::KeyListJob * job = backend->keyListJob( false );
  Q_ASSERT( job );

  std::vector< GpgME::Key > keys;
  GpgME::KeyListResult res = job->exec( QStringList(), true, keys );

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
  MessageViewer::EmptySource es;
  MessageViewer::NodeHelper* nh = new MessageViewer::NodeHelper;
  MessageViewer::ObjectTreeParser otp( &es, nh, 0, false, false, true );

  // ensure the signed part exists and is parseable
  if( f & Kleo::OpenPGPMIMEFormat ) {
    KMime::Content* signedPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pgp-signature", true, true );
    Q_ASSERT( signedPart );

    // process the result..
    MessageViewer::ProcessResult pResult( nh );
    kDebug() << resultMessage->topLevel();
    otp.parseObjectTree( resultMessage );
    Q_ASSERT( nh->signatureState( resultMessage ) == MessageViewer::KMMsgFullySigned );

    // make sure the good sig is of what we think it is
    Q_ASSERT( MessageViewer::NodeHelper::firstChild( resultMessage )->body() == signedContent );

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
  MessageViewer::EmptySource es;
  MessageViewer::NodeHelper* nh = new MessageViewer::NodeHelper;
  MessageViewer::ObjectTreeParser otp( &es, nh, 0, false, false, true );

  if( f & Kleo::OpenPGPMIMEFormat ) {
    // ensure the enc part exists and is parseable
    KMime::Content* encPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pgp-encrypted", true, true );
    Q_ASSERT( encPart );

    // process the result..
    MessageViewer::ProcessResult pResult( nh );
  //   kDebug() << resultMessage->topLevel();
    otp.parseObjectTree( resultMessage );
    Q_ASSERT( nh->encryptionState( resultMessage ) == MessageViewer::KMMsgFullyEncrypted );

    // make sure the decoded content is what we encrypted
    // processMultiPartEncrypted will add a child part with the unencrypted data
    Q_ASSERT( MessageViewer::NodeHelper::firstChild( resultMessage )->body() == encrContent );

    return true;
    
  }

  return false;
}

bool ComposerTestUtil::verifySignatureAndEncryption( KMime::Content* content, QByteArray origContent, Kleo::CryptoMessageFormat f, bool withAttachment )
{
  // store it in a KMime::Message, that's what OTP needs
  KMime::Message* resultMessage =  new KMime::Message;
  resultMessage->setContent( content->encodedContent() );
  resultMessage->parse();
  
  // parse the result and make sure it is valid in various ways
  MessageViewer::EmptySource es;
  MessageViewer::NodeHelper* nh = new MessageViewer::NodeHelper;
  MessageViewer::ObjectTreeParser otp( &es, nh, 0, false, false, true );

  if( f & Kleo::OpenPGPMIMEFormat ) {
    // ensure the enc part exists and is parseable
    KMime::Content* encPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pgp-encrypted", true, true );
    Q_ASSERT( encPart );

    MessageViewer::ProcessResult pResult( nh );
    otp.parseObjectTree( resultMessage );
//     kDebug() << "message:" << resultMessage->encodedContent();
    Q_ASSERT( nh->encryptionState( resultMessage ) == MessageViewer::KMMsgFullyEncrypted );

    KMime::Content* signedPart = MessageViewer::NodeHelper::firstChild( resultMessage );
//       kDebug() << "resultMessage signedpart:" << signedPart->encodedContent();
    otp.processMultiPartSignedSubtype( signedPart, pResult );
    Q_ASSERT( nh->signatureState( signedPart ) == MessageViewer::KMMsgFullySigned );

    if( withAttachment ) {
      // if there's an attachment we need to dig deeper
      Q_ASSERT( MessageViewer::NodeHelper::firstChild( MessageViewer::NodeHelper::firstChild( signedPart ) )->body() == origContent );
    } else {
      Q_ASSERT( MessageViewer::NodeHelper::firstChild( signedPart )->body() == origContent );
    }
    return true;
  }

  return false;
}

