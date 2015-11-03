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

#include "collectionannotationsattributetest.h"
#include "../attributes/collectionannotationsattribute.h"
#include <qtest.h>
CollectionAnnotationsAttributeTest::CollectionAnnotationsAttributeTest(QObject *parent)
    : QObject(parent)
{

}

CollectionAnnotationsAttributeTest::~CollectionAnnotationsAttributeTest()
{

}

void CollectionAnnotationsAttributeTest::shouldHaveDefaultValue()
{
    MailCommon::CollectionAnnotationsAttribute attr;
    QVERIFY(attr.annotations().isEmpty());
}

void CollectionAnnotationsAttributeTest::shouldAssignAttribute()
{
    QMap<QByteArray, QByteArray> annotations;
    annotations.insert("foo", "foo");
    annotations.insert("bla", "bla");
    annotations.insert("foo2", "bli");
    MailCommon::CollectionAnnotationsAttribute attr(annotations);
    QCOMPARE(attr.annotations(), annotations);
}

void CollectionAnnotationsAttributeTest::shouldDeserializedAttribute()
{
    QMap<QByteArray, QByteArray> annotations;
    annotations.insert("foo", "foo");
    annotations.insert("bla", "bla");
    annotations.insert("foo2", "bli");
    MailCommon::CollectionAnnotationsAttribute attr(annotations);
    const QByteArray ba = attr.serialized();
    MailCommon::CollectionAnnotationsAttribute result;
    result.deserialize(ba);
    QVERIFY(attr == result);
}

void CollectionAnnotationsAttributeTest::shouldCloneAttribute()
{
    QMap<QByteArray, QByteArray> annotations;
    annotations.insert("foo", "foo");
    annotations.insert("bla", "bla");
    annotations.insert("foo2", "bli");
    MailCommon::CollectionAnnotationsAttribute attr(annotations);
    MailCommon::CollectionAnnotationsAttribute *result = static_cast<MailCommon::CollectionAnnotationsAttribute *>(attr.clone());
    QVERIFY(attr == *result);
    delete result;
}

void CollectionAnnotationsAttributeTest::shouldHaveType()
{
    MailCommon::CollectionAnnotationsAttribute attr;
    QCOMPARE(attr.type(), QByteArray("collectionannotations"));
}

QTEST_MAIN(CollectionAnnotationsAttributeTest)
