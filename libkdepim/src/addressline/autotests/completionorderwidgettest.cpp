/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "completionorderwidgettest.h"
#include "../completionorder/completionorderwidget.h"
#include <QPushButton>
#include <QTreeWidget>
#include <qtest.h>

CompletionOrderWidgetTest::CompletionOrderWidgetTest(QObject *parent)
    : QObject(parent)
{

}

CompletionOrderWidgetTest::~CompletionOrderWidgetTest()
{

}

void CompletionOrderWidgetTest::shouldHaveDefaultValue()
{
    KPIM::CompletionOrderWidget w;
    QTreeWidget *treewidget = w.findChild<QTreeWidget *>(QStringLiteral("listview"));
    QVERIFY(treewidget);
    QVERIFY(treewidget->isHeaderHidden());
    QVERIFY(treewidget->isSortingEnabled());
    QCOMPARE(treewidget->topLevelItemCount(), 0);

    QPushButton *up = w.findChild<QPushButton *>(QStringLiteral("mUpButton"));
    QVERIFY(up);
    QVERIFY(up->autoRepeat());

    QPushButton *down = w.findChild<QPushButton *>(QStringLiteral("mDownButton"));
    QVERIFY(down);
    QVERIFY(down->autoRepeat());
}

QTEST_MAIN(CompletionOrderWidgetTest)
