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

#include "collectionaclwidgettest.h"
#include "../collectionaclwidget.h"
#include <qlistview.h>
#include <QPushButton>
#include <qtest.h>

CollectionAclWidgetTest::CollectionAclWidgetTest(QObject *parent)
    : QObject(parent)
{

}

CollectionAclWidgetTest::~CollectionAclWidgetTest()
{

}

void CollectionAclWidgetTest::shouldHaveDefaultValue()
{
    PimCommon::CollectionAclWidget w;
    QVERIFY(w.aclManager());
    QListView *listView = qFindChild<QListView *>(&w, QLatin1String("list_view"));
    QVERIFY(listView);
    QPushButton *button = qFindChild<QPushButton *>(&w, QLatin1String("add"));
    QVERIFY(button);
    button = qFindChild<QPushButton *>(&w, QLatin1String("edit"));
    QVERIFY(button);
    button = qFindChild<QPushButton *>(&w, QLatin1String("delete"));
    QVERIFY(button);
}

QTEST_MAIN(CollectionAclWidgetTest)
