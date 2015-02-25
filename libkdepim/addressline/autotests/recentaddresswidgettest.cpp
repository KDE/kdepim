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
#include "../recentaddresswidget.h"
#include <KLineEdit>
#include <QPushButton>
#include <qlistwidget.h>
#include <qtest.h>

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

    QPushButton *newButton = qFindChild<QPushButton *>(&w, QLatin1String("new_button"));
    QVERIFY(newButton);

    QPushButton *removeButton = qFindChild<QPushButton *>(&w, QLatin1String("remove_button"));
    QVERIFY(removeButton);

    QListWidget *listview = qFindChild<QListWidget *>(&w, QLatin1String("list_view"));
    QVERIFY(listview);
    QCOMPARE(listview->count(), 0);
}

QTEST_MAIN(RecentAddressWidgetTest)
