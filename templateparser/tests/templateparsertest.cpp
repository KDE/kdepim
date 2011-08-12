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
#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include "qwebpage.h"
#include "qwebframe.h"
#include "qtest_kde.h"
#include "kdebug.h"

using namespace MessageViewer;

void TemplateParserTester::test_convertedHtml_data()
{
  QTest::addColumn<QString>( "mailFileName" );
  QTest::addColumn<QString>( "referenceFileName" );

  QDir dir( MAIL_DATA_DIR );
  foreach ( const QString &file, dir.entryList( QStringList("plain*.mbox"), QDir::Files | QDir::Readable | QDir::NoSymLinks  ) ) {
    QTest::newRow( file.toLatin1() ) << QString(dir.path() + '/' +  file) << QString(dir.path() + '/' + file + ".html");
  }
}

void TemplateParserTester::test_convertedHtml()
{
  QFETCH( QString, mailFileName );
  QFETCH( QString, referenceFileName );

  // load input mail
  QFile mailFile( mailFileName );
  QVERIFY( mailFile.open( QIODevice::ReadOnly ) );
  const QByteArray mailData = KMime::CRLFtoLF( mailFile.readAll() );
  QVERIFY( !mailData.isEmpty() );
  KMime::Message::Ptr msg( new KMime::Message );
  msg->setContent( mailData );
  msg->parse();

  // load expected result
  QFile referenceFile( referenceFileName );
  QVERIFY( referenceFile.open( QIODevice::ReadOnly ) );
  const QByteArray referenceRawData = KMime::CRLFtoLF( referenceFile.readAll() );
  const QString referenceData = QString( referenceRawData );
  QVERIFY( !referenceData.isEmpty() );

  EmptySource emptySource;

  QCOMPARE( msg->subject()->as7BitString( false ).constData(), "Plain Message Test" );
  QCOMPARE( msg->contents().size(), 0 );

  ObjectTreeParser otp( &emptySource );
  otp.parseObjectTree( msg.get() );

  QVERIFY( otp.htmlContent().isEmpty() );
  QVERIFY( !otp.plainTextContent().isEmpty() );
  const QString convertedHtmlContent = otp.convertedHtmlContent();
  QVERIFY( !convertedHtmlContent.isEmpty() );

  QCOMPARE( convertedHtmlContent, referenceData );
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

void TemplateParserTester::test_processWithTemplates_data()
{
  QTest::addColumn<QString>( "command" );
  QTest::addColumn<QString>( "text" );
  QTest::addColumn<QString>( "expected" );
  QTest::addColumn<QString>( "selection" );

  QTest::newRow( "%OTEXT") << "%OTEXT" << "Original text.\nLine two." << "Original text.\nLine two." << "";
  QTest::newRow( "%QUOTE") << "%QUOTE" << "Quoted text.\nLine two." << "> Quoted text.\n> Line two." << "";
}

void TemplateParserTester::test_processWithTemplates()
{
  QFETCH( QString, command );
  QFETCH( QString, text );
  QFETCH( QString, expected );
  QFETCH( QString, selection );

  KMime::Message::Ptr msg( new KMime::Message() );
  msg->setBody( text.toLocal8Bit() );
  TemplateParser::TemplateParser parser( msg, TemplateParser::TemplateParser::Reply );
  parser.setSelection( selection );
  KPIMIdentities::IdentityManager* identMan = new KPIMIdentities::IdentityManager;
  parser.setIdentityManager( identMan );
  parser.setAllowDecryption( false );
  parser.processWithTemplate( command );

  identMan->deleteLater();
  QCOMPARE( QString( msg->encodedBody() ), expected );
}

QTEST_KDEMAIN( TemplateParserTester, GUI )

#include "templateparsertest.moc"
