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

#include "sendlaterconfigtest.h"
#include <qtest_kde.h>


SendLaterConfigTest::SendLaterConfigTest(QObject *parent)
    : QObject(parent)
{

}

SendLaterConfigTest::~SendLaterConfigTest()
{

}

void SendLaterConfigTest::init()
{
    mConfig = KSharedConfig::openConfig(QLatin1String("test-sendlateragent.rc"));
    mFollowupRegExpFilter = QRegExp( QLatin1String("SendLaterItem \\d+") );
    cleanup();
}

void SendLaterConfigTest::cleanup()
{
    const QStringList filterGroups = mConfig->groupList();
    foreach ( const QString &group, filterGroups ) {
        mConfig->deleteGroup( group );
    }
    mConfig->sync();
    mConfig->reparseConfiguration();
}

void SendLaterConfigTest::cleanupTestCase()
{
    //Make sure to clean config
    cleanup();
}

void SendLaterConfigTest::shouldConfigBeEmpty()
{
    const QStringList filterGroups = mConfig->groupList();
    QCOMPARE(filterGroups.isEmpty(), true);
}


QTEST_KDEMAIN(SendLaterConfigTest, NoGUI)
