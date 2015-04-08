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

#include "filteractionreplytotest.h"
#include "../filteractions/filteractionreplyto.h"
#include <qtest.h>
#include <QWidget>
FilterActionReplyToTest::FilterActionReplyToTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionReplyToTest::~FilterActionReplyToTest()
{

}

void FilterActionReplyToTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionReplyTo filter;
    QWidget *w = filter.createParamWidget(0);
    QCOMPARE(w->objectName(), QStringLiteral("emailaddressrequester"));
}

void FilterActionReplyToTest::shouldBeEmpty()
{
    MailCommon::FilterActionReplyTo filter;
    QVERIFY(filter.isEmpty());
}

void FilterActionReplyToTest::shouldHadReplyToHeader()
{
    const QString replyTo = QStringLiteral("fooreply@kde.org");

    const QByteArray data = "From: foo@kde.org\n"
                            "To: foo@kde.org\n"
                            "Subject: test\n"
                            "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                            "MIME-Version: 1.0\n"
                            "\n"
                            "test";
    const QByteArray output = "From: foo@kde.org\n"
                              "To: foo@kde.org\n"
                              "Subject: test\n"
                              "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                              "MIME-Version: 1.0\n"
                              "Reply-To: fooreply@kde.org\n"
                              "\n"
                              "test";

    MailCommon::FilterActionReplyTo filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    msgPtr->setContent(data);
    msgPtr->parse();
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, true);

    filter.argsFromString(replyTo);
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), true);
    QCOMPARE(msgPtr->encodedContent(), output);

}

void FilterActionReplyToTest::shouldReplaceReplyToHeader()
{
    const QString replyTo = QStringLiteral("fooreply@kde.org");

    const QByteArray data = "From: foo@kde.org\n"
                            "To: foo@kde.org\n"
                            "Subject: test\n"
                            "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                            "MIME-Version: 1.0\n"
                            "Reply-To: oldfooreply@kde.org\n"
                            "\n"
                            "test";
    const QByteArray output = "From: foo@kde.org\n"
                              "To: foo@kde.org\n"
                              "Subject: test\n"
                              "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                              "MIME-Version: 1.0\n"
                              "Reply-To: fooreply@kde.org\n"
                              "\n"
                              "test";

    MailCommon::FilterActionReplyTo filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    msgPtr->setContent(data);
    msgPtr->parse();
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, true);

    filter.argsFromString(replyTo);
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), true);
    QCOMPARE(msgPtr->encodedContent(), output);
}

void FilterActionReplyToTest::shouldHaveRequiredPart()
{
    MailCommon::FilterActionReplyTo filter;
    QCOMPARE(filter.requiredPart(), MailCommon::SearchRule::CompleteMessage);
}

void FilterActionReplyToTest::shouldNotCreateReplyToWhenAddressIsEmpty()
{
    const QByteArray data = "From: foo@kde.org\n"
                            "To: foo@kde.org\n"
                            "Subject: test\n"
                            "Date: Wed, 01 Apr 2015 09:33:01 +0200\n"
                            "MIME-Version: 1.0\n"
                            "Reply-To: oldfooreply@kde.org\n"
                            "\n"
                            "test";

    MailCommon::FilterActionReplyTo filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    msgPtr->setContent(data);
    msgPtr->parse();
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    MailCommon::ItemContext context(item, true);

    filter.argsFromString(QString());
    QCOMPARE(filter.process(context, false), MailCommon::FilterAction::ErrorButGoOn);
    QCOMPARE(context.needsPayloadStore(), false);
    QCOMPARE(msgPtr->encodedContent(), data);
}

QTEST_MAIN(FilterActionReplyToTest)
