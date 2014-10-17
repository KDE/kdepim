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


#include "followupremindernoanswerdialogtest.h"
#include "../followupremindernoanswerdialog.h"
#include "../followupreminderinfowidget.h"
#include "../followupreminderinfo.h"
#include <QTreeWidget>
#include <qtest.h>

FollowupReminderNoAnswerDialogTest::FollowupReminderNoAnswerDialogTest(QObject *parent)
    : QObject(parent)
{

}

FollowupReminderNoAnswerDialogTest::~FollowupReminderNoAnswerDialogTest()
{

}

void FollowupReminderNoAnswerDialogTest::shouldHaveDefaultValues()
{
    FollowUpReminderNoAnswerDialog dlg;
    FollowUpReminderInfoWidget *infowidget = qFindChild<FollowUpReminderInfoWidget *>(&dlg, QLatin1String("FollowUpReminderInfoWidget"));
    QVERIFY(infowidget);

    QTreeWidget *treeWidget = qFindChild<QTreeWidget *>(infowidget, QLatin1String("treewidget"));
    QVERIFY(treeWidget);

    QCOMPARE(treeWidget->topLevelItemCount(), 0);
}

void FollowupReminderNoAnswerDialogTest::shouldAddItemInTreeList()
{
    FollowUpReminderNoAnswerDialog dlg;
    FollowUpReminderInfoWidget *infowidget = qFindChild<FollowUpReminderInfoWidget *>(&dlg, QLatin1String("FollowUpReminderInfoWidget"));
    QTreeWidget *treeWidget = qFindChild<QTreeWidget *>(infowidget, QLatin1String("treewidget"));
    QList<FollowUpReminder::FollowUpReminderInfo *> lstInfo;
    for (int i = 0; i<10; ++i) {
        FollowUpReminder::FollowUpReminderInfo *info = new FollowUpReminder::FollowUpReminderInfo();
        lstInfo.append(info);
    }
    dlg.setInfo(lstInfo);
    //We load invalid infos.
    QCOMPARE(treeWidget->topLevelItemCount(), 0);

    //Load valid infos
    for (int i = 0; i<10; ++i) {
        FollowUpReminder::FollowUpReminderInfo *info = new FollowUpReminder::FollowUpReminderInfo();
        info->setOriginalMessageItemId(42);
        info->setMessageId(QLatin1String("foo"));
        info->setFollowUpReminderDate(QDateTime::currentDateTime());
        info->setTo(QLatin1String("To"));
        lstInfo.append(info);
    }

    dlg.setInfo(lstInfo);
    QCOMPARE(treeWidget->topLevelItemCount(), 10);
}


QTEST_MAIN(FollowupReminderNoAnswerDialogTest)
