/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
    const QString content = QLatin1String("<html><body></body></html>");
    QCOMPARE(testHtml(content), false);
}

void ScamDetectionTest::testIp()
{
    //TODO
    QString content = QLatin1String("<html><body></body></html>");
    QCOMPARE(testHtml(content), false);
}

QTEST_KDEMAIN( ScamDetectionTest, GUI )

#include "test_scamdetection.moc"
