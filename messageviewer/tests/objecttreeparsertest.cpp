/* Copyright 2009 Thomas McGuire <mcguire@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "objecttreeparsertest.h"
#include "util.h"

#include "objecttreeparser.h"
#include "objecttreeemptysource.h"
#include "interfaces/htmlwriter.h"
#include "messageviewer/csshelper.h"

#include <akonadi/item.h>

#include "qtest_kde.h"
#include <messagecore/tests/util.h>

using namespace MessageViewer;

QTEST_KDEMAIN( ObjectTreeParserTester, GUI )

void ObjectTreeParserTester::initTestCase()
{
  setenv("GNUPGHOME", KDESRCDIR "../../messagecore/tests/gnupg_home" , 1 );
  setenv("LC_ALL", "C", 1);
  setenv( "KDEHOME", QFile::encodeName(  QDir::homePath() + QString::fromAscii(  "/.kde-unit-test" ) ), 1 );
}

void ObjectTreeParserTester::test_parsePlainMessage()
{
  KMime::Message::Ptr msg( new KMime::Message() );
  QByteArray content(
      "From: Thomas McGuire <dontspamme@gmx.net>\n"
      "Subject: Plain Message Test\n"
      "Date: Wed, 5 Aug 2009 10:58:27 +0200\n"
      "MIME-Version: 1.0\n"
      "Content-Type: text/plain;\n"
      "  charset=\"iso-8859-15\"\n"
      "\n"
      "This is the message text.\n" );
  msg->setContent( content );
  msg->parse();

  QCOMPARE( msg->subject()->as7BitString( false ).constData(), "Plain Message Test" );
  QCOMPARE( msg->contents().size(), 0 );

  // Parse the message
  EmptySource emptySource;
  ObjectTreeParser otp( &emptySource );
  otp.parseObjectTree( msg.get() );

  // Check that the textual content and the charset have the expected values
  QCOMPARE( otp.plainTextContent(), QString( "This is the message text." ) );
  QVERIFY( otp.htmlContent().isEmpty() );
  QCOMPARE( otp.plainTextContentCharset().toLower(), QByteArray( "iso-8859-15" ) );

  // Check that the message was not modified in any way
  QCOMPARE( msg->encodedContent().constData(), content.constData() );

  // Test that the charset of messages without an explicit charset declaration
  // is correct
  content =
      "From: Thomas McGuire <dontspamme@gmx.net>\n"
      "Subject: Plain Message Test\n"
      "Date: Wed, 5 Aug 2009 10:58:27 +0200\n"
      "MIME-Version: 1.0\n"
      "Content-Type: text/plain;\n"
      "\n"
      "This is the message text.\n";
  msg->setContent( content );
  msg->parse();
  ObjectTreeParser otp2( &emptySource );
  otp2.parseObjectTree( msg.get() );
  QCOMPARE( otp2.plainTextContentCharset().constData(), msg->defaultCharset().constData() );
}

void ObjectTreeParserTester::test_parseEncapsulatedMessage()
{
  KMime::Message::Ptr msg = readAndParseMail( "encapsulated-with-attachment.mbox" );
  QCOMPARE( msg->subject()->as7BitString( false ).constData(), "Fwd: Test with attachment" );
  QCOMPARE( msg->contents().size(), 2 );

  // Parse the message
  TestHtmlWriter testWriter;
  TestCSSHelper testCSSHelper;
  NodeHelper nodeHelper;
  MessageCore::Test::TestObjectTreeSource emptySource( &testWriter, &testCSSHelper );
  ObjectTreeParser otp( &emptySource, &nodeHelper );
  otp.parseObjectTree( msg.get() );

  // Check that the OTP didn't modify the message in weird ways
  QCOMPARE( msg->contents().size(), 2 );
  QCOMPARE( msg->contents().at( 0 )->contents().size(), 0 );
  QCOMPARE( msg->contents().at( 1 )->contents().size(), 1 );
  QCOMPARE( msg->contents().at( 1 )->contents().first()->contents().size(), 2 );
  QCOMPARE( msg->contents().at( 1 )->contents().first()->contents().at( 0 )->contents().size(), 0 );
  QCOMPARE( msg->contents().at( 1 )->contents().first()->contents().at( 1 )->contents().size(), 0 );

  // Check that the textual content and the charset have the expected values
  QCOMPARE( otp.plainTextContent(), QString( "This is the encapsulating message." ) );
  QCOMPARE( otp.plainTextContentCharset().toLower(), QByteArray( "iso-8859-15" ) );
  QVERIFY( otp.htmlContent().isEmpty() );

  // Check that the objecttreeparser did process the encapsulated message
  KMime::Message::Ptr encapsulated = msg->contents().at( 1 )->bodyAsMessage();
  QVERIFY( encapsulated );
  QVERIFY( nodeHelper.nodeProcessed( encapsulated.get() ) );
  QVERIFY( nodeHelper.nodeProcessed( encapsulated->contents().at( 0 ) ) );
  QVERIFY( nodeHelper.nodeProcessed( encapsulated->contents().at( 1 ) ) );
  QVERIFY( nodeHelper.partMetaData( msg->contents().at( 1 ) ).isEncapsulatedRfc822Message );
}

void ObjectTreeParserTester::test_missingContentTypeHeader()
{
  KMime::Message::Ptr msg = readAndParseMail( "no-content-type.mbox" );
  QCOMPARE( msg->subject()->as7BitString( false ).constData(), "Simple Mail Without Content-Type Header" );
  QCOMPARE( msg->contents().size(), 0 );

  TestHtmlWriter testWriter;
  TestCSSHelper testCSSHelper;
  NodeHelper nodeHelper;
  MessageCore::Test::TestObjectTreeSource emptySource( &testWriter, &testCSSHelper );
  ObjectTreeParser otp( &emptySource, &nodeHelper );
  otp.parseObjectTree( msg.get() );

  QCOMPARE( otp.plainTextContent().toAscii().data(), "asdfasdf" );
  QVERIFY( otp.htmlContent().isEmpty() );
}

// This is used to override the default message output handler. In unit tests, the special message
// output handler can write messages to stdout delayed, i.e. after the actual kDebug() call. This
// interfers with KPGP, since KPGP reads output from stdout, which needs to be kept clean.
void nullMessageOutput(QtMsgType type, const char *msg)
{
  Q_UNUSED(type);
  Q_UNUSED(msg);
}

void ObjectTreeParserTester::test_inlinePGPDecryption()
{
  KMime::Message::Ptr msg = readAndParseMail( "inlinepgpencrypted.mbox" );

  QCOMPARE( msg->subject()->as7BitString( false ).constData(), "inlinepgpencrypted" );
  QCOMPARE( msg->contents().size(), 0 );

  TestHtmlWriter testWriter;
  TestCSSHelper testCSSHelper;
  NodeHelper nodeHelper;
  MessageCore::Test::TestObjectTreeSource emptySource( &testWriter, &testCSSHelper );
  ObjectTreeParser otp( &emptySource, &nodeHelper );

  qInstallMsgHandler(nullMessageOutput);
  otp.parseObjectTree( msg.get() );
  qInstallMsgHandler(0);

  QCOMPARE( otp.plainTextContent().toAscii().data(), "some random text" );
  QVERIFY( otp.htmlContent().isEmpty() );
}

void ObjectTreeParserTester::test_HTML()
{
  KMime::Message::Ptr msg = readAndParseMail( "html.mbox" );

  QCOMPARE( msg->subject()->as7BitString( false ).constData(), "HTML test" );
  QCOMPARE( msg->contents().size(), 2 );

  EmptySource emptySource;
  ObjectTreeParser otp( &emptySource );

  otp.parseObjectTree( msg.get() );

  QCOMPARE( otp.plainTextContent().toAscii().data(), "Some HTML text" );
  QVERIFY( otp.htmlContent().contains( "Some <span style=\" font-weight:600;\">HTML</span> text" ) );
  QCOMPARE( otp.htmlContentCharset().data(), "windows-1252" );
}

void ObjectTreeParserTester::test_HTMLOnly()
{
  KMime::Message::Ptr msg = readAndParseMail( "htmlonly.mbox" );

  QCOMPARE( msg->subject()->as7BitString( false ).constData(), "HTML test" );
  QCOMPARE( msg->contents().size(), 0 );

  EmptySource emptySource;
  ObjectTreeParser otp( &emptySource );

  otp.parseObjectTree( msg.get() );

  QCOMPARE( otp.plainTextContent().toAscii().data(), "" );
  QVERIFY( otp.htmlContent().contains( "<b>SOME</b> HTML text." ) );
}


