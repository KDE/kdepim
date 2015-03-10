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

#include "mergecontactselectinformationtabwidgettest.h"
#include "../searchduplicate/mergecontactselectinformationtabwidget.h"
#include <qtest.h>

MergeContactSelectInformationTabWidgetTest::MergeContactSelectInformationTabWidgetTest(QObject *parent)
    : QObject(parent)
{

}

MergeContactSelectInformationTabWidgetTest::~MergeContactSelectInformationTabWidgetTest()
{

}

void MergeContactSelectInformationTabWidgetTest::shouldHaveDefaultValue()
{
    KABMergeContacts::MergeContactSelectInformationTabWidget w;
    QVERIFY(!w.tabBarVisible());
    QCOMPARE(w.count(), 0);
}

void MergeContactSelectInformationTabWidgetTest::shouldAddTab()
{
    KABMergeContacts::MergeContactSelectInformationTabWidget w;

    QVector<KABMergeContacts::MergeConflictResult> list;
    KABMergeContacts::MergeConflictResult conflict;
    Akonadi::Item::List listItem;
    KContacts::Addressee address1;
    Akonadi::Item item1;
    address1.setName(QLatin1String("foo1"));
    item1.setPayload<KContacts::Addressee>( address1 );

    KContacts::Addressee address2;
    Akonadi::Item item2;
    address2.setName(QLatin1String("foo2"));
    item2.setPayload<KContacts::Addressee>( address2 );

    listItem << item1;
    listItem << item2;
    conflict.list = listItem;

    KABMergeContacts::MergeContacts::ConflictInformations conflictInformation = KABMergeContacts::MergeContacts::Birthday;
    conflict.list = listItem;
    conflict.conflictInformation = conflictInformation;

    list << conflict;
    w.setRequiresSelectInformationWidgets(list, Akonadi::Collection(42));
    QVERIFY(!w.tabBarVisible());
    QCOMPARE(w.count(), 1);

    list << conflict;
    w.setRequiresSelectInformationWidgets(list, Akonadi::Collection(42));
    QVERIFY(!w.tabBarVisible());
    QCOMPARE(w.count(), 2);
}

QTEST_MAIN(MergeContactSelectInformationTabWidgetTest)
