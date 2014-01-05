/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "test_scamdetection.h"
#include "messageviewer/scamdetection/scamdetection.h"
#include "messageviewer/scamdetection/scamcheckshorturl.h"

#include <QWebElement>
#include <QWebFrame>
#include <QWebSettings>
#include <QWebPage>

bool ScamDetectionTest::scanPage(QWebFrame *frame)
{
    QString details;
    bool foundScam = false;
    const QWebElement rootElement = frame->documentElement();
    bool result = MessageViewer::ScamDetection::scanFrame(rootElement, details);
    if (result) {
        foundScam = true;
    }
    foreach(QWebFrame *childFrame, frame->childFrames()) {
        result = MessageViewer::ScamDetection::scanFrame(childFrame->documentElement(), details);
        if (result) {
            foundScam = true;
        }
    }
    return foundScam;
}

bool ScamDetectionTest::testHtml(const QString &content)
{
    QWebPage page;
    page.settings()->setAttribute( QWebSettings::JavascriptEnabled, false );
    page.settings()->setAttribute( QWebSettings::JavaEnabled, false );
    page.settings()->setAttribute( QWebSettings::PluginsEnabled, false );

    page.currentFrame()->setHtml( content );

    return scanPage(page.currentFrame());
}

void ScamDetectionTest::testNoScam()
{
    QString content = QLatin1String("<html><body><a href=\"www.kde.org\">kde</a></body></html>");
    QCOMPARE(testHtml(content), false);

    content = QLatin1String("<html><body><a href=\"http://www.kde.org\" title=\"http://www.kde.org\">kde</a></body></html>");
    QCOMPARE(testHtml(content), false);

    content = QLatin1String("<html><body><a href=\"https://www.kde.org\" title=\"https://www.kde.org\">kde</a></body></html>");
    QCOMPARE(testHtml(content), false);
}

void ScamDetectionTest::testHexaValue()
{
    const QString content = QLatin1String("<html><body><a href=\"http://125.15.55.88/\" title=\"http://0x12.0x1e.0x0A.0x00\">test</a></body></html>");
    QCOMPARE(testHtml(content), true);
}

void ScamDetectionTest::testIp()
{
    QString content = QLatin1String("<html><body><a href=\"http://127.0.0.1/\">test</a></body></html>");
    QCOMPARE(testHtml(content), false);

    content = QLatin1String("<html><body><a href=\"http://125.15.55.88/\" title=\"http://www.kde.org\">test</a></body></html>");
    QCOMPARE(testHtml(content), true);

    content = QLatin1String("<html><body><a href=\"http://125.15.55.88/\" title=\"http://125.15.55.88/\">test</a></body></html>");
    QCOMPARE(testHtml(content), true);
}

void ScamDetectionTest::testHref()
{
    const QString content = QLatin1String("<html><body><a href=\"http://www.kde.org/\" title=\"http://www.kde.org\">test</a></body></html>");
    QCOMPARE(testHtml(content), false);
}

void ScamDetectionTest::testRedirectUrl()
{
    const QString content = QLatin1String("<html><body><a href=\"http://www.google.fr/url?q=http://www.yahoo.com\">test</a></body></html>");
    QCOMPARE(testHtml(content), true);
}

void ScamDetectionTest::testUrlWithNumericValue()
{
    QString content = QLatin1String("<html><body><a href=\"http://baseball2.2ndhalfplays.com/nested/attribs/\">http://baseball2.2ndhalfplays.com/nested/attribs</html>");
    QCOMPARE(testHtml(content), false);
    content = QLatin1String("<html><body><a href=\"http://25.15.55.88/\">test</a></body></html>");
    QCOMPARE(testHtml(content), true);
    content = QLatin1String("<html><body><a href=\"http://255.0.1.1/\">test</a></body></html>");
    QCOMPARE(testHtml(content), true);
    content = QLatin1String("<html><body><a href=\"http://1.0.1.1/\">test</a></body></html>");
    QCOMPARE(testHtml(content), true);
    content = QLatin1String("<html><body><a href=\"http://255.500.1.1/\">test</a></body></html>");
    QCOMPARE(testHtml(content), true);
    content = QLatin1String("<html><body><a href=\"http://baseball.2ndhalfplays.com/nested/attribs/\">http://baseball2.2ndhalfplays.com/nested/attribs</html>");
    QCOMPARE(testHtml(content), false);
    content = QLatin1String("<html><body><a href=\"http://baseball.2ndhalfplays.com/nested/attribs/\">http://baseball2.2ndhalfplays.com/nested/attribs</html>");
    QCOMPARE(testHtml(content), false);
}

void ScamDetectionTest::testShortUrl()
{
    MessageViewer::ScamCheckShortUrl::loadLongUrlServices();
    const QString content = QLatin1String("<html><body><a href=\"http://tinyurl.com/6d3x\">http://tinyurl.com/6d3x</a></body></html>");
    QCOMPARE(testHtml(content), true);
}


QTEST_KDEMAIN( ScamDetectionTest, GUI )

