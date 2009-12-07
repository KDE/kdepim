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

#include <akonadi/item.h>
#include <kmime/kmime_message.h>

#include "qtest_kde.h"

using namespace MessageViewer;


QTEST_KDEMAIN( ObjectTreeParserTester, GUI )

void ObjectTreeParserTester::test_parsePlainMessage()
{
  KMime::Message *msg = new KMime::Message();
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
  otp.parseObjectTree( Akonadi::Item(), msg );

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
  otp2.parseObjectTree( Akonadi::Item(), msg );
  QCOMPARE( otp2.textualContentCharset().constData(), msg->defaultCharset().constData() );
}

void ObjectTreeParserTester::test_parseEncapsulatedMessage()
{
  QVERIFY( true );
  KMime::Message *msg = new KMime::Message();
  const QByteArray content(
      "From: Thomas McGuire <dontspamme@gmx.net>\n"
      "Subject: Fwd: Test with attachment\n"
      "Date: Wed, 5 Aug 2009 10:58:27 +0200\n"
      "MIME-Version: 1.0\n"
      "Content-Type: Multipart/Mixed;\n"
      "  boundary=\"Boundary-00=_zmUeKB+A8hGfCVZ\"\n"
      "\n"
      "\n"
      "--Boundary-00=_zmUeKB+A8hGfCVZ\n"
      "Content-Type: text/plain;\n"
      "  charset=\"iso-8859-15\"\n"
      "Content-Transfer-Encoding: 7bit\n"
      "Content-Disposition: inline\n"
      "\n"
      "This is the encapsulating message.\n"
      "\n"
      "--Boundary-00=_zmUeKB+A8hGfCVZ\n"
      "Content-Type: message/rfc822;\n"
      "  name=\"forwarded message\"\n"
      "Content-Transfer-Encoding: 7bit\n"
      "Content-Description: Thomas McGuire <dontspamme@gmx.net>: Test with attachment\n"
      "Content-Disposition: inline\n"
      "\n"
      "From: Thomas McGuire <dontspamme@gmx.net>\n"
      "Subject: Test with attachment\n"
      "Date: Wed, 5 Aug 2009 10:57:58 +0200\n"
      "MIME-Version: 1.0\n"
      "Content-Type: Multipart/Mixed;\n"
      "  boundary=\"Boundary-00=_WmUeKQpGb0DHyx1\"\n"
      "\n"
      "--Boundary-00=_WmUeKQpGb0DHyx1\n"
      "Content-Type: text/plain;\n"
      "  charset=\"us-ascii\"\n"
      "Content-Transfer-Encoding: 7bit\n"
      "Content-Disposition: inline\n"
      "\n"
      "\n"
      "\n"
      "\n"
      "This is the encapsulated message.\n"
      "\n"
      "--Boundary-00=_WmUeKQpGb0DHyx1\n"
      "Content-Type: text/plain;\n"
      "  name=\"attachment.txt\"\n"
      "Content-Transfer-Encoding: 7bit\n"
      "Content-Disposition: attachment;\n"
      "        filename=\"attachment.txt\"\n"
      "\n"
      "This is an attachment.\n"
      "\n"
      "--Boundary-00=_WmUeKQpGb0DHyx1--\n"
      "\n"
      "--Boundary-00=_zmUeKB+A8hGfCVZ--\n" );
    msg->setContent( content );
    msg->parse();
    QCOMPARE( msg->subject()->as7BitString( false ).constData(), "Fwd: Test with attachment" );
    QCOMPARE( msg->contents().size(), 2 );

    // Parse the message
    EmptySource emptySource;
    ObjectTreeParser otp( &emptySource );
    otp.parseObjectTree( Akonadi::Item(), msg );

    // Check that the textual content and the charset have the expected values
    QCOMPARE( otp.textualContent(), QString( "This is the encapsulating message." ) );
    QCOMPARE( otp.textualContentCharset().toLower(), QByteArray( "iso-8859-15" ) );

    // Check that the encapsulated message parsing worked as expected
    //qDebug() << msg->encodedContent().constData();
    QEXPECT_FAIL( "", "Encapulated message handling totally broken!", Continue );
    QCOMPARE( msg->encodedContent().constData(), content.constData() );
}
