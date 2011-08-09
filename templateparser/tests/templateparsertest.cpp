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
#include "qtest_kde.h"
#include "kdebug.h"

using namespace TemplateParser;
using namespace MessageViewer;

QTEST_KDEMAIN( TemplateParserTester, GUI )

void TemplateParserTester::test_htmlMessageText()
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
  QString result( "<html><head></head><body>This is the message text from Sudhendu Kumar"
                  "&lt;dontspamme@yoohoo.com&gt;.<br /><br />-- <br />"
                  "Thanks &amp; Regards<br />Sudhendu Kumar</body></html>");
  QCOMPARE( otp.convertedHtmlContent(), result );
}

void TemplateParserTester::test_quotedPlainText()
{}

void TemplateParserTester::test_quotedHtmlText()
{}

void TemplateParserTester::test_createPlainPart()
{}

void TemplateParserTester::test_createMultipartAlternative()
{}

void TemplateParserTester::test_plainToHtml()
{}

void TemplateParserTester::test_makeValidHtml()
{}

void TemplateParserTester::test_addProcessedBodyToMessage()
{}

void TemplateParserTester::test_processWithTemplate()
{}
