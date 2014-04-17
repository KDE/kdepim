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


#include "mergecontactinfowidgettest.h"

#include "../mergecontactinfowidget.h"
#include <qtest_kde.h>
#include <QStackedWidget>
using namespace KABMergeContacts;

MergeContactInfoWidgetTest::MergeContactInfoWidgetTest()
{
}

void MergeContactInfoWidgetTest::shouldHaveDefaultValueOnCreation()
{
    MergeContactInfoWidget infoWidget;
    QStackedWidget *stackedWidget = qFindChild<QStackedWidget *>(&infoWidget, QLatin1String("stackedwidget"));
    QVERIFY(stackedWidget);
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QLatin1String("nocontact"));
}

void MergeContactInfoWidgetTest::shouldHaveActivateDisplayWidgetWhenSelectOneContact()
{
    MergeContactInfoWidget infoWidget;
    Akonadi::Item item(4);
    infoWidget.setContacts(Akonadi::Item::List()<<item);
    QStackedWidget *stackedWidget = qFindChild<QStackedWidget *>(&infoWidget, QLatin1String("stackedwidget"));
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QLatin1String("contactwidget"));
}


void MergeContactInfoWidgetTest::shouldHaveActivateTooManyWidgetWhenSelectMultiContact()
{
    MergeContactInfoWidget infoWidget;
    Akonadi::Item item(4);
    Akonadi::Item item2(4);
    infoWidget.setContacts(Akonadi::Item::List()<<item<<item2);
    QStackedWidget *stackedWidget = qFindChild<QStackedWidget *>(&infoWidget, QLatin1String("stackedwidget"));
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QLatin1String("toomanycontacts"));
}

void MergeContactInfoWidgetTest::shouldHaveActivateNoWidgetWhenSelectNoContact()
{
    MergeContactInfoWidget infoWidget;
    Akonadi::Item item(4);
    infoWidget.setContacts(Akonadi::Item::List()<<item);
    QStackedWidget *stackedWidget = qFindChild<QStackedWidget *>(&infoWidget, QLatin1String("stackedwidget"));
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QLatin1String("contactwidget"));
    infoWidget.setContacts(Akonadi::Item::List());
    QCOMPARE(stackedWidget->currentWidget()->objectName(), QLatin1String("nocontact"));
}

QTEST_KDEMAIN(MergeContactInfoWidgetTest, GUI )
