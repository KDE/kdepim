/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "attachmentparttest.h"
#include "qtest_messagecore.h"

#include <QHash>

#include "messagecore_debug.h"
#include <qtest.h>

#include <messagecore/attachment/attachmentpart.h>
using namespace MessageCore;

QTEST_MAIN(AttachmentPartTest)

void AttachmentPartTest::testApi()
{
    const QString str = QStringLiteral("test");
    AttachmentPart::Ptr part = AttachmentPart::Ptr(new AttachmentPart);

    // Test that an AttachmentPart::Ptr can be put in a QHash.
    QHash<AttachmentPart::Ptr, QString> hash;
    hash[ part ] = str;
    QVERIFY(hash.contains(part));

    // Test that an AttachmentPart::Ptr can be put in a QVariant.
    QVariant variant = QVariant::fromValue(part);
    QVERIFY(variant.isValid());
    QVERIFY(variant.canConvert<AttachmentPart::Ptr>());
    QVERIFY(variant.value<AttachmentPart::Ptr>() == part);
}

void AttachmentPartTest::shouldHaveDefaultValue()
{
    AttachmentPart part;
    QCOMPARE(part.size(), (qint64) - 1);
    QVERIFY(!part.isInline());
    QVERIFY(part.isAutoEncoding());
    QVERIFY(!part.isCompressed());
    QVERIFY(!part.isEncrypted());
    QVERIFY(!part.isSigned());
    QCOMPARE(part.encoding(), KMime::Headers::CE7Bit);
    QVERIFY(!part.url().isValid());
    QVERIFY(part.name().isEmpty());
    QVERIFY(part.fileName().isEmpty());
    QVERIFY(part.description().isEmpty());
    QVERIFY(part.charset().isEmpty());
    QVERIFY(part.mimeType().isEmpty());
    QVERIFY(part.data().isEmpty());
}

