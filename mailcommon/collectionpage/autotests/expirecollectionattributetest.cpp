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

#include "expirecollectionattributetest.h"
#include "../expirecollectionattribute.h"
#include <qtest_kde.h>
ExpireCollectionAttributeTest::ExpireCollectionAttributeTest(QObject *parent)
    : QObject(parent)
{

}

ExpireCollectionAttributeTest::~ExpireCollectionAttributeTest()
{

}

void ExpireCollectionAttributeTest::shouldHaveDefaultValue()
{
    MailCommon::ExpireCollectionAttribute attr;
    QVERIFY(!attr.isAutoExpire());
    QCOMPARE(attr.unreadExpireAge(), 28);
    QCOMPARE(attr.readExpireAge(), 14);
    QCOMPARE(attr.expireAction(), MailCommon::ExpireCollectionAttribute::ExpireDelete);
    QCOMPARE(attr.unreadExpireUnits(), MailCommon::ExpireCollectionAttribute::ExpireNever);
    QCOMPARE(attr.readExpireUnits(), MailCommon::ExpireCollectionAttribute::ExpireNever);
    QCOMPARE(attr.expireToFolderId(), (qint64) - 1);
}

void ExpireCollectionAttributeTest::shouldAssignValue_data()
{
    //TODO
}

void ExpireCollectionAttributeTest::shouldAssignValue()
{
    MailCommon::ExpireCollectionAttribute attr;
}

QTEST_KDEMAIN(ExpireCollectionAttributeTest, NoGUI)
