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

#include "followupreminderinfotest.h"
#include "../followupreminderinfo.h"
#include <qtest_kde.h>

FollowUpReminderInfoTest::FollowUpReminderInfoTest()
{
}

void FollowUpReminderInfoTest::shouldHaveDefaultValue()
{
    FollowUpReminderInfo info;
    QCOMPARE(info.id(), Akonadi::Item::Id(-1));
    QCOMPARE(info.messageId(), QString());
    QCOMPARE(info.isValid(), false);
}

void FollowUpReminderInfoTest::shoudBeNotValid()
{
    FollowUpReminderInfo info;
    //We need a Akonadi::Id valid and a messageId not empty and a valid date
    info.setMessageId(QLatin1String("foo"));
    QCOMPARE(info.isValid(), false);

    QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDateTime(date));
    QCOMPARE(info.isValid(), false);

    info.setId(Akonadi::Item::Id(42));
    QCOMPARE(info.isValid(), true);
}

QTEST_KDEMAIN(FollowUpReminderInfoTest, NoGUI)
