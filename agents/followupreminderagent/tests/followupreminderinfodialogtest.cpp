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

#include "followupreminderinfodialogtest.h"
#include "../followupreminderinfodialog.h"
#include "../followupreminderinfowidget.h"
#include <qtest_kde.h>


FollowupReminderInfoDialogTest::FollowupReminderInfoDialogTest(QObject *parent)
    : QObject(parent)
{

}

FollowupReminderInfoDialogTest::~FollowupReminderInfoDialogTest()
{

}

void FollowupReminderInfoDialogTest::shouldHaveDefaultValues()
{
    FollowUpReminderInfoDialog dlg;
    FollowUpReminderInfoWidget *infowidget = qFindChild<FollowUpReminderInfoWidget *>(&dlg, QLatin1String("FollowUpReminderInfoWidget"));
    QVERIFY(infowidget);

    QTreeWidget *treeWidget = qFindChild<QTreeWidget *>(infowidget, QLatin1String("treewidget"));
    QVERIFY(treeWidget);

    QCOMPARE(treeWidget->topLevelItemCount(), 0);
}

void FollowupReminderInfoDialogTest::shouldAddItemInTreeList()
{
    FollowUpReminderInfoDialog dlg;
    FollowUpReminderInfoWidget *infowidget = qFindChild<FollowUpReminderInfoWidget *>(&dlg, QLatin1String("FollowUpReminderInfoWidget"));
    QTreeWidget *treeWidget = qFindChild<QTreeWidget *>(infowidget, QLatin1String("treewidget"));


}

QTEST_KDEMAIN(FollowupReminderInfoDialogTest, GUI)
