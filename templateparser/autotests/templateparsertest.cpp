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
#define protected public
#include "templateparser.h"
#undef protected
#include "MessageViewer/ObjectTreeParser"
#include "MessageViewer/ObjectTreeEmptySource"
#include <KIdentityManagement/kidentitymanagement/identitymanager.h>
#include <KIdentityManagement/kidentitymanagement/identity.h>
#include "qwebpage.h"
#include "qwebframe.h"
#include "qtest.h"
#include <QDir>
#include <QLocale>

using namespace MessageViewer;

void TemplateParserTester::test_convertedHtml_data()
{
    QTest::addColumn<QString>("mailFileName");
    QTest::addColumn<QString>("referenceFileName");

    QDir dir(QStringLiteral(MAIL_DATA_DIR));
    foreach (const QString &file, dir.entryList(QStringList(QLatin1String("plain*.mbox")), QDir::Files | QDir::Readable | QDir::NoSymLinks)) {
        QTest::newRow(file.toLatin1()) << QString(dir.path() + QLatin1Char('/') +  file) << QString(dir.path() + QLatin1Char('/') + file + QLatin1String(".html"));
    }
}

void TemplateParserTester::test_convertedHtml()
{
    QFETCH(QString, mailFileName);
    QFETCH(QString, referenceFileName);

    // load input mail
    QFile mailFile(mailFileName);
    QVERIFY(mailFile.open(QIODevice::ReadOnly));
    const QByteArray mailData = KMime::CRLFtoLF(mailFile.readAll());
    QVERIFY(!mailData.isEmpty());
    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(mailData);
    msg->parse();

    // load expected result
    QFile referenceFile(referenceFileName);
    QVERIFY(referenceFile.open(QIODevice::ReadOnly));
    const QByteArray referenceRawData = KMime::CRLFtoLF(referenceFile.readAll());
    const QString referenceData = QString::fromLatin1(referenceRawData);
    QVERIFY(!referenceData.isEmpty());

    EmptySource emptySource;

    QCOMPARE(msg->subject()->as7BitString(false).constData(), "Plain Message Test");
    QCOMPARE(msg->contents().size(), 0);

    ObjectTreeParser otp(&emptySource);
    otp.parseObjectTree(msg.data());

    QVERIFY(otp.htmlContent().isEmpty());
    QVERIFY(!otp.plainTextContent().isEmpty());
    const QString convertedHtmlContent = otp.convertedHtmlContent();
    QVERIFY(!convertedHtmlContent.isEmpty());

    QCOMPARE(convertedHtmlContent, referenceData);
}

void TemplateParserTester::test_bodyFromHtml()
{
    const QString content(QStringLiteral("<html><head><title>Plain mail with signature</title></head>"
                                         "<body>This is the message text from Sudhendu Kumar&lt;"
                                         "dontspamme@yoohoo.com&gt;.<br /><br />-- <br />Thanks &amp; "
                                         "Regards<br />Sudhendu Kumar</body></html>"));
    QWebPage page;
    page.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    page.settings()->setAttribute(QWebSettings::JavaEnabled, false);
    page.settings()->setAttribute(QWebSettings::PluginsEnabled, false);

    page.currentFrame()->setHtml(content);

    page.settings()->setAttribute(QWebSettings::JavascriptEnabled, true);

    const QString bodyElement = page.currentFrame()->evaluateJavaScript(
                                    QStringLiteral("document.getElementsByTagName('body')[0].innerHTML")).toString();

    page.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);

    const QString expectedBody(QStringLiteral("This is the message text from Sudhendu Kumar"
                               "&lt;dontspamme@yoohoo.com&gt;.<br><br>-- <br>"
                               "Thanks &amp; Regards<br>Sudhendu Kumar"));

    QCOMPARE(bodyElement, expectedBody);

    page.settings()->setAttribute(QWebSettings::JavascriptEnabled, true);

    const QString headElement = page.currentFrame()->evaluateJavaScript(
                                    QStringLiteral("document.getElementsByTagName('head')[0].innerHTML")).toString();

    page.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);

    const QString expectedHead(QStringLiteral("<title>Plain mail with signature</title>"));

    QCOMPARE(headElement, expectedHead);
}

