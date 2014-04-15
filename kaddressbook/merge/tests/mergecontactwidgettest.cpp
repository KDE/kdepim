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

#include "mergecontactwidgettest.h"
#include <Akonadi/Item>
#include "../mergecontactwidget.h"
#include <qtest_kde.h>
#include <QListWidget>
#include <QPushButton>

MergeContactWidgetTest::MergeContactWidgetTest()
{
}

void MergeContactWidgetTest::shouldHaveDefaultValueOnCreation()
{
    Akonadi::Item::List lst;
    MergeContactWidget mergeWidget(lst);
    QListWidget *listWidget = qFindChild<QListWidget *>(&mergeWidget, QLatin1String("listcontact"));
    QVERIFY(listWidget);
    QCOMPARE(listWidget->count(), 0);
    QPushButton *button = qFindChild<QPushButton *>(&mergeWidget, QLatin1String("mergebutton"));
    QVERIFY(button);
    QCOMPARE(button->isEnabled(), false);
}

void MergeContactWidgetTest::shouldFillList()
{
    Akonadi::Item::List lst;
    for (int i=0; i <10; ++i) {
        lst.append(Akonadi::Item(i));
    }
    MergeContactWidget mergeWidget(lst);
    QListWidget *listWidget = qFindChild<QListWidget *>(&mergeWidget, QLatin1String("listcontact"));
    QCOMPARE(listWidget->count(), 10);
}


QTEST_KDEMAIN(MergeContactWidgetTest , GUI )
