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

#include "sendlaterinfotest.h"
#include "../sendlaterinfo.h"
#include <qtest_kde.h>
#include <QDateTime>
#include <KConfigGroup>

SendLaterInfoTest::SendLaterInfoTest()
{
}

void SendLaterInfoTest::shouldHaveDefaultValue()
{
    SendLater::SendLaterInfo info;
    QCOMPARE(info.itemId(), Akonadi::Item::Id(-1));
    QCOMPARE(info.isRecurrence(), false);
    QVERIFY(!info.dateTime().isValid());
    QVERIFY(!info.lastDateTimeSend().isValid());
    QCOMPARE(info.to(), QString());
    QCOMPARE(info.subject(), QString());
    QVERIFY(!info.isValid());
    QCOMPARE(info.recurrenceUnit(), SendLater::SendLaterInfo::Days);
    QCOMPARE(info.recurrenceEachValue(), 1);
}

void SendLaterInfoTest::shouldRestoreFromSettings()
{
    SendLater::SendLaterInfo info;
    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    info.setItemId(Akonadi::Item::Id(42));
    info.setSubject(QLatin1String("Subject"));
    info.setRecurrence(true);
    info.setRecurrenceEachValue(5);
    info.setRecurrenceUnit(SendLater::SendLaterInfo::Years);
    const QDate date(2014,1,1);
    info.setDateTime(QDateTime(date));
    info.setLastDateTimeSend(QDateTime(date));

    KConfigGroup grp(KGlobal::config(), "testsettings");
    info.writeConfig(grp);

    SendLater::SendLaterInfo restoreInfo(grp);
    QCOMPARE(info, restoreInfo);

}

QTEST_KDEMAIN(SendLaterInfoTest, NoGUI)
