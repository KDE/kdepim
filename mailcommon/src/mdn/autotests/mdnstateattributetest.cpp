/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "mdnstateattributetest.h"
#include "../mdnstateattribute.h"
#include <qtest.h>

MDNStateAttributeTest::MDNStateAttributeTest(QObject *parent)
    : QObject(parent)
{

}

MDNStateAttributeTest::~MDNStateAttributeTest()
{

}

void MDNStateAttributeTest::shouldHaveDefaultValue()
{
    MailCommon::MDNStateAttribute attr;
    QCOMPARE(attr.mdnState(), MailCommon::MDNStateAttribute::MDNStateUnknown);
}

void MDNStateAttributeTest::shouldHaveType()
{
    MailCommon::MDNStateAttribute attr;
    QCOMPARE(attr.type(), QByteArray("MDNStateAttribute"));
}

void MDNStateAttributeTest::shouldSerializedAttribute()
{
    MailCommon::MDNStateAttribute attr;

    attr.setMDNState(MailCommon::MDNStateAttribute::MDNDenied);
    QCOMPARE(attr.mdnState(), MailCommon::MDNStateAttribute::MDNDenied);
    const QByteArray ba = attr.serialized();
    MailCommon::MDNStateAttribute result;
    result.deserialize(ba);
    QVERIFY(attr == result);
}

void MDNStateAttributeTest::shouldCloneAttribute()
{
    MailCommon::MDNStateAttribute attr;
    attr.setMDNState(MailCommon::MDNStateAttribute::MDNDenied);

    MailCommon::MDNStateAttribute *result = attr.clone();
    QVERIFY(attr == *result);
    delete result;
}

QTEST_MAIN(MDNStateAttributeTest)
