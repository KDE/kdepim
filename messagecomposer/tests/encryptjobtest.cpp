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
#include "cryptofunctions.h"

#include <kmime/kmime_content.h>

#include <kleo/enum.h>
#include <kleo/keylistjob.h>
#include <kleo/cryptobackendfactory.h>
#include <kjob.h>

#include <messagecomposer/composer.h>
#include <messagecomposer/encryptjob.h>
#include <messagecomposer/transparentjob.h>
#include <messagecomposer/maintextjob.h>
#include <messagecomposer/textpart.h>
#include <messagecomposer/globalpart.h>

#include <messageviewer/objecttreeparser.h>
#include <messageviewer/objecttreeemptysource.h>
#include <messageviewer/nodehelper.h>
#include <messagecore/tests/util.h>

#include <stdlib.h>
#include <KCharsets>

QTEST_KDEMAIN( EncryptJobTest, GUI )

void EncryptJobTest::initTestCase()
{
  MessageCore::Test::setupEnv();
}

void EncryptJobTest::testContentDirect() {


  Message::Composer *composer = new Message::Composer;
  QList<QByteArray> charsets;
  charsets << "us-ascii";
  composer->globalPart()->setCharsets( charsets );
  Message::TextPart* part = new Message::TextPart( this );
  part->setWordWrappingEnabled(false);
  part->setCleanPlainText( QString::fromLatin1("one flew over the cuckoo's nest"));

  
  Message::MainTextJob *mainTextJob = new Message::MainTextJob( part, composer );
  
  QVERIFY( composer );
  QVERIFY( mainTextJob );


  mainTextJob->exec();

  std::vector< GpgME::Key > keys = MessageCore::Test::getKeys();

  Message::EncryptJob* eJob = new Message::EncryptJob( composer );

  QVERIFY( eJob );

  QStringList recipients;
  recipients << QString::fromLocal8Bit( "test@kolab.org" );

  eJob->setContent( mainTextJob->content() );
  eJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );
  eJob->setRecipients( recipients );
  eJob->setEncryptionKeys( keys );

  bool encrWorked = checkEncryption( eJob );
  QVERIFY( encrWorked );
  

}

void EncryptJobTest::testContentChained()
{
  Message::Composer *composer = new Message::Composer;
  QList<QByteArray> charsets;
  charsets << "us-ascii";
  composer->globalPart()->setCharsets( charsets );
  Message::TextPart* part = new Message::TextPart( this );
  part->setWordWrappingEnabled(false);
  part->setCleanPlainText( QString::fromLatin1("one flew over the cuckoo's nest"));


  Message::MainTextJob *mainTextJob = new Message::MainTextJob( part, composer );

  QVERIFY( composer );
  QVERIFY( mainTextJob );

  mainTextJob->exec();  
  
  std::vector< GpgME::Key > keys = MessageCore::Test::getKeys();
  kDebug() << "done getting keys";
  Message::EncryptJob* eJob = new Message::EncryptJob( composer );

  QStringList recipients;
  recipients << QString::fromLocal8Bit( "test@kolab.org" );

  eJob->setCryptoMessageFormat( Kleo::OpenPGPMIMEFormat );
  eJob->setRecipients( recipients );
  eJob->setEncryptionKeys( keys );
  eJob->setContent( mainTextJob->content() );
 
  bool eWorked = checkEncryption( eJob );
  QVERIFY( eWorked );

}


void EncryptJobTest::testHeaders()
{
   std::vector< GpgME::Key > keys = MessageCore::Test::getKeys();

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

  return ComposerTestUtil::verifyEncryption( result, QString::fromLocal8Bit( "one flew over the cuckoo's nest" ).toUtf8(), Kleo::OpenPGPMIMEFormat );

}

#include "encryptjobtest.moc"
