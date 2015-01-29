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

#include "blacklistbalooemailcompletionwidgettest.h"
#include "../blacklistbalooemailcompletionwidget.h"
#include "../blacklistbalooemaillist.h"
#include <QLabel>
#include <klineedit.h>
#include <kpushbutton.h>
#include <qtest_kde.h>

BlackListBalooEmailCompletionWidgetTest::BlackListBalooEmailCompletionWidgetTest(QObject *parent)
    : QObject(parent)
{

}

BlackListBalooEmailCompletionWidgetTest::~BlackListBalooEmailCompletionWidgetTest()
{

}

void BlackListBalooEmailCompletionWidgetTest::shouldHaveDefaultValue()
{
    KPIM::BlackListBalooEmailCompletionWidget widget;
    QLabel *searchLabel = qFindChild<QLabel *>(&widget, QLatin1String("search_label"));
    QVERIFY(searchLabel);

    KLineEdit *searchLineEdit = qFindChild<KLineEdit *>(&widget, QLatin1String("search_lineedit"));
    QVERIFY(searchLineEdit);
    QVERIFY(searchLineEdit->isClearButtonShown());
    QVERIFY(searchLineEdit->trapReturnKey());
    QVERIFY(searchLineEdit->text().isEmpty());


    KPushButton *seachButton = qFindChild<KPushButton *>(&widget, QLatin1String("search_button"));
    QVERIFY(seachButton);
    QVERIFY(!seachButton->isEnabled());

    KPIM::BlackListBalooEmailList *emailList = qFindChild<KPIM::BlackListBalooEmailList *>(&widget, QLatin1String("email_list"));
    QVERIFY(emailList);

    KPushButton *selectButton = qFindChild<KPushButton *>(&widget, QLatin1String("select_email"));
    QVERIFY(selectButton);
    KPushButton *unselectButton = qFindChild<KPushButton *>(&widget, QLatin1String("unselect_email"));
    QVERIFY(unselectButton);


}

void BlackListBalooEmailCompletionWidgetTest::shouldEnablePushButtonWhenTestSizeSupperiorToTwo()
{
    KPIM::BlackListBalooEmailCompletionWidget widget;
    KLineEdit *searchLineEdit = qFindChild<KLineEdit *>(&widget, QLatin1String("search_lineedit"));
    KPushButton *seachButton = qFindChild<KPushButton *>(&widget, QLatin1String("search_button"));
    QVERIFY(!seachButton->isEnabled());
    searchLineEdit->setText(QLatin1String("fo"));
    QVERIFY(!seachButton->isEnabled());
    searchLineEdit->setText(QLatin1String("foo"));
    QVERIFY(seachButton->isEnabled());

    searchLineEdit->setText(QLatin1String("o  "));
    QVERIFY(!seachButton->isEnabled());
    searchLineEdit->setText(QLatin1String(" o "));
    QVERIFY(!seachButton->isEnabled());
}

QTEST_KDEMAIN(BlackListBalooEmailCompletionWidgetTest, GUI)
