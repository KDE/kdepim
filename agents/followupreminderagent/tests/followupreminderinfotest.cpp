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
#include <KConfigGroup>
#include <qtest_kde.h>

FollowUpReminderInfoTest::FollowUpReminderInfoTest()
{
}

void FollowUpReminderInfoTest::shouldHaveDefaultValue()
{
    FollowUpReminder::FollowUpReminderInfo info;
    QCOMPARE(info.id(), Akonadi::Item::Id(-1));
    QCOMPARE(info.messageId(), QString());
    QCOMPARE(info.isValid(), false);
    QCOMPARE(info.to(), QString());
    QCOMPARE(info.subject(), QString());
}

void FollowUpReminderInfoTest::shoudBeNotValid()
{
    FollowUpReminder::FollowUpReminderInfo info;
    //We need a Akonadi::Id valid and a messageId not empty and a valid date and a "To" not empty
    info.setMessageId(QLatin1String("foo"));
    QCOMPARE(info.isValid(), false);

    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDateTime(date));
    QCOMPARE(info.isValid(), false);

    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    QCOMPARE(info.isValid(), false);

    info.setId(Akonadi::Item::Id(42));
    QCOMPARE(info.isValid(), true);
}

void FollowUpReminderInfoTest::shoudBeValidEvenIfSubjectIsEmpty()
{
    FollowUpReminder::FollowUpReminderInfo info;
    //We need a Akonadi::Id valid and a messageId not empty and a valid date and a "To" not empty
    info.setMessageId(QLatin1String("foo"));
    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDateTime(date));
    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    info.setId(Akonadi::Item::Id(42));
    QCOMPARE(info.isValid(), true);
}

void FollowUpReminderInfoTest::shouldRestoreFromSettings()
{
    FollowUpReminder::FollowUpReminderInfo info;
    info.setMessageId(QLatin1String("foo"));
    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDateTime(date));
    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    info.setId(Akonadi::Item::Id(42));
    info.setSubject(QLatin1String("Subject"));

    KConfigGroup grp(KGlobal::config(), "testsettings");
    info.writeConfig(grp);

    FollowUpReminder::FollowUpReminderInfo restoreInfo(grp);
    QCOMPARE(info, restoreInfo);
}



QTEST_KDEMAIN(FollowUpReminderInfoTest, NoGUI)
