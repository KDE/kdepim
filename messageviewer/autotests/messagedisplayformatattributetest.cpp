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

#include "messagedisplayformatattributetest.h"
#include "../src/viewer/messagedisplayformatattribute.h"
#include <qtest.h>

MessageDisplayFormatAttributeTest::MessageDisplayFormatAttributeTest(QObject *parent)
    : QObject(parent)
{

}

MessageDisplayFormatAttributeTest::~MessageDisplayFormatAttributeTest()
{

}

void MessageDisplayFormatAttributeTest::shouldHaveDefaultValue()
{
    MessageViewer::MessageDisplayFormatAttribute attr;
    QVERIFY(!attr.remoteContent());
    QCOMPARE(attr.messageFormat(), MessageViewer::Viewer::UseGlobalSetting);
}

void MessageDisplayFormatAttributeTest::shouldChangeRemoteValue()
{
    MessageViewer::MessageDisplayFormatAttribute attr;
    attr.setRemoteContent(true);
    QVERIFY(attr.remoteContent());
}

void MessageDisplayFormatAttributeTest::shouldChangeMessageFormat()
{
    MessageViewer::Viewer::DisplayFormatMessage format = MessageViewer::Viewer::Html;
    MessageViewer::MessageDisplayFormatAttribute attr;
    attr.setMessageFormat(format);
    QCOMPARE(attr.messageFormat(), format);

    format = MessageViewer::Viewer::Text;
    attr.setMessageFormat(format);
    QCOMPARE(attr.messageFormat(), format);

    format = MessageViewer::Viewer::UseGlobalSetting;
    attr.setMessageFormat(format);
    QCOMPARE(attr.messageFormat(), format);

}

void MessageDisplayFormatAttributeTest::shouldDeserializeValue()
{
    MessageViewer::Viewer::DisplayFormatMessage format = MessageViewer::Viewer::Html;
    MessageViewer::MessageDisplayFormatAttribute attr;
    attr.setMessageFormat(format);
    attr.setRemoteContent(true);
    const QByteArray ba = attr.serialized();
    MessageViewer::MessageDisplayFormatAttribute result;
    result.deserialize(ba);
    QVERIFY(attr == result);
}

void MessageDisplayFormatAttributeTest::shouldCloneAttribute()
{
    MessageViewer::Viewer::DisplayFormatMessage format = MessageViewer::Viewer::Html;
    MessageViewer::MessageDisplayFormatAttribute attr;
    attr.setMessageFormat(format);
    attr.setRemoteContent(true);
    MessageViewer::MessageDisplayFormatAttribute *result = attr.clone();
    QVERIFY(attr == *result);
    delete result;
}

void MessageDisplayFormatAttributeTest::shouldDefineType()
{
    MessageViewer::MessageDisplayFormatAttribute attr;
    QCOMPARE(attr.type(), QByteArray("MessageDisplayFormatAttribute"));
}

QTEST_MAIN(MessageDisplayFormatAttributeTest)
