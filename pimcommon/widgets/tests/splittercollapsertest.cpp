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

#include "splittercollapsertest.h"
#include "widgets/splittercollapser.h"
#include <QHBoxLayout>
#include <QSplitter>
#include <QTextEdit>
#include <qtest_kde.h>
#include <qtestmouse.h>

TestSplitter::TestSplitter(QObject *parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    setLayout(lay);
    splitter = new QSplitter;
    lay->addWidget(splitter);
    edit1 = new QTextEdit;
    splitter->addWidget(edit1);

    edit2 = new QTextEdit;
    splitter->addWidget(edit2);
}

SplitterCollapserTest::SplitterCollapserTest(QObject *parent)
    : QObject(parent)
{

}

SplitterCollapserTest::~SplitterCollapserTest()
{

}

void SplitterCollapserTest::shouldHasDefaultValue()
{
    TestSplitter testSplitter;

    PimCommon::SplitterCollapser *splitterCollapser = new PimCommon::SplitterCollapser(testSplitter.splitter, testSplitter.edit2);

    testSplitter.show();
    QTest::qWaitForWindowShown(&testSplitter);
    QVERIFY(testSplitter.isVisible());

    PimCommon::SplitterCollapser *collapser = qFindChild<PimCommon::SplitterCollapser *>(&testSplitter, QLatin1String("splittercollapser"));
    QVERIFY(collapser);
    QVERIFY(!collapser->isCollapsed());
}

void SplitterCollapserTest::shouldCollapsedWhenClickOnButton()
{
    TestSplitter testSplitter;

    PimCommon::SplitterCollapser *splitterCollapser = new PimCommon::SplitterCollapser(testSplitter.splitter, testSplitter.edit2);

    testSplitter.show();
    QTest::qWaitForWindowShown(&testSplitter);

    PimCommon::SplitterCollapser *collapser = qFindChild<PimCommon::SplitterCollapser *>(&testSplitter, QLatin1String("splittercollapser"));
    QVERIFY(!collapser->isCollapsed());
    QTest::mouseClick(collapser, Qt::LeftButton);
    QVERIFY(collapser->isCollapsed());

    QTest::mouseClick(collapser, Qt::LeftButton);
    QVERIFY(!collapser->isCollapsed());
}




QTEST_KDEMAIN(SplitterCollapserTest, GUI)
