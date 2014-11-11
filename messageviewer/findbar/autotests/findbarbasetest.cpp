/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "findbarbasetest.h"
#include "findbar/findbarbase.h"

#include <KLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <qtest.h>
#include <QSignalSpy>

FindBarBaseTest::FindBarBaseTest(QObject *parent)
    : QObject(parent)
{

}

FindBarBaseTest::~FindBarBaseTest()
{

}

void FindBarBaseTest::shouldHaveDefaultValue()
{
    MessageViewer::FindBarBase bar;
    QLabel *status = bar.findChild<QLabel *>(QLatin1String("status"));
    QVERIFY(status);
    QVERIFY(status->text().isEmpty());

    QPushButton *previous = bar.findChild<QPushButton *>(QLatin1String("findprevious"));
    QVERIFY(previous);
    QVERIFY(!previous->isEnabled());

    QPushButton *next = bar.findChild<QPushButton *>(QLatin1String("findnext"));
    QVERIFY(next);
    QVERIFY(!next->isEnabled());

    QToolButton *close = bar.findChild<QToolButton *>(QLatin1String("close"));
    QVERIFY(close);

    KLineEdit *lineedit = bar.findChild<KLineEdit *>(QLatin1String("searchline"));
    QVERIFY(lineedit);
    QVERIFY(lineedit->text().isEmpty());
}

void FindBarBaseTest::shouldClearLineWhenClose()
{
    MessageViewer::FindBarBase bar;
    bar.show();
    QSignalSpy spy(&bar, SIGNAL(hideFindBar()));
    QTest::qWaitForWindowExposed(&bar);
    QVERIFY(bar.isVisible());
    bar.focusAndSetCursor();
    KLineEdit *lineedit = bar.findChild<KLineEdit *>(QLatin1String("searchline"));
    lineedit->setText(QLatin1String("FOO"));
    QVERIFY(!lineedit->text().isEmpty());
    QVERIFY(lineedit->hasFocus());
    bar.closeBar();
    QVERIFY(lineedit->text().isEmpty());
    QVERIFY(!lineedit->hasFocus());
    QCOMPARE(spy.count(), 1);
}

void FindBarBaseTest::shouldEnableDisableNextPreviousButton()
{
    MessageViewer::FindBarBase bar;
    bar.show();
    QTest::qWaitForWindowExposed(&bar);
    QPushButton *previous = bar.findChild<QPushButton *>(QLatin1String("findprevious"));

    QPushButton *next = bar.findChild<QPushButton *>(QLatin1String("findnext"));

    bar.autoSearch(QLatin1String("FOO"));
    QVERIFY(next->isEnabled());
    QVERIFY(previous->isEnabled());

    bar.autoSearch(QString());
    QVERIFY(!next->isEnabled());
    QVERIFY(!previous->isEnabled());

}

void FindBarBaseTest::shouldClearAllWhenShowBar()
{
    MessageViewer::FindBarBase bar;
    bar.show();
    QTest::qWaitForWindowExposed(&bar);
    QLabel *status = bar.findChild<QLabel *>(QLatin1String("status"));
    status->setText(QLatin1String("FOO"));
    bar.closeBar();

    bar.show();
    bar.focusAndSetCursor();
    KLineEdit *lineedit = bar.findChild<KLineEdit *>(QLatin1String("searchline"));
    QVERIFY(lineedit->hasFocus());
    QVERIFY(status->text().isEmpty());
}

QTEST_MAIN(FindBarBaseTest)
