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

#include "imapaclattributetest.h"
#include "../imapaclattribute.h"
#include <qtest_kde.h>
ImapAclAttributeTest::ImapAclAttributeTest(QObject *parent)
    : QObject(parent)
{

}

ImapAclAttributeTest::~ImapAclAttributeTest()
{

}

void ImapAclAttributeTest::shouldHaveDefaultValue()
{
    PimCommon::ImapAclAttribute attr;
    QVERIFY(attr.oldRights().isEmpty());
    QVERIFY(attr.rights().isEmpty());
}

void ImapAclAttributeTest::shouldBuildAttribute()
{
    QMap<QByteArray, KIMAP::Acl::Rights> right;
    right.insert("test", KIMAP::Acl::Admin);
    right.insert("foo", KIMAP::Acl::Admin);

    QMap<QByteArray, KIMAP::Acl::Rights> oldright;
    right.insert("test", KIMAP::Acl::Delete);
    right.insert("foo", KIMAP::Acl::Delete);
    PimCommon::ImapAclAttribute attr(right, oldright);
    QCOMPARE(attr.oldRights(), oldright);
    QCOMPARE(attr.rights(), right);
}

void ImapAclAttributeTest::shouldAssignValue()
{
    PimCommon::ImapAclAttribute attr;
    QMap<QByteArray, KIMAP::Acl::Rights> right;
    right.insert("test", KIMAP::Acl::Admin);
    right.insert("foo", KIMAP::Acl::Admin);
    attr.setRights(right);
    QCOMPARE(attr.rights(), right);
}

void ImapAclAttributeTest::shouldCloneAttr()
{
    PimCommon::ImapAclAttribute attr;
    QMap<QByteArray, KIMAP::Acl::Rights> right;
    right.insert("test", KIMAP::Acl::Admin);
    right.insert("foo", KIMAP::Acl::Admin);
    attr.setRights(right);
    PimCommon::ImapAclAttribute *clone = attr.clone();
    QVERIFY(attr==*clone);
    delete clone;
}

void ImapAclAttributeTest::shouldSerializedAttribute()
{
    QMap<QByteArray, KIMAP::Acl::Rights> right;
    right.insert("test", KIMAP::Acl::Admin);
    right.insert("foo", KIMAP::Acl::Admin);

    QMap<QByteArray, KIMAP::Acl::Rights> oldright;
    right.insert("test", KIMAP::Acl::Delete);
    right.insert("foo", KIMAP::Acl::Delete);
    PimCommon::ImapAclAttribute attr(right, oldright);
    const QByteArray ba = attr.serialized();
    PimCommon::ImapAclAttribute result;
    result.deserialize(ba);
    QVERIFY(attr==result);
}

void ImapAclAttributeTest::shouldHaveType()
{
    PimCommon::ImapAclAttribute attr;
    QCOMPARE(attr.type(), QByteArray("imapacl"));
}

QTEST_KDEMAIN(ImapAclAttributeTest, NoGUI)
