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

#include "mergecontactselectlistwidgettest.h"
#include "merge/job/mergecontacts.h"
#include "merge/widgets/mergecontactselectlistwidget.h"
#include <QListWidget>
#include <qlabel.h>
#include <qtest.h>

MergeContactSelectListWidgetTest::MergeContactSelectListWidgetTest(QObject *parent)
    : QObject(parent)
{

}

MergeContactSelectListWidgetTest::~MergeContactSelectListWidgetTest()
{

}

void MergeContactSelectListWidgetTest::shouldHaveDefaultValue()
{
    KABMergeContacts::MergeContactSelectListWidget selectListWidget;

    QLabel *title = qFindChild<QLabel *>(&selectListWidget, QStringLiteral("title"));
    QVERIFY(title);
    QListWidget *listWidget = qFindChild<QListWidget *>(&selectListWidget, QStringLiteral("listwidget"));
    QVERIFY(listWidget);
    QCOMPARE(selectListWidget.selectedContact(), -1);
    QCOMPARE(selectListWidget.conflictType(), KABMergeContacts::MergeContacts::None);
}

QTEST_MAIN(MergeContactSelectListWidgetTest)
