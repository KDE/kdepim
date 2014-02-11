/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/


#include "quicksearchlinetest.h"
#include "messagelist/core/quicksearchline.h"
#include <qtest_kde.h>
#include <qtestkeyboard.h>
#include <qtestmouse.h>
#include <KLineEdit>
#include <QToolButton>
#include <QPushButton>


using namespace MessageList::Core;
QuickSearchLineTest::QuickSearchLineTest()
{
}

void QuickSearchLineTest::shouldHaveDefaultValueOnCreation()
{
    QuickSearchLine searchLine;
    QVERIFY(searchLine.searchEdit()->text().isEmpty());
    QVERIFY(!searchLine.lockSearch()->isChecked());
    QWidget *widget = qFindChild<QWidget *>(&searchLine, QLatin1String("extraoptions"));
    QVERIFY(widget);
    QVERIFY(widget->isHidden());
}

void QuickSearchLineTest::shouldEmitTextChanged()
{
    QuickSearchLine searchLine;
    QSignalSpy spy(&searchLine, SIGNAL(searchEditTextEdited(QString)));
    QTest::keyClick(searchLine.searchEdit(), 'F');
    QCOMPARE(spy.count(),1);
}

void QuickSearchLineTest::shouldShowExtraOptionWidget()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::keyClick(searchLine.searchEdit(), 'F');
    QTest::qWaitForWindowShown(&searchLine);
    QWidget *widget = qFindChild<QWidget *>(&searchLine, QLatin1String("extraoptions"));
    QVERIFY(widget->isVisible());
}

void QuickSearchLineTest::shouldHideExtraOptionWidgetWhenClearLineEdit()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::keyClick(searchLine.searchEdit(), 'F');
    QTest::qWaitForWindowShown(&searchLine);
    QWidget *widget = qFindChild<QWidget *>(&searchLine, QLatin1String("extraoptions"));

    searchLine.searchEdit()->clear();
    QVERIFY(!widget->isVisible());
}

void QuickSearchLineTest::shouldHideExtraOptionWidgetWhenResetFilter()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QTest::keyClick(searchLine.searchEdit(), 'F');
    QTest::qWaitForWindowShown(&searchLine);
    QWidget *widget = qFindChild<QWidget *>(&searchLine, QLatin1String("extraoptions"));

    searchLine.resetFilter();
    QVERIFY(!widget->isVisible());
}

void QuickSearchLineTest::shouldEmitSearchOptionChanged()
{
    QuickSearchLine searchLine;
    searchLine.show();
    QSignalSpy spy(&searchLine, SIGNAL(searchOptionChanged()));
    QPushButton *button = qFindChild<QPushButton *>(&searchLine, QLatin1String("subject"));
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

QTEST_KDEMAIN( QuickSearchLineTest, GUI )