void TemplateParserTester::test_processWithTemplatesForBody_data()
{
    QTest::addColumn<QString>("command");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<QString>("selection");

    QTest::newRow("%OTEXT") << "%OTEXT" << "Original text.\nLine two." << "Original text.\nLine two." << "";
    QTest::newRow("%OTEXT") << "%OTEXT" << "-----BEGIN PGP MESSAGE-----\nVersion: GnuPG v1.4.12 (GNU/Linux)\n"
                            "\n"
                            "hQEMAwzOQ1qnzNo7AQgA1345CrnOBTGf2eo4ABR6wkOdasI9SELRBKA1fNkFcq+Z\n"
                            "Qg0gWB5RLapU+VFRc5hK1zPOZ1dY6j3+uPHO4RhjfUgfiZ8T7oaWav15yP+07u21\n"
                            "EI9W9sk+eQU9GZSOayURucmZa/mbBz9hrsmePpORxD+C3uNTYa6ePTFlQP6wEZOI\n"
                            "7E53DrtJnF0EzIsCBIVep6CyuYfuSSwQ5gMgyPzfBqiGHNw96w2i/eayErc6lquL\n"
                            "JPFhIcMMq8w9Yo9+vXCAbkns6dtBAzlnAzuV86VFUZ/MnHTlCNk2yHyGLP6BS6hG\n"
                            "kFEUmgdHrGRizdz1sjo1tSmOLu+Gyjlv1Ir/Sqr8etJQAeTq3heKslAfhtotAMMt\n"
                            "R3tk228Su13Q3CAP/rktAyuGMDFtH8klW09zFdsZBDu8svE6d9e2nZ541NGspFVI\n"
                            "6XTZHUMMdlgnTBcu3aPc0ow=\n"
                            "=0xtc\n"
                            "-----END PGP MESSAGE-----" << "Crypted line.\nCrypted line two." << "";
    QTest::newRow("%QUOTE") << "%QUOTE" << "Quoted text.\nLine two." << "> Quoted text.\n> Line two." << "";
}

void TemplateParserTester::test_processWithTemplatesForBody()
{
    QFETCH(QString, command);
    QFETCH(QString, text);
    QFETCH(QString, expected);
    QFETCH(QString, selection);

    KMime::Message::Ptr msg(new KMime::Message());
    msg->setBody(text.toLocal8Bit());
    msg->parse();
    TemplateParser::TemplateParser parser(msg, TemplateParser::TemplateParser::Reply);
    parser.setSelection(selection);
    KIdentityManagement::IdentityManager *identMan = new KIdentityManagement::IdentityManager;
    parser.setIdentityManager(identMan);
    parser.setAllowDecryption(true);
    parser.mOrigMsg = msg;

    parser.processWithTemplate(command);

    identMan->deleteLater();
    QCOMPARE(QString::fromLatin1(msg->encodedBody()), expected);
}

