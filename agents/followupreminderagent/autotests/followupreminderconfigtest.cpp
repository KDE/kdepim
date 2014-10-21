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


#include "followupreminderconfigtest.h"
#include "../followupreminderutil.h"
#include "../followupreminderinfo.h"
#include <qtest_kde.h>
#include <KSharedConfig>


FollowUpReminderConfigTest::FollowUpReminderConfigTest(QObject *parent)
    : QObject(parent)
{

}

FollowUpReminderConfigTest::~FollowUpReminderConfigTest()
{

}

void FollowUpReminderConfigTest::init()
{
    mConfig = KSharedConfig::openConfig(QLatin1String("test-followupreminder.rc"));
    mFollowupRegExpFilter = QRegExp( QLatin1String("FollowupReminderItem \\d+") );
    cleanup();
}

void FollowUpReminderConfigTest::cleanup()
{
    const QStringList filterGroups = mConfig->groupList();
    foreach ( const QString &group, filterGroups ) {
        mConfig->deleteGroup( group );
    }
    mConfig->sync();
    mConfig->reparseConfiguration();
}

void FollowUpReminderConfigTest::cleanupTestCase()
{
    //Make sure to clean config
    cleanup();
}

void FollowUpReminderConfigTest::shouldConfigBeEmpty()
{
    const QStringList filterGroups = mConfig->groupList();
    QCOMPARE(filterGroups.isEmpty(), true);
}

void FollowUpReminderConfigTest::shouldAddAnItem()
{
    FollowUpReminder::FollowUpReminderInfo info;
    info.setMessageId(QLatin1String("foo"));
    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDate(date));
    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    const QStringList itemList = mConfig->groupList().filter( mFollowupRegExpFilter );

    QCOMPARE(itemList.isEmpty(), false);
    QCOMPARE(itemList.count(), 1);
    QCOMPARE(mConfig->hasGroup(QLatin1String("General")), true);
}

void FollowUpReminderConfigTest::shouldNotAddAnInvalidItem()
{
    FollowUpReminder::FollowUpReminderInfo info;
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    const QStringList itemList = mConfig->groupList().filter( mFollowupRegExpFilter );
    QCOMPARE(itemList.isEmpty(), true);
}

QTEST_KDEMAIN(FollowUpReminderConfigTest, NoGUI)
