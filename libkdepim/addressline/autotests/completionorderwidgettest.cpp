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
#include <KPushButton>
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
    QTreeWidget *treewidget = qFindChild<QTreeWidget *>(&w, QLatin1String("listview"));
    QVERIFY(treewidget);
    QVERIFY(treewidget->isHeaderHidden());
    QVERIFY(treewidget->isSortingEnabled());
    QCOMPARE(treewidget->topLevelItemCount(), 0);

    KPushButton *up = qFindChild<KPushButton *>(&w, QLatin1String("mUpButton"));
    QVERIFY(up);
    QVERIFY(up->autoRepeat());

    KPushButton *down = qFindChild<KPushButton *>(&w, QLatin1String("mDownButton"));
    QVERIFY(down);
    QVERIFY(down->autoRepeat());
}

QTEST_MAIN(CompletionOrderWidgetTest)
