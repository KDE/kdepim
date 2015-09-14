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

#include "invalidfilterinfotest.h"
#include "../filter/invalidfilters/invalidfilterinfo.h"

#include <qtest.h>

InvalidFilterInfoTest::InvalidFilterInfoTest(QObject *parent)
    : QObject(parent)
{

}

InvalidFilterInfoTest::~InvalidFilterInfoTest()
{

}

void InvalidFilterInfoTest::shouldAddEmptyValue()
{
    MailCommon::InvalidFilterInfo info;
    QVERIFY(info.name().isEmpty());
    QVERIFY(info.information().isEmpty());
}

void InvalidFilterInfoTest::shouldAssignValueFromConstructor()
{
    const QString name = QStringLiteral("foo");
    const QString information = QStringLiteral("bla");
    MailCommon::InvalidFilterInfo info(name, information);
    QCOMPARE(info.name(), name);
    QCOMPARE(info.information(), information);
}

void InvalidFilterInfoTest::shouldAssignValue()
{
    MailCommon::InvalidFilterInfo info;
    const QString name = QStringLiteral("foo");
    const QString information = QStringLiteral("bla");
    info.setName(name);
    info.setInformation(information);
    QCOMPARE(info.name(), name);
    QCOMPARE(info.information(), information);
}

void InvalidFilterInfoTest::shouldBeEqual()
{
    MailCommon::InvalidFilterInfo info;
    const QString name = QStringLiteral("foo");
    const QString information = QStringLiteral("bla");
    info.setName(name);
    info.setInformation(information);
    MailCommon::InvalidFilterInfo copyInfo;
    copyInfo = info;
    QVERIFY(copyInfo == info);
}

QTEST_MAIN(InvalidFilterInfoTest)
