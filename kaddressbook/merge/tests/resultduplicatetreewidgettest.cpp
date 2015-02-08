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
#include <qtest_kde.h>

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
    QCOMPARE(w.topLevelItemCount(), 1);
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
    QCOMPARE(w.topLevelItemCount(), 1);
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

void ResultDuplicateTreeWidgetTest::shouldReturnNotEmptyContactList()
{
    KABMergeContacts::ResultDuplicateTreeWidget w;
    Akonadi::Item::List lst;
    lst << Akonadi::Item(42);
    lst << Akonadi::Item(43);
    lst << Akonadi::Item(44);
    QList<Akonadi::Item::List> itemLst;
    itemLst << lst;
    w.setContacts(itemLst);

    for(int i=0; i < w.topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = w.topLevelItem(i);
        const int childCount = item->childCount();
        if (childCount > 0) {
            for (int child = 0; child < childCount; ++child) {
                KABMergeContacts::ResultDuplicateTreeWidgetItem *childItem = static_cast<KABMergeContacts::ResultDuplicateTreeWidgetItem *> (item->child(child));
                childItem->setCheckState(0, Qt::Checked);
            }
        }
    }
    QVERIFY(!w.selectedContactsToMerge().isEmpty());
}



QTEST_KDEMAIN(ResultDuplicateTreeWidgetTest, GUI)

