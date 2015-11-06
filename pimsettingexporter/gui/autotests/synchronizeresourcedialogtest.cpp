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

#include "synchronizeresourcedialogtest.h"
#include "../dialog/synchronizeresourcedialog.h"
#include <QDialogButtonBox>
#include <QTest>
#include <QListWidget>
#include <KListWidgetSearchLine>
#include <QStandardPaths>

#include <QLabel>

SynchronizeResourceDialogTest::SynchronizeResourceDialogTest(QObject *parent)
    : QObject(parent)
{
    QStandardPaths::setTestModeEnabled(true);
}

SynchronizeResourceDialogTest::~SynchronizeResourceDialogTest()
{

}

void SynchronizeResourceDialogTest::shouldHaveDefaultValue()
{
    SynchronizeResourceDialog dlg;
    QDialogButtonBox *buttonBox = dlg.findChild<QDialogButtonBox *>(QStringLiteral("buttonbox"));
    QVERIFY(buttonBox);

    QListWidget *listWidget = dlg.findChild<QListWidget *>(QStringLiteral("listresourcewidget"));
    QVERIFY(listWidget);

    KListWidgetSearchLine *searchLine = dlg.findChild<KListWidgetSearchLine *>(QStringLiteral("listwidgetsearchline"));
    QVERIFY(searchLine);

    QLabel *label = dlg.findChild<QLabel *>(QStringLiteral("label"));
    QVERIFY(label);
    QVERIFY(label->wordWrap());

    QVERIFY(dlg.resources().isEmpty());
}

void SynchronizeResourceDialogTest::shouldNotEmptyList()
{
    SynchronizeResourceDialog dlg;
    QListWidget *listWidget = dlg.findChild<QListWidget *>(QStringLiteral("listresourcewidget"));
    QHash<QString, QString> lst;
    lst.insert(QStringLiteral("foo"), QStringLiteral("foo"));
    lst.insert(QStringLiteral("faa"), QStringLiteral("faa"));
    dlg.setResources(lst);
    QCOMPARE(dlg.resources().count(), 0);
    QCOMPARE(listWidget->count(), lst.count());
}

QTEST_MAIN(SynchronizeResourceDialogTest)
