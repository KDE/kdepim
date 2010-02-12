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

#include "objecttreeparser.h"
#include "objecttreeemptysource.h"
#include "interfaces/htmlwriter.h"
#include "messageviewer/csshelper.h"

#include <akonadi/item.h>

#include "qtest_kde.h"

using namespace MessageViewer;

class TestHtmlWriter : public MessageViewer::HtmlWriter {
  public:
    explicit TestHtmlWriter() {}
    virtual ~TestHtmlWriter() {}


    virtual void begin( const QString & ) {}
    virtual void write( const QString & ) {}
    virtual void end() {}
    virtual void reset() {}
    virtual void queue( const QString & ) {}
    virtual void flush() {}
    virtual void embedPart( const QByteArray &, const QString & ) {}

};

class TestCSSHelper : public MessageViewer::CSSHelper {
  public:
    TestCSSHelper() : MessageViewer::CSSHelper( 0 ) {}
    virtual ~TestCSSHelper() {}

    QString nonQuotedFontTag() const { return QString::fromAscii( "<" ); }

    QString quoteFontTag( int ) const { return QString::fromAscii( "<" ); }
};

// We can't use EmptySource, since that doesn't provide a HTML writer. Therefore, derive
// from EmptySource so we can provide our own HTML writer.
class TestObjectTreeSource : public MessageViewer::EmptySource
{
  public:
    TestObjectTreeSource( MessageViewer::HtmlWriter *writer,
                          MessageViewer::CSSHelper *cssHelper )
      : mWriter( writer ), mCSSHelper( cssHelper )
    {
    }

    virtual MessageViewer::HtmlWriter * htmlWriter() { return mWriter; }
    virtual MessageViewer::CSSHelper * cssHelper() { return mCSSHelper; }

  private:
    MessageViewer::HtmlWriter *mWriter;
    MessageViewer::CSSHelper *mCSSHelper;
};

QTEST_KDEMAIN( ObjectTreeParserTester, GUI )

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
  otp.parseObjectTree( Akonadi::Item(), msg.get() );

  // Check that the textual content and the charset have the expected values
  QCOMPARE( otp.textualContent(), QString( "This is the message text." ) );
  QCOMPARE( otp.textualContentCharset().toLower(), QByteArray( "iso-8859-15" ) );

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
  otp2.parseObjectTree( Akonadi::Item(), msg.get() );
  QCOMPARE( otp2.textualContentCharset().constData(), msg->defaultCharset().constData() );
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
  TestObjectTreeSource emptySource( &testWriter, &testCSSHelper );
  ObjectTreeParser otp( &emptySource, &nodeHelper );
  otp.parseObjectTree( Akonadi::Item(), msg.get() );

  // Check that the OTP didn't modify the message in weird ways
  QCOMPARE( msg->contents().size(), 2 );
  QCOMPARE( msg->contents().at( 0 )->contents().size(), 0 );
  QCOMPARE( msg->contents().at( 1 )->contents().size(), 1 );
  QCOMPARE( msg->contents().at( 1 )->contents().first()->contents().size(), 2 );
  QCOMPARE( msg->contents().at( 1 )->contents().first()->contents().at( 0 )->contents().size(), 0 );
  QCOMPARE( msg->contents().at( 1 )->contents().first()->contents().at( 1 )->contents().size(), 0 );

  // Check that the textual content and the charset have the expected values
  QCOMPARE( otp.textualContent(), QString( "This is the encapsulating message." ) );
  QCOMPARE( otp.textualContentCharset().toLower(), QByteArray( "iso-8859-15" ) );

  // Check that the objecttreeparser did process the encapsulated message
  KMime::Message::Ptr encapsulated = msg->contents().at( 1 )->bodyAsMessage();
  QVERIFY( encapsulated );
  QVERIFY( nodeHelper.nodeProcessed( encapsulated.get() ) );
  QVERIFY( nodeHelper.nodeProcessed( encapsulated->contents().at( 0 ) ) );
  QVERIFY( nodeHelper.nodeProcessed( encapsulated->contents().at( 1 ) ) );
  QVERIFY( nodeHelper.partMetaData( msg->contents().at( 1 ) ).isEncapsulatedRfc822Message );
}

KMime::Message::Ptr ObjectTreeParserTester::readAndParseMail( const QString &mailFile ) const
{
  QFile file( MAIL_DATA_DIR"/" + mailFile );
  Q_ASSERT( file.open( QIODevice::ReadOnly ) );
  const QByteArray data = KMime::CRLFtoLF( file.readAll() );
  Q_ASSERT( !data.isEmpty() );
  KMime::Message::Ptr msg( new KMime::Message );
  msg->setContent( data );
  msg->parse();
  return msg;
}