void TemplateParserTester::test_processWithTemplatesForContent_data()
{
    QTest::addColumn<QString>("command");
    QTest::addColumn<QString>("mailFileName");
    QTest::addColumn<QString>("expectedBody");
    QTest::addColumn<bool>("hasDictionary");

    QDir dir(QStringLiteral(MAIL_DATA_DIR));
    const QString file = QStringLiteral("plain-message.mbox");
    const QString fileName = QString(dir.path() + QLatin1Char('/') +  file);
    QTest::newRow("%OTIME") << "%OTIME" << fileName << QLocale::system().toString(QTime(11, 30, 27), QLocale::ShortFormat) << false;
    QTest::newRow("%OTIMELONG") << "%OTIMELONG" << fileName << QLocale::system().toString(QTime(11, 30, 27), QLocale::LongFormat) << false;
    QTest::newRow("%OTIMELONGEN") << "%OTIMELONGEN" << fileName << QLocale(QLocale::C).toString(QTime(11, 30, 27), QLocale::LongFormat) << false;
    QTest::newRow("%ODATE") << "%ODATE" << fileName << QLocale::system().toString(QDate(2011, 8, 7), QLocale::LongFormat) << false;
    QTest::newRow("%ODATESHORT") << "%ODATESHORT" << fileName << QLocale::system().toString(QDate(2011, 8, 7), QLocale::ShortFormat) << false;
    QTest::newRow("%ODATEEN") << "%ODATEEN" << fileName << QLocale::c().toString(QDate(2011, 8, 7), QLocale::LongFormat) << false;
    QTest::newRow("%OFULLSUBJ") << "%OFULLSUBJ" << fileName << "Plain Message Test" << false;
    QTest::newRow("%OFULLSUBJECT") << "%OFULLSUBJECT" << fileName << "Plain Message Test" << false;
    QTest::newRow("%OFROMFNAME") << "%OFROMFNAME" << fileName << "Sudhendu" << false;
    QTest::newRow("%OFROMLNAME") << "%OFROMLNAME" << fileName  << "Kumar" << false;
    QTest::newRow("%OFROMNAME") << "%OFROMNAME" << fileName << "Sudhendu Kumar" << false;
    QTest::newRow("%OFROMADDR") << "%OFROMADDR" << fileName << "Sudhendu Kumar <dontspamme@yoohoo.com>" << false;
    QTest::newRow("%OTOADDR") << "%OTOADDR" << fileName << "kde <foo@yoohoo.org>" << false;
    QTest::newRow("%OTOFNAME") << "%OTOFNAME" << fileName << "kde" << false;
    QTest::newRow("%OTONAME") << "%OTONAME" << fileName << "kde" << false;
    QTest::newRow("%OTOLNAME") << "%OTOLNAME" << fileName << "" << false;
    QTest::newRow("%OTOLIST") << "%OTOLIST" << fileName << "kde <foo@yoohoo.org>" << false;
    QTest::newRow("%ODOW") << "%ODOW" << fileName << "Sunday" << false;
    QTest::newRow("%BLANK") << "%BLANK" << fileName << "" << false;
    QTest::newRow("%NOP") << "%NOP" << fileName << "" << false;
    QTest::newRow("%DICTIONARYLANGUAGE=\"en\"") << "%DICTIONARYLANGUAGE=\"en\"" << fileName << "" << true;
    QTest::newRow("%DICTIONARYLANGUAGE=\"\"") << "%DICTIONARYLANGUAGE=\"\"" << fileName << "" << false;
    QTest::newRow("%OTIMELONG %OFULLSUBJECT") << "%OTIMELONG %OFULLSUBJECT" << fileName << QLocale::system().toString(QTime(11, 30, 27), QLocale::LongFormat) + QStringLiteral(" Plain Message Test") << false;
    QTest::newRow("%OTIMELONG\n%OFULLSUBJECT") << "%OTIMELONG\n%OFULLSUBJECT" << fileName << QLocale::system().toString(QTime(11, 30, 27), QLocale::LongFormat) + QStringLiteral("\nPlain Message Test") << false;
    QTest::newRow("%REM=\"sdfsfsdsdfsdf\"") << "%REM=\"sdfsfsdsdfsdf\"" << fileName << "" << false;
    QTest::newRow("%CLEAR") << "%CLEAR" << fileName << "" << false;
    QTest::newRow("FOO foo") << "FOO foo" << fileName << "FOO foo" << false;
    const QString insertFileName = QString(dir.path() + QLatin1Char('/') +  QLatin1String("insert-file.txt"));
    QString insertFileNameCommand = QStringLiteral("%INSERT=\"%1\"").arg(insertFileName);
    QTest::newRow("%INSERT") << insertFileNameCommand << fileName << "test insert file!\n" << false;
    insertFileNameCommand = QStringLiteral("%PUT=\"%1\"").arg(insertFileName);
    QTest::newRow("%PUT") << insertFileNameCommand << fileName << "test insert file!\n" << false;
    QTest::newRow("%MSGID") << "%MSGID" << fileName << "<20150@foo.kde.org>" << false;
    QTest::newRow("%SYSTEM") << "%SYSTEM=\"echo foo\"" << fileName << "foo\n" << false;
    QTest::newRow("%DEBUG") << "%DEBUG" << fileName << "" << false;
    QTest::newRow("%DEBUGOFF") << "%DEBUGOFF" << fileName << "" << false;
    QTest::newRow("%HEADER=\"Reply-To\"") << "%HEADER=\"Reply-To\"" << fileName << "bla@yoohoo.org" << false;
    //Header doesn't exist => don't add value
    QTest::newRow("%OHEADER=\"SSS\"") << "%HEADER=\"SSS\"" << fileName << "" << false;
    QTest::newRow("%OHEADER=\"To\"") << "%OHEADER=\"To\"" << fileName << "kde <foo@yoohoo.org>" << false;
    //Unknown command
    QTest::newRow("unknown command") << "%GGGGG" << fileName << "%GGGGG" << false;
}

void TemplateParserTester::test_processWithTemplatesForContent()
{
    QFETCH(QString, command);
    QFETCH(QString, mailFileName);
    QFETCH(QString, expectedBody);
    QFETCH(bool, hasDictionary);

    QFile mailFile(mailFileName);
    QVERIFY(mailFile.open(QIODevice::ReadOnly));
    const QByteArray mailData = KMime::CRLFtoLF(mailFile.readAll());
    QVERIFY(!mailData.isEmpty());
    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(mailData);
    msg->parse();

    TemplateParser::TemplateParser parser(msg, TemplateParser::TemplateParser::Reply);
    KIdentityManagement::IdentityManager *identMan = new KIdentityManagement::IdentityManager;
    parser.setIdentityManager(identMan);
    parser.setAllowDecryption(false);
    parser.mOrigMsg = msg;
    parser.processWithTemplate(command);
    QCOMPARE(msg->hasHeader("X-KMail-Dictionary"), hasDictionary);

    identMan->deleteLater();
    QCOMPARE(QString::fromLatin1(msg->encodedBody()), expectedBody);
}

QTEST_MAIN(TemplateParserTester)

