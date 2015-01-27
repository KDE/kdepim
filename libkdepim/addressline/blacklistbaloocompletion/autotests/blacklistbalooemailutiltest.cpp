/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "blacklistbalooemailutiltest.h"
#include "../blacklistbalooemailutil.h"
#include <qtest.h>
BlackListBalooEmailUtilTest::BlackListBalooEmailUtilTest(QObject *parent)
    : QObject(parent)
{

}

BlackListBalooEmailUtilTest::~BlackListBalooEmailUtilTest()
{

}

void BlackListBalooEmailUtilTest::shouldReturnEmptyResult()
{
    KPIM::BlackListBalooEmailUtil util;
    QVERIFY(util.createNewBlackList().isEmpty());
}

void BlackListBalooEmailUtilTest::shouldDontChangeWhenNotChanged()
{
    KPIM::BlackListBalooEmailUtil util;
    const QStringList lst = QStringList() << QLatin1String("foo") << QLatin1String("foo1") << QLatin1String("foo2");
    util.initialBlackList(lst);
    QCOMPARE(util.createNewBlackList(), lst);
}

void BlackListBalooEmailUtilTest::shouldCreateNewList()
{
    KPIM::BlackListBalooEmailUtil util;
    const QStringList lst = QStringList() << QLatin1String("foo") << QLatin1String("foo1") << QLatin1String("foo2");
    util.initialBlackList(lst);
    QHash<QString, bool> newList;
    newList.insert(QLatin1String("foo"), false);
    util.newBlackList(newList);
    QCOMPARE(util.createNewBlackList(), QStringList() << QLatin1String("foo1") << QLatin1String("foo2"));
}

void BlackListBalooEmailUtilTest::shouldAddNewElements()
{
    KPIM::BlackListBalooEmailUtil util;
    QHash<QString, bool> newList;
    newList.insert(QLatin1String("foo"), false);
    newList.insert(QLatin1String("foo1"), false);
    newList.insert(QLatin1String("foo2"), false);
    util.newBlackList(newList);
    QCOMPARE(util.createNewBlackList().count(), 0);

    newList.clear();
    newList.insert(QLatin1String("foo"), true);
    newList.insert(QLatin1String("foo1"), true);
    newList.insert(QLatin1String("foo2"), true);
    util.newBlackList(newList);
    QCOMPARE(util.createNewBlackList().count(), 3);
}

QTEST_MAIN(BlackListBalooEmailUtilTest)

