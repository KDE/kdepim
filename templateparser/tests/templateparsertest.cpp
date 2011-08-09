/* Copyright 2011 Sudhendu Kumar <sudhendu.kumar.roy@gmail.com>

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

#include "templateparsertest.h"
#include "templateparser/templateparser.h"
#include "messageviewer/objecttreeparser.h"
#include "messageviewer/objecttreeemptysource.h"
#include "qwebpage.h"
#include "qwebframe.h"
#include "qtest_kde.h"
#include "kdebug.h"

using namespace TemplateParser;
using namespace MessageViewer;

QTEST_KDEMAIN( TemplateParserTester, GUI )

void TemplateParserTester::test_validHtml()
{
  //check whether plain messages are converted to valid html
  KMime::Message::Ptr mOrigMsg( new KMime::Message() );
  QByteArray content(
      "From: Sudhendu Kumar <dontspamme@yoohoo.com>\n"
      "Subject: Plain Message Test\n"
      "Date: Sun, 7 Aug 2011 11:30:27 +0530\n"
      "MIME-Version: 1.0\n"
      "Content-Type: text/plain;\n"
      "  charset=\"iso-8859-15\"\n"
      "\n"
      "This is the message text from Sudhendu Kumar<dontspamme@yoohoo.com>.\n"
      "\n-- \n"
      "Thanks & Regards\n"
      "Sudhendu Kumar" );

  EmptySource emptySource;
  mOrigMsg->setContent( content );
  mOrigMsg->parse();

  QCOMPARE( mOrigMsg->subject()->as7BitString( false ).constData(), "Plain Message Test" );
  QCOMPARE( mOrigMsg->contents().size(), 0 );

  ObjectTreeParser otp( &emptySource );
  otp.parseObjectTree( mOrigMsg.get() );

  QVERIFY( otp.htmlContent().isEmpty() );
  QVERIFY( !otp.plainTextContent().isEmpty() );
  const QString convertedHtmlContent = otp.convertedHtmlContent();
  QVERIFY( !convertedHtmlContent.isEmpty() );
  const QString result( "<html><head></head><body>This is the message text from Sudhendu "
                        "Kumar&lt;dontspamme@yoohoo.com&gt;.<br /><br />-- <br />"
                        "Thanks &amp; Regards<br />Sudhendu Kumar</body></html>");
  QCOMPARE( convertedHtmlContent, result );
}

void TemplateParserTester::test_bodyFromHtml()
{
  const QString content( "<html><head><title>Plain mail with signature</title></head>"
                         "<body>This is the message text from Sudhendu Kumar&lt;"
                         "dontspamme@yoohoo.com&gt;.<br /><br />-- <br />Thanks &amp; "
                         "Regards<br />Sudhendu Kumar</body></html>");
  QWebPage page;
  page.settings()->setAttribute( QWebSettings::JavascriptEnabled, false );
  page.settings()->setAttribute( QWebSettings::JavaEnabled, false );
  page.settings()->setAttribute( QWebSettings::PluginsEnabled, false );

  page.currentFrame()->setHtml( content );

  page.settings()->setAttribute( QWebSettings::JavascriptEnabled, true );

  const QString bodyElement = page.currentFrame()->evaluateJavaScript(
    "document.getElementsByTagName('body')[0].innerHTML").toString();

  page.settings()->setAttribute( QWebSettings::JavascriptEnabled, false );

  const QString expectedBody( "This is the message text from Sudhendu Kumar"
                              "&lt;dontspamme@yoohoo.com&gt;.<br><br>-- <br>"
                              "Thanks &amp; Regards<br>Sudhendu Kumar" );

  QCOMPARE( bodyElement, expectedBody );

  page.settings()->setAttribute( QWebSettings::JavascriptEnabled, true );

  const QString headElement = page.currentFrame()->evaluateJavaScript(
    "document.getElementsByTagName('head')[0].innerHTML" ).toString();

  page.settings()->setAttribute( QWebSettings::JavascriptEnabled, false );

  const QString expectedHead( "<title>Plain mail with signature</title>" );

  QCOMPARE( headElement, expectedHead );
}
