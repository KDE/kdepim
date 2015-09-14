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

#include "imapaclattributetest.h"
#include "../imapaclattribute.h"
#include <qtest.h>

typedef QMap<QByteArray, KIMAP::Acl::Rights> ImapAcl;
Q_DECLARE_METATYPE(ImapAcl)
Q_DECLARE_METATYPE(KIMAP::Acl::Rights)

using namespace PimCommon;

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
    QVERIFY(!attr.myRights());
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
    ImapAclAttribute *clone = attr.clone();
    QVERIFY(attr == *clone);
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
    QVERIFY(attr == result);
}

void ImapAclAttributeTest::shouldHaveType()
{
    PimCommon::ImapAclAttribute attr;
    QCOMPARE(attr.type(), QByteArray("imapacl"));
}

void ImapAclAttributeTest::testMyRights()
{
    ImapAclAttribute attr;
    KIMAP::Acl::Rights myRight = KIMAP::Acl::Admin;

    attr.setMyRights(myRight);
    QCOMPARE(attr.myRights(), myRight);

    ImapAclAttribute *clone = attr.clone();
    QCOMPARE(clone->myRights(), myRight);

    QVERIFY(*clone == attr);

    clone->setMyRights(KIMAP::Acl::Custom0);
    QVERIFY(!(*clone == attr));
    delete clone;
}

void ImapAclAttributeTest::testDeserialize_data()
{
    QTest::addColumn<ImapAcl>("rights");
    QTest::addColumn<KIMAP::Acl::Rights>("myRights");
    QTest::addColumn<QByteArray>("serialized");

    KIMAP::Acl::Rights rights = KIMAP::Acl::None;

    {
        ImapAcl acl;
        QTest::newRow("empty") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray(" %% ");
    }

    {
        ImapAcl acl;
        acl.insert("user@host", rights);
        QTest::newRow("none") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("user@host  %% ");
    }

    {
        ImapAcl acl;
        acl.insert("user@host", KIMAP::Acl::Lookup);
        QTest::newRow("lookup") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("user@host l %% ");
    }

    {
        ImapAcl acl;
        acl.insert("user@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
        QTest::newRow("lookup/read") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("user@host lr %% ");
    }

    {
        ImapAcl acl;
        acl.insert("user@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
        acl.insert("otheruser@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
        QTest::newRow("lookup/read") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("otheruser@host lr % user@host lr %% ");
    }

    {
        QTest::newRow("myrights") << ImapAcl() << KIMAP::Acl::rightsFromString("lrswipckxtdaen") << QByteArray(" %%  %% lrswipckxtdaen");
    }
}

void ImapAclAttributeTest::testDeserialize()
{
    QFETCH(ImapAcl, rights);
    QFETCH(KIMAP::Acl::Rights, myRights);
    QFETCH(QByteArray, serialized);

    ImapAclAttribute deserializeAttr;
    deserializeAttr.deserialize(serialized);
    QCOMPARE(deserializeAttr.rights(), rights);
    QCOMPARE(deserializeAttr.myRights(), myRights);
}

void ImapAclAttributeTest::testSerializeDeserialize_data()
{
    QTest::addColumn<ImapAcl>("rights");
    QTest::addColumn<KIMAP::Acl::Rights>("myRights");
    QTest::addColumn<QByteArray>("serialized");
    QTest::addColumn<QByteArray>("oldSerialized");

    ImapAcl acl;
    QTest::newRow("empty") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray(" %% ") << QByteArray("testme@host l %% ");

    acl.insert("user@host", KIMAP::Acl::None);
    QTest::newRow("none") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("user@host  %% ") << QByteArray("testme@host l %% user@host ");

    acl.insert("user@host", KIMAP::Acl::Lookup);
    QTest::newRow("lookup") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("user@host l %% ") << QByteArray("testme@host l %% user@host l");

    acl.insert("user@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
    QTest::newRow("lookup/read") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None)  << QByteArray("user@host lr %% ") << QByteArray("testme@host l %% user@host lr");

    acl.insert("otheruser@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
    QTest::newRow("lookup/read") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("otheruser@host lr % user@host lr %% ")
                                 << QByteArray("testme@host l %% otheruser@host lr % user@host lr");

    QTest::newRow("myrights") << acl << KIMAP::Acl::rightsFromString("lrswipckxtdaen") << QByteArray("otheruser@host lr % user@host lr %%  %% lrswipckxtdaen")
                              << QByteArray("testme@host l %% otheruser@host lr % user@host lr %% lrswipckxtdaen");
}

void ImapAclAttributeTest::testSerializeDeserialize()
{
    QFETCH(ImapAcl, rights);
    QFETCH(KIMAP::Acl::Rights, myRights);
    QFETCH(QByteArray, serialized);
    QFETCH(QByteArray, oldSerialized);

    ImapAclAttribute *attr = new ImapAclAttribute();
    attr->setRights(rights);
    attr->setMyRights(myRights);
    QCOMPARE(attr->serialized(), serialized);

    ImapAcl acl;
    acl.insert("testme@host", KIMAP::Acl::Lookup);
    attr->setRights(acl);

    QCOMPARE(attr->serialized(), oldSerialized);

    delete attr;

    ImapAclAttribute deserializeAttr;
    deserializeAttr.deserialize(serialized);
    QCOMPARE(deserializeAttr.rights(), rights);
    QCOMPARE(deserializeAttr.myRights(), myRights);
}

void ImapAclAttributeTest::testOldRights()
{
    ImapAcl acls;
    acls.insert("first_user@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
    acls.insert("second_user@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
    acls.insert("third_user@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);

    ImapAclAttribute *attr = new ImapAclAttribute();
    attr->setRights(acls);

    ImapAcl oldAcls = acls;
    acls.remove("first_user@host");
    acls.remove("third_user@host");

    attr->setRights(acls);

    QCOMPARE(attr->oldRights(), oldAcls);

    attr->setRights(acls);

    QCOMPARE(attr->oldRights(), acls);
    delete attr;
}

QTEST_MAIN(ImapAclAttributeTest)
