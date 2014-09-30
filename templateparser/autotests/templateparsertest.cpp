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
#include "templateparser/templateparser.h"
#undef protected
#include "messageviewer/viewer/objecttreeparser.h"
#include "messageviewer/viewer/objecttreeemptysource.h"
#include <KIdentityManagement/kidentitymanagement/identitymanager.h>
#include <KIdentityManagement/kidentitymanagement/identity.h>
#include "qwebpage.h"
#include "qwebframe.h"
#include "qtest.h"
#include "qdebug.h"
#include <QDir>

using namespace MessageViewer;

void TemplateParserTester::test_convertedHtml_data()
{
    QTest::addColumn<QString>("mailFileName");
    QTest::addColumn<QString>("referenceFileName");

    QDir dir(QLatin1String(MAIL_DATA_DIR));
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
    otp.parseObjectTree(msg.get());

    QVERIFY(otp.htmlContent().isEmpty());
    QVERIFY(!otp.plainTextContent().isEmpty());
    const QString convertedHtmlContent = otp.convertedHtmlContent();
    QVERIFY(!convertedHtmlContent.isEmpty());

    QCOMPARE(convertedHtmlContent, referenceData);
}

void TemplateParserTester::test_bodyFromHtml()
{
    const QString content(QLatin1String("<html><head><title>Plain mail with signature</title></head>"
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
                                    QLatin1String("document.getElementsByTagName('body')[0].innerHTML")).toString();

    page.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);

    const QString expectedBody(QLatin1String("This is the message text from Sudhendu Kumar"
                               "&lt;dontspamme@yoohoo.com&gt;.<br><br>-- <br>"
                               "Thanks &amp; Regards<br>Sudhendu Kumar"));

    QCOMPARE(bodyElement, expectedBody);

    page.settings()->setAttribute(QWebSettings::JavascriptEnabled, true);

    const QString headElement = page.currentFrame()->evaluateJavaScript(
                                    QLatin1String("document.getElementsByTagName('head')[0].innerHTML")).toString();

    page.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);

    const QString expectedHead(QLatin1String("<title>Plain mail with signature</title>"));

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
    QTest::addColumn<QString>("expected");

    QDir dir(QLatin1String(MAIL_DATA_DIR));
    foreach (const QString &file, dir.entryList(QStringList(QLatin1String("plain*.mbox")), QDir::Files | QDir::Readable | QDir::NoSymLinks)) {
        QTest::newRow(file.toLatin1()) << "%OTIME" << QString(dir.path() + QLatin1Char('/') +  file) << "11:30";
        QTest::newRow(file.toLatin1()) << "%OTIMELONG" << QString(dir.path() + QLatin1Char('/') +  file) << "11:30:27";
        QTest::newRow(file.toLatin1()) << "%OTIMELONGEN" << QString(dir.path() + QLatin1Char('/') +  file) << "11:30:27";
        QTest::newRow(file.toLatin1()) << "%ODATE" << QString(dir.path() + QLatin1Char('/') +  file) << "Sunday 07 August 2011";
        QTest::newRow(file.toLatin1()) << "%ODATESHORT" << QString(dir.path() + QLatin1Char('/') +  file) << "2011-08-07";
        QTest::newRow(file.toLatin1()) << "%ODATEEN" << QString(dir.path() + QLatin1Char('/') +  file) << "Sunday 07 August 2011";
        QTest::newRow(file.toLatin1()) << "%OFULLSUBJ" << QString(dir.path() + QLatin1Char('/') +  file) << "Plain Message Test";
        QTest::newRow(file.toLatin1()) << "%OFULLSUBJECT" << QString(dir.path() + QLatin1Char('/') +  file) << "Plain Message Test";
        QTest::newRow(file.toLatin1()) << "%OFROMFNAME" << QString(dir.path() + QLatin1Char('/') +  file) << "Sudhendu";
        QTest::newRow(file.toLatin1()) << "%OFROMLNAME" << QString(dir.path() + QLatin1Char('/') +  file) << "Kumar";
        QTest::newRow(file.toLatin1()) << "%OFROMNAME" << QString(dir.path() + QLatin1Char('/') +  file) << "Sudhendu Kumar";
        QTest::newRow(file.toLatin1()) << "%OFROMADDR" << QString(dir.path() + QLatin1Char('/') +  file) << "Sudhendu Kumar <dontspamme@yoohoo.com>";
    }
}

void TemplateParserTester::test_processWithTemplatesForContent()
{
    QFETCH(QString, command);
    QFETCH(QString, mailFileName);
    QFETCH(QString, expected);

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

    identMan->deleteLater();
    QCOMPARE(QString::fromLatin1(msg->encodedBody()), expected);
}

QTEST_MAIN(TemplateParserTester)

