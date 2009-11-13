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

#include "encryptjobtest.h"

#include <KDebug>
#include <qtest_kde.h>
#include "qtest_messagecomposer.h"

#include <kmime/kmime_content.h>

#include <kleo/enum.h>
#include <kleo/keylistjob.h>
#include <kleo/cryptobackendfactory.h>
#include <kjob.h>

#include <messagecomposer/composer.h>
#include <messagecomposer/encryptjob.h>
#include <messagecomposer/transparentjob.h>

#include <messageviewer/objecttreeparser.h>
#include <messageviewer/objecttreeemptysource.h>
#include <messageviewer/nodehelper.h>

#include <stdlib.h>

QTEST_KDEMAIN( EncryptJobTest, NoGUI )

std::vector<GpgME::Key> EncryptJobTest::getKeys()
{

  setupEnv();

  const Kleo::CryptoBackend::Protocol * const backend = Kleo::CryptoBackendFactory::instance()->protocol( "openpgp" );
  Kleo::KeyListJob * job = backend->keyListJob( false );
  Q_ASSERT( job );

  std::vector< GpgME::Key > keys;
  GpgME::KeyListResult res = job->exec( QStringList(), true, keys );

  Q_ASSERT( keys.size() == 1 );
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

void EncryptJobTest::testContentDirect() {

  std::vector< GpgME::Key > keys = getKeys();

  Message::Composer *composer = new Message::Composer;
  Message::EncryptJob* eJob = new Message::EncryptJob( composer );

  QVERIFY( composer );
  QVERIFY( eJob );

  QByteArray data( QString::fromLocal8Bit( "one flew over the cuckoo's nest" ).toUtf8() );
  KMime::Content* content = new KMime::Content;
  content->setBody( data );

  QStringList recipients;
  recipients << QString::fromLocal8Bit( "test@kolab.org" );

  eJob->setContent( content );
  eJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );
  eJob->setRecipients( recipients );
  eJob->setEncryptionKeys( keys );
  
  bool encrWorked = checkEncryption( eJob );
  QVERIFY( encrWorked );

}


void EncryptJobTest::testContentChained()
{
  std::vector< GpgME::Key > keys = getKeys();

  QByteArray data( QString::fromLocal8Bit( "one flew over the cuckoo's nest" ).toUtf8() );
  KMime::Content* content = new KMime::Content;
  content->setBody( data );

  Message::TransparentJob* tJob =  new Message::TransparentJob;
  tJob->setContent( content );

  Message::Composer *composer = new Message::Composer;
  Message::EncryptJob* eJob = new Message::EncryptJob( composer );

  QStringList recipients;
  recipients << QString::fromLocal8Bit( "test@kolab.org" );

  eJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );
  eJob->setRecipients( recipients );
  eJob->setEncryptionKeys( keys );
 
  eJob->appendSubjob( tJob );

  bool eWorked = checkEncryption( eJob );
  QVERIFY( eWorked );

}


void EncryptJobTest::testHeaders()
{
   std::vector< GpgME::Key > keys = getKeys();

  Message::Composer *composer = new Message::Composer;
  Message::EncryptJob* eJob = new Message::EncryptJob( composer );

  QVERIFY( composer );
  QVERIFY( eJob );

  QByteArray data( QString::fromLocal8Bit( "one flew over the cuckoo's nest" ).toUtf8() );
  KMime::Content* content = new KMime::Content;
  content->setBody( data );

  QStringList recipients;
  recipients << QString::fromLocal8Bit( "test@kolab.org" );

  eJob->setContent( content );
  eJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );
  eJob->setRecipients( recipients );
  eJob->setEncryptionKeys( keys );

  VERIFYEXEC( eJob );

  QByteArray mimeType( "multipart/encrypted" );
  QByteArray charset( "ISO-8859-1" );

  KMime::Content *result = eJob->content();
  result->assemble();
  kDebug() << result->encodedContent();

  QVERIFY( result->contentType( false ) );
  QCOMPARE( result->contentType()->mimeType(), mimeType );
  QCOMPARE( result->contentType()->charset(), charset );
  QCOMPARE( result->contentType()->parameter( QString::fromLocal8Bit( "protocol" ) ), QString::fromLocal8Bit( "application/pgp-encrypted" ) );
  QCOMPARE( result->contentTransferEncoding()->encoding(), KMime::Headers::CE7Bit );
}


bool EncryptJobTest::checkEncryption( Message::EncryptJob* eJob )
{

  eJob->exec();

  KMime::Content* result = eJob->content();
  Q_ASSERT( result );
  result->assemble();

  // store it in a KMime::Message, that's what OTP needs
  KMime::Message* resultMessage =  new KMime::Message;
  resultMessage->setContent( result->encodedContent() );
  resultMessage->parse();

  // parse the result and make sure it is valid in various ways
  MessageViewer::EmptySource es;
  MessageViewer::NodeHelper* nh = new MessageViewer::NodeHelper;
  MessageViewer::ObjectTreeParser otp( &es, nh, 0, false, false, true );

  // ensure the enc part exists and is parseable
  KMime::Content* encPart = MessageViewer::ObjectTreeParser::findType( resultMessage, "application", "pgp-encrypted", true, true );
  Q_ASSERT( encPart );

  // process the result..
  MessageViewer::ProcessResult pResult( nh );
//   kDebug() << resultMessage->topLevel();
  otp.processMultiPartEncryptedSubtype( resultMessage, pResult );
  Q_ASSERT( nh->encryptionState( resultMessage ) == MessageViewer::KMMsgFullyEncrypted );

  // make sure the decoded content is what we encrypted
  // processMultiPartEncrypted will add a child part with the unencrypted data
  Q_ASSERT( QString::fromUtf8( MessageViewer::NodeHelper::firstChild( resultMessage )->body() ) == QString::fromLocal8Bit( "one flew over the cuckoo's nest" ) );

  return true;
}

void EncryptJobTest::setupEnv()
{
  setenv("GNUPGHOME", QDir::currentPath().toLocal8Bit() +  "/gnupg_home" , 1 );
  setenv("LC_ALL", "C", 1); \
  setenv("KDEHOME", QFile::encodeName( QDir::homePath() + QString::fromAscii( "/.kde-unit-test" ) ), 1);

  kDebug() << "Set test GNUPG home to:" <<  QDir::currentPath().toLocal8Bit() +  "/gnupg_home";
}

#include "encryptjobtest.moc"
