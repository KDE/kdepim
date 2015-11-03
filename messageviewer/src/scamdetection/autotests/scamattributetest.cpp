/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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
#include "scamattributetest.h"
#include "../scamattribute.h"
#include <qtest.h>
ScamAttributeTest::ScamAttributeTest(QObject *parent)
    : QObject(parent)
{

}

ScamAttributeTest::~ScamAttributeTest()
{

}

void ScamAttributeTest::shouldHaveDefaultValue()
{
    MessageViewer::ScamAttribute attr;
    QVERIFY(!attr.isAScam());
}

void ScamAttributeTest::shouldAffectValue()
{
    MessageViewer::ScamAttribute attr;
    bool isScam = false;
    attr.setIsAScam(isScam);
    QCOMPARE(attr.isAScam(), isScam);
    isScam = true;
    attr.setIsAScam(isScam);
    QCOMPARE(attr.isAScam(), isScam);
}

void ScamAttributeTest::shouldDeserializeValue()
{
    MessageViewer::ScamAttribute attr;
    const bool isScam = true;
    attr.setIsAScam(isScam);
    const QByteArray ba = attr.serialized();
    MessageViewer::ScamAttribute result;
    result.deserialize(ba);
    QVERIFY(result == attr);
}

void ScamAttributeTest::shouldCloneAttribute()
{
    MessageViewer::ScamAttribute attr;
    const bool isScam = true;
    attr.setIsAScam(isScam);
    MessageViewer::ScamAttribute *cloneAttr = attr.clone();
    QCOMPARE(attr.isAScam(), cloneAttr->isAScam());
    delete cloneAttr;
}

void ScamAttributeTest::shouldHaveType()
{
    MessageViewer::ScamAttribute attr;
    QCOMPARE(attr.type(), QByteArray("ScamAttribute"));
}

QTEST_MAIN(ScamAttributeTest)
