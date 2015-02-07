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

#include "resultduplicatetreewidgettest.h"
#include "../searchduplicate/resultduplicatetreewidget.h"
#include <qtest.h>

ResultDuplicateTreeWidgetTest::ResultDuplicateTreeWidgetTest(QObject *parent)
    : QObject(parent)
{

}

ResultDuplicateTreeWidgetTest::~ResultDuplicateTreeWidgetTest()
{

}

void ResultDuplicateTreeWidgetTest::shouldHaveDefaultValue()
{
    KABMergeContacts::ResultDuplicateTreeWidget w;
    QCOMPARE(w.topLevelItemCount(), 0);
}

void ResultDuplicateTreeWidgetTest::shouldFillList()
{
    KABMergeContacts::ResultDuplicateTreeWidget w;
    Akonadi::Item::List lst;
    lst << Akonadi::Item(42);
    lst << Akonadi::Item(43);
    lst << Akonadi::Item(44);
    QList<Akonadi::Item::List> itemLst;
    itemLst << lst;
    w.setContacts(itemLst);
    QCOMPARE(w.topLevelItemCount(), 4);
}

void ResultDuplicateTreeWidgetTest::shouldClearList()
{
    KABMergeContacts::ResultDuplicateTreeWidget w;
    Akonadi::Item::List lst;
    lst << Akonadi::Item(42);
    lst << Akonadi::Item(43);
    lst << Akonadi::Item(44);
    QList<Akonadi::Item::List> itemLst;
    itemLst << lst;
    w.setContacts(itemLst);

    lst << Akonadi::Item(45);
    itemLst.clear();
    itemLst << lst;
    w.setContacts(itemLst);
    QCOMPARE(w.topLevelItemCount(), 5);
}

void ResultDuplicateTreeWidgetTest::shouldEmptyListIfNotContactSelected()
{
    KABMergeContacts::ResultDuplicateTreeWidget w;
    Akonadi::Item::List lst;
    lst << Akonadi::Item(42);
    lst << Akonadi::Item(43);
    lst << Akonadi::Item(44);
    QList<Akonadi::Item::List> itemLst;
    itemLst << lst;
    w.setContacts(itemLst);
    QVERIFY(w.selectedContactsToMerge().isEmpty());
}

QTEST_MAIN(ResultDuplicateTreeWidgetTest)

