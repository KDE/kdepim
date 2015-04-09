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

#include "recentaddresswidgettest.h"
#include "../recentaddress/recentaddresswidget.h"
#include <KLineEdit>
#include <KPushButton>
#include <qlistwidget.h>
#include <qtest_kde.h>
#include <qtestmouse.h>

RecentAddressWidgetTest::RecentAddressWidgetTest(QObject *parent)
    : QObject(parent)
{

}

RecentAddressWidgetTest::~RecentAddressWidgetTest()
{

}

void RecentAddressWidgetTest::shouldHaveDefaultValue()
{
    KPIM::RecentAddressWidget w;
    KLineEdit *lineedit = qFindChild<KLineEdit *>(&w, QLatin1String("line_edit"));
    QVERIFY(lineedit);

    KPushButton *newButton = qFindChild<KPushButton *>(&w, QLatin1String("new_button"));
    QVERIFY(newButton);

    KPushButton *removeButton = qFindChild<KPushButton *>(&w, QLatin1String("remove_button"));
    QVERIFY(removeButton);

    QListWidget *listview = qFindChild<QListWidget *>(&w, QLatin1String("list_view"));
    QVERIFY(listview);
    QCOMPARE(listview->count(), 0);
}

void RecentAddressWidgetTest::shouldAddAddresses()
{
    KPIM::RecentAddressWidget w;
    QListWidget *listview = qFindChild<QListWidget *>(&w, QLatin1String("list_view"));
    QCOMPARE(listview->count(), 0);
    QStringList lst;
    lst << QLatin1String("foo");
    lst << QLatin1String("foo1");
    lst << QLatin1String("foo2");
    w.setAddresses(lst);
    QCOMPARE(listview->count(), lst.count());
    //Clear list before to add
    w.setAddresses(lst);
    QCOMPARE(listview->count(), lst.count());
}

void RecentAddressWidgetTest::shouldInformThatItWasChanged()
{
    KPIM::RecentAddressWidget w;
    QVERIFY(!w.wasChanged());
    KPushButton *newButton = qFindChild<KPushButton *>(&w, QLatin1String("new_button"));
    QVERIFY(newButton);
    QTest::mouseClick(newButton, Qt::LeftButton);
    QVERIFY(w.wasChanged());
    QListWidget *listview = qFindChild<QListWidget *>(&w, QLatin1String("list_view"));
    QCOMPARE(listview->count(), 1);
}

void RecentAddressWidgetTest::shouldNotAddMultiEmptyLine()
{
    KPIM::RecentAddressWidget w;
    KLineEdit *lineedit = qFindChild<KLineEdit *>(&w, QLatin1String("line_edit"));
    QVERIFY(lineedit);

    KPushButton *newButton = qFindChild<KPushButton *>(&w, QLatin1String("new_button"));
    QVERIFY(newButton);

    QListWidget *listview = qFindChild<QListWidget *>(&w, QLatin1String("list_view"));
    QCOMPARE(listview->count(), 0);

    QTest::mouseClick(newButton, Qt::LeftButton);
    QCOMPARE(listview->count(), 1);

    QTest::mouseClick(newButton, Qt::LeftButton);
    QCOMPARE(listview->count(), 1);
}

QTEST_KDEMAIN(RecentAddressWidgetTest, GUI)
