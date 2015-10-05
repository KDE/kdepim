/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "quicksearchwarningtest.h"
#include "../src/core/widgets/quicksearchwarning.h"
#include <QAction>
#include <qtest.h>
QuickSearchWarningTest::QuickSearchWarningTest(QObject *parent)
    : QObject(parent)
{

}

QuickSearchWarningTest::~QuickSearchWarningTest()
{

}

void QuickSearchWarningTest::shouldHaveDefaultValue()
{
    MessageList::Core::QuickSearchWarning w;
    QVERIFY(!w.isVisible());
    QAction *act = w.findChild<QAction *>(QStringLiteral("donotshowagain"));
    QVERIFY(act);
}

void QuickSearchWarningTest::shouldSetVisible()
{
    MessageList::Core::QuickSearchWarning w;
    w.setSearchText(QStringLiteral("1"));
    QVERIFY(w.isVisible());
}

void QuickSearchWarningTest::shouldSetSearchText()
{
    QFETCH(QString, input);
    QFETCH(bool, visible);
    MessageList::Core::QuickSearchWarning w;
    w.setSearchText(input);
    QCOMPARE(w.isVisible(), visible);
}

void QuickSearchWarningTest::shouldSetSearchText_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("visible");
    QTest::newRow("bigword") <<  QStringLiteral("foofoofoo") << false;
    QTest::newRow("1character") <<  QStringLiteral("f") << true;
    QTest::newRow("multibigword") <<  QStringLiteral("foo foo foo") << false;
    QTest::newRow("multibigwordwithasmallone") <<  QStringLiteral("foo foo foo 1") << true;
    QTest::newRow("aspace") <<  QStringLiteral(" ") << false;
    QTest::newRow("multispace") <<  QStringLiteral("            ") << false;
}

QTEST_MAIN(QuickSearchWarningTest)
