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


#include "newmailnotifierattributetest.h"
#include "../newmailnotifierattribute.h"
#include <qtest_kde.h>

NewMailNotifierAttributeTest::NewMailNotifierAttributeTest(QObject *parent)
    : QObject(parent)
{

}

NewMailNotifierAttributeTest::~NewMailNotifierAttributeTest()
{

}

void NewMailNotifierAttributeTest::shouldHaveDefaultValue()
{
    MailCommon::NewMailNotifierAttribute attr;
    QVERIFY(!attr.ignoreNewMail());
}

void NewMailNotifierAttributeTest::shouldSetIgnoreNotification()
{
    MailCommon::NewMailNotifierAttribute attr;
    bool ignore = false;
    attr.setIgnoreNewMail(ignore);
    QCOMPARE(attr.ignoreNewMail(), ignore);
    ignore = true;
    attr.setIgnoreNewMail(ignore);
    QCOMPARE(attr.ignoreNewMail(), ignore);
}

void NewMailNotifierAttributeTest::shouldSerializedData()
{
    MailCommon::NewMailNotifierAttribute attr;
    attr.setIgnoreNewMail(true);
    QByteArray ba = attr.serialized();
    MailCommon::NewMailNotifierAttribute result;
    result.deserialize(ba);
    QVERIFY(attr == result);
}

QTEST_KDEMAIN(NewMailNotifierAttributeTest, NoGUI)
