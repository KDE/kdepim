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

#include "mergecontactselectinformationscrollareatest.h"
#include <qtest.h>
#include "../widgets/mergecontactselectinformationwidget.h"
#include "../widgets/mergecontactselectinformationscrollarea.h"
#include "../widgets/mergecontactinfowidget.h"
#include <QScrollArea>
#include <QPushButton>
#include <QStackedWidget>

MergeContactSelectInformationScrollAreaTest::MergeContactSelectInformationScrollAreaTest(QObject *parent)
    : QObject(parent)
{

}

MergeContactSelectInformationScrollAreaTest::~MergeContactSelectInformationScrollAreaTest()
{

}

void MergeContactSelectInformationScrollAreaTest::shouldHaveDefaultValue()
{
    KABMergeContacts::MergeContactSelectInformationScrollArea w;
    QScrollArea *area = qFindChild<QScrollArea *>(&w, QStringLiteral("scrollarea"));
    QVERIFY(area);
    QPushButton *mergeButton = qFindChild<QPushButton *>(&w, QStringLiteral("merge"));
    QVERIFY(mergeButton);

    QStackedWidget *stackedwidget = qFindChild<QStackedWidget *>(&w, QStringLiteral("stackwidget"));
    QVERIFY(stackedwidget);

    KABMergeContacts::MergeContactSelectInformationWidget *widget =
        qFindChild<KABMergeContacts::MergeContactSelectInformationWidget *>(&w, QStringLiteral("selectinformationwidget"));
    QVERIFY(widget);

    KABMergeContacts::MergeContactInfoWidget *mergecontactinfowidget = qFindChild<KABMergeContacts::MergeContactInfoWidget *>(&w, QStringLiteral("mergedcontactwidget"));
    QVERIFY(mergecontactinfowidget);

    QWidget *selectWidget = qFindChild<QWidget *>(&w, QStringLiteral("selectwidget"));
    QVERIFY(selectWidget);

    QCOMPARE(stackedwidget->currentWidget(), selectWidget);
}

QTEST_MAIN(MergeContactSelectInformationScrollAreaTest)
